#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<linux/semaphore.h>

/*
  This is my first char device driver
  linux/cdev.h - this is for registering the char device with the kernel
  linux/fs.h   - this is for the file operations structure that is accessed from user space
  asm/uaccess.h- this is for copy_from_user() and copy_to_user()
  linux/semaphore.h -this is used for synchronization
 */

#define FOR(i,x,n) for(i=x;i<n;i++)
#define DEVICE_NAME "my_test_char_device"
#define MAX_BUF_SIZE 100

// create the structure for a contrived device
struct test_device
 {
  char data[MAX_BUF_SIZE];
  struct semaphore sem;
}my_dev;

//create char device driver pointer
struct cdev *mcdev;
dev_t dev_num;                        // holds the major and minor numbers assigned by the kernel
int major_number;                     // used to store the major number portion of the device number obtained from the kernel
int ret;                              // used to store the return value as we don't want to eat kernel memory


/*
  create the callback functions that are used to accomplish the individual user actions -  open ,close , read and write
 */


// called when a process attempts to open the device
int device_open(struct inode *dev_inode,struct file *filp)
{
  // ensure that only one process can open the file at a time
  if (down_interruptible(&my_dev.sem) != 0)
    {
      printk(KERN_ALERT "could not get a lock on the device %s. It is currently being used by another process\n");
      return -1;
    }

  return 0;
}

// called when a process attempts to read from a device
ssize_t device_read(struct file *filp,char *user_data_buffer,size_t user_buf_count,loff_t *curr_offset)
{
  if (user_buf_count > MAX_BUF_SIZE)
    {
      printk(KERN_ALERT "ERROR: cannot read more than %d data from device. Exiting now\n",MAX_BUF_SIZE);
      return -1;
    }

  // if bytes requested can be served, copy_to_user
  printk(KERN_ALERT "reading from device %s",DEVICE_NAME);
  ret = copy_to_user(user_data_buffer,my_dev.data,user_buf_count);
  return ret;
}


// called when a process attempts to write to the device
ssize_t device_write(struct inode *dev_inode,char *user_data_buf,size_t user_buf_count,loff_t *curr_offset)
{
   if (user_buf_count > MAX_BUF_SIZE)
    {
      printk(KERN_ALERT "ERROR: cannot write more than %d data to the device. Exiting now\n",MAX_BUF_SIZE);
      return -1;
    }

  // if bytes requested can be served, copy_to_user
  printk(KERN_ALERT "writing to device %s",DEVICE_NAME);
  ret = copy_from_user(my_dev.data,user_data_buf,user_buf_count);
  return ret;
}

// this is used to release the lock on the device
int device_close(struct inode *dev_inode,struct file *filp)
{

  up(&my_dev.sem);
  printk(KERN_ALERT "Releasing the lock on the device %s. closing the device now\n",DEVICE_NAME);
  return 0;
}



/*
  create the file_operations structure to register the different callback functions that the kernel can invoke when the user
  interacts with our device. 
*/

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
    .read = device_read,
    .write = device_write
};




/*
  This entry point is for registering the capabilities of our device driver to the system
 */
static int driver_entry_fn(void)
{

  // dynamically obtain device numbers from the kernel, first minor number is 0 and we need a total of 1 minor numbers
  ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
  if (ret < 0)
    {
      printk(KERN_ALERT "ERROR: could not obtain device numbers for %s. exit with error %d\n",DEVICE_NAME,ret);
      return ret;
    }
  major_number = MAJOR(dev_num);
  printk(KERN_ALERT "device %s has been allocated a major number %d\n",DEVICE_NAME,major_number);

  // create a char device struct

  mcdev = cdev_alloc();
  mcdev->ops = &fops;
  mcdev->owner = THIS_MODULE;
  
  // register the char device struct with the kernel and associate it with the obtained device numbers
  ret = cdev_add(mcdev,dev_num,1);
  if (ret < 0)
    {
      printk(KERN_ALERT "ERROR: could not add driver for device %s to the character device drivers in the kernel\n",DEVICE_NAME);
      return ret;
    }

  // initialize the semaphore to 1. 
  // NOTE: this is not a user level sempahore like the one used with pthreads. 
  sema_init(&my_dev.sem,1);
  
  return 0;
}


/*
  This function is used to deallocate all resources allocated by our device driver
  all operations are done in the reverse order
 */
static void driver_exit_fn(void)
{
  // delete the character device driver from the kernel
  cdev_del(mcdev);
  
  // deallocate the device region
  unregister_chrdev_region(dev_num,1);
  printk(KERN_ALERT "unregistered the device %s\n",DEVICE_NAME);
  
}


// tell the kernel where to start and stop
module_init(driver_entry_fn);
module_exit(driver_exit_fn);
