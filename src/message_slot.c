// Declare what kind of code we want
// from the header files. Defining __KERNEL__
// and MODULE allows us to access kernel-level
// code not usually available to userspace programs.
#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>    /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/

MODULE_LICENSE("GPL");

//Our custom definitions of IOCTL operations
#include "message_slot.h"

static typedef struct{
  int minorNum;
  char** wordBuffers;
} meSlotDS;

static meSlotDS* message_slots = NULL;
static int message_slots_number = 0;

// helper functions
static int minor_exists(int minor_to_search) {
  if (message_slots == NULL) {
    return 0;
  }
  int i = 0;
  for(; i < message_slots_number; i++) {
    if (message_slots[i].minorNum == minor_to_search) {
      return 1; // found
    }
  }
  return 0;  // did not find
}

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
  printk("Invoking device_open(%p)\n", file);

  // get minor from file
  int minor = iminor(inode)
  if (!minor_exists(minor)) {  // new minor
    // going to add a new entry to message slots
    if (message_slots == NULL) {
      // first entry
      message_slots_number++;
      message_slots = kmalloc(message_slots_number * sizeof(meSlotDS));
      memset(message_slots, 0, sizeof(meSlotDS))
      message_slots[message_slots_number - 1].wordBuffers = kmalloc(BUF_NUM * sizeof(char*));  // allocate 4 channels
      memset(message_slots[message_slots_number - 1].wordBuffers, 0, BUF_NUM * sizeof(char*))
      int i = 0;
      for(; i < BUF_NUM; i++) {
        message_slots[message_slots_number - 1].wordBuffers[i] = kmalloc((BUF_LEN + 1) * sizeof(char))
        memset(message_slots[message_slots_number - 1].wordBuffers, 0, (BUF_LEN + 1) * sizeof(char))
      }
    }
    // at least 1 entry exists
    message_slots_number++;
    meSlotDS* temp_ptr = kmalloc(message_slots_number * sizeof(meSlotDS)); // new allocation
    memset(temp_ptr, 0, message_slots_number * sizeof(meSlotDS))
    memcpy(temp_ptr, message_slots, sizeof(meSlotDS) * (message_slots_number - 1));  // copy old data
    temp_ptr[message_slots_number - 1].wordBuffers = kmalloc(BUF_NUM * sizeof(char*));  // allocate 4 channels
    memset(temp_ptr[message_slots_number - 1].wordBuffers, 0, BUF_NUM * sizeof(char*))
    int i = 0;
    for(; i < BUF_NUM; i++) {
      temp_ptr[message_slots_number - 1].wordBuffers[i] = kmalloc((BUF_LEN + 1) * sizeof(char))
       memset(temp_ptr[message_slots_number - 1].wordBuffers, 0, (BUF_LEN + 1) * sizeof(char))
    }
    message_slots = temp_ptr;

  }


  return SUCCESS;
}

//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file)
{
  printk("Invoking device_release(%p,%p)\n", inode, file);
  return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
  return SUCCESS;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
  // int i;
  // printk("Invoking device_write(%p,%d)\n", file, length);
  // for( i = 0; i < length && i < BUF_LEN; ++i )
  // {
  //   get_user(the_message[i], &buffer[i]);
  //   if( 1 == encryption_flag )
  //     the_message[i] += 1;
  // }
 
  // // return the number of input characters used
  return SUCCESS;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
  // Switch according to the ioctl called
  // if( IOCTL_SET_ENC == ioctl_command_id )
  // {
  //   // Get the parameter given to ioctl by the process
  //   printk( "Invoking ioctl: setting encryption "
  //           "flag to %ld\n", ioctl_param );
  //   encryption_flag = ioctl_param;
  // }

  return SUCCESS;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops =
{
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void)
{
  int rc = -1;
  // Register driver capabilities. Obtain major num
  rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );

  // Negative values signify an error
  if( rc < 0 )
  {
    printk( KERN_ALERT "%s registraion failed for  %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM );
    return rc;
  }

  printk( "Registeration is successful. ");
  printk( "If you want to talk to the device driver,\n" );
  printk( "you have to create a device file:\n" );
  printk( "mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM );
  printk( "Dont forget to rm the device file and "
          "rmmod when you're done\n" );

  return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void)
{
  // Unregister the device
  // Should always succeed
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================
