#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/semaphore.h>
#include<asm/uaccess.h>
#include<asm/siginfo.h>
#include<linux/sched.h>
#include<linux/device.h>
#include<linux/spinlock.h>


const char *AUTHOR = "morpheus";
const char *DEVICE_NAME = "kernel_signal";
const char *CLASS_NAME = "kernel_signal_class";

#define SIG_TEST 44
#define MAX_SIZE 5
#define FOR(i,x,n) for(i=x;i<n;i++)
#define FIRST_MINOR_NUM 0
#define MINOR_COUNT 1

typedef struct
{
  int pid;
  char kbuf[MAX_SIZE];
  struct semaphore sem;
  spinlock_t pidlock;
}my_device;

static my_device pdev;

static void initialize(void)
{
  sema_init(&pdev.sem,1);
  spin_lock_init(&pdev.pidlock);
}

static dev_t dev_num;
static int ret;
static struct class *pclass;
static struct cdev *pcdev;


static int ks_open(struct inode *ksinode,struct file *filp)
{
  printk(KERN_INFO "ks_open() on /dev/%s\n",DEVICE_NAME);
  return 0;
}


static int ks_close(struct inode *ksinode,struct file *filp)
{
  printk(KERN_INFO "ks_close() on /dev/%s\n",DEVICE_NAME);
  return 0;
}


static ssize_t ks_write(struct file *filp,char __user *buf,size_t count,loff_t *pos)
{


  if (down_interruptible(&pdev.sem))
    {
      printk(KERN_INFO "down_interruptible() was interrupted (interrupt?) while calling ks_write() on /dev/%s\n",DEVICE_NAME);
      return -ERESTARTSYS;
    }
  
  memset(pdev.kbuf,0,sizeof(pdev.kbuf));
  
  count = (count > MAX_SIZE) ? MAX_SIZE : count;
  printk(KERN_INFO "count : %d, for /dev/%s\n",count,DEVICE_NAME);
  ret = copy_from_user(pdev.kbuf,buf,count);
  if (ret)
    {
      printk(KERN_INFO "copy_from_user() failed : %d, for %s\n",ret,DEVICE_NAME);
      up(&pdev.sem);
      return -ENOMEM;
    }
  
  printk(KERN_INFO "ks_write() called on /dev/%s\n",DEVICE_NAME);

  sscanf(pdev.kbuf,"%d",&pdev.pid);

  printk(KERN_INFO "wrote pid : %d for /dev/%s\n",pdev.pid,DEVICE_NAME);
  
  // here we send a signal to the process whose pid was given
  
  struct task_struct *sig_task;
  sig_task = NULL;
  unsigned long flags;
  int flag = 0;
  spin_lock_irqsave(&pdev.pidlock,flags);
  
  sig_task  = pid_task(find_get_pid(pdev.pid),PIDTYPE_PID);
  if (!sig_task)
    {
      flag = 1;
    }
  spin_unlock_irqrestore(&pdev.pidlock,flags);
  

  if (flag)
    {
      printk(KERN_INFO "could not identify process with pid : %d for /dev/%s\n",pdev.pid,DEVICE_NAME);
      up(&pdev.sem);
      return count;
    }
  

  struct siginfo info;
  memset(&info,0,sizeof(struct siginfo));
  info.si_signo = SIG_TEST;
  info.si_code = SI_QUEUE;
  info.si_int = 1234;

  ret = send_sig_info(SIG_TEST,&info,sig_task);
  if (ret < 0)
    {
      printk(KERN_INFO "send_sig_info() failed (%d) for /dev/%s\n",DEVICE_NAME);
      up(&pdev.sem);
      return count;
    }
  
   up(&pdev.sem);
   printk(KERN_INFO "signal sent to process for /dev/%s\n",DEVICE_NAME);
  
   // return number of bytes of data copied successfully
  return count;
  
}

static struct file_operations fops = {

  .owner = THIS_MODULE,
  .open  = ks_open,
  .release = ks_close,
  .write = ks_write
};

static int __init sk_init(void)
{
  // create a major,minor number pair for the device
  
  ret = alloc_chrdev_region(&dev_num,FIRST_MINOR_NUM,MINOR_COUNT,DEVICE_NAME);
  if (ret)
    {
      printk(KERN_INFO "could not allocate major,minor pair fpr %s\n",DEVICE_NAME);
      return -1;
    }

  pclass = class_create(THIS_MODULE,CLASS_NAME);
  if (!pclass)
    {
      printk(KERN_INFO "could not create /sys/class/%s for %s\n",CLASS_NAME,DEVICE_NAME);
      unregister_chrdev_region(dev_num,MINOR_COUNT);
      return -1;
    }

  if (!device_create(pclass,NULL,dev_num,NULL,DEVICE_NAME))
    {
      printk(KERN_INFO "could not create a device file /dev/%s\n",DEVICE_NAME);
      class_destroy(pclass);
      unregister_chrdev_region(dev_num,MINOR_COUNT);
      return -2;
    }

  // allocate memory for the character device driver
  pcdev = cdev_alloc();
  pcdev->owner = THIS_MODULE;
  pcdev->ops = &fops;

  ret = cdev_add(pcdev,dev_num,MINOR_COUNT);
  if (ret)
    {
      printk(KERN_INFO "could not register a struct cdev for %s\n",DEVICE_NAME);
      device_destroy(pclass,dev_num);
      class_destroy(pclass);
      unregister_chrdev_region(dev_num,MINOR_COUNT);
      return -3;
    }
  
  initialize();
  printk(KERN_INFO "successfully registered a char dev with device number %d at /dev/%s\n",MAJOR(dev_num),DEVICE_NAME);

  return 0;

}

static void __exit sk_exit(void)
{
  cdev_del(pcdev);
  device_destroy(pclass,dev_num);
  class_destroy(pclass);
  unregister_chrdev_region(dev_num,MINOR_COUNT);
}

module_init(sk_init);
module_exit(sk_exit);
MODULE_OWNER(AUTHOR);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sending signals from the kernel");
