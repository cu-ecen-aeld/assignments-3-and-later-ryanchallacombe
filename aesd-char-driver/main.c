/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>     /* kmalloc() */
#include <linux/uaccess.h>  /* copy_*_user */
#include <linux/fs.h>     /* fixed_size_llseek() */

#include "aesdchar.h"
#include "aesd_ioctl.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Ryan Challacombe");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;        // global scope

/************************************************************
* aesd_open
*   This is called everytime the userspace interacts with the driver
*   this is the first thing the driver does
*/
int aesd_open(struct inode *inode, struct file *filp)   
{
    PDEBUG("starting aesd_open() function");

    /***********************************************************
     * TODO: handle open
     */

    struct aesd_dev *dev;        // device information 
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;

    /************************************************************/

    return 0;
}

/************************************************************
* aesd_release
*   This is called everytime the userspace interacts with the driver
*   this is the last thing the driver does before finishing it's transaction
*   e.g. after a call to write or read
*/
int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("starting aesd_release() function");

    /************************************************************
     * TODO: handle release
     */

    // No actions here

    /************************************************************/

    return 0;
}

/************************************************************
* aesd_read
*   This is a read from the driver to the userspace
*   It is a write from the driver's perspective
*/
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    //PDEBUG("starting aesd_read() function");
    //PDEBUG("requesting a read of %zu bytes with offset %lld", count, *f_pos);
    ssize_t retval = 0;     // number of bytes read
    
    /***********************************************************
     * TODO: handle read
     */
    struct aesd_dev *dev = filp->private_data;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // read from aesd circular buffer
    size_t ret_offset;                      // holds the offset of the first char in the returned buffer entry
    struct aesd_buffer_entry *ret_ent;      // will hold the entry value found at f_pos or NULL if none
    ret_ent = aesd_circular_buffer_find_entry_offset_for_fpos( &dev->circ_buff, *f_pos, &ret_offset ); 

    if ( ret_ent == NULL) {
        // nothing to read
        retval = 0;
        //PDEBUG("Nothing to read. aesd_read() returning with 0\n");
    } else {
        // Write a maximum of 1 circular buffer entry
        // if more bytes are requested we will limit it to 1 entry
        // if fewer bytes are requested we will send only the requested bytes
        // if userspace buffer is small enough that multiple reads are required, 
        if ( count > ret_ent->size - ret_offset)
            count = ret_ent->size - ret_offset;

        //PDEBUG("copy_to_user count = %zu bytes", count);
        //PDEBUG("copy_to_user circular buffer f_pos: %lld", *f_pos);
        //PDEBUG("copy_to_user circular buffer ret_ent offset (*ret_offset): %zu", ret_offset);

        unsigned long uncopied_count, copied_count;
        // uncopied_count = copy_to_user( buf, (void *) ret_ent->buffptr, ret_ent->size );
        uncopied_count = copy_to_user( buf, (void *) ret_ent->buffptr + ret_offset, count );
        copied_count = retval = count - uncopied_count;

        //PDEBUG("aesd_read() complete: %zu bytes with offset %lld", retval, *f_pos);

        // update pointer to point to next entry
        *f_pos = *f_pos + copied_count;
        //PDEBUG("f_pos updated to %lld\n", *f_pos);

    }
    /************************************************************/

    mutex_unlock(&dev->lock);
    return retval;
}


/************************************************************
* aesd_write
*   This is a write to the driver from the userspace
*   It is a read from the driver's perspective
*/
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    
    PDEBUG("starting aesd_write() function");
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    /************************************************************
     * TODO: handle write
     */
    
    ssize_t retval = -ENOMEM;   // value used in "goto out" statements
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *g_ent = dev->g_ent;

    // Lock data
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // allocate memory for this write
    // if we are starting with an empty buffer, we can just allocate @param count bytes
    // if our buffer has data from a previous write, we need to allocate more
    struct aesd_buffer_entry *l_ent = kmalloc( sizeof(struct aesd_buffer_entry), GFP_KERNEL );
    if ( l_ent == NULL ) {
        PDEBUG("error allocating for l_ent\n");
        retval = -ENOMEM;
        goto out;
    }
    memset(l_ent, 0, sizeof(struct aesd_buffer_entry));

    // this allocation accounts for existing write data to the buff pointed to by g_ent
    // if no existing data in g_ent, the size is zero and the pointer is NULL
    l_ent->buffptr = kmalloc(count + g_ent->size, GFP_KERNEL);
    if ( l_ent->buffptr == NULL ) {
        PDEBUG("error allocating for l_ent->buffptr\n");
        retval = -ENOMEM;
        kfree(l_ent);
        goto out;
    }
    l_ent->size = 0;
    
    // save the size of the g_ent buffer for later use
    size_t g_ent_start_size = g_ent->size;

    // if existing data pointed to by g_ent->buffptr, we need to copy it to new and bigger buffer, then free it
    if ( g_ent->buffptr != NULL ) {
        //PDEBUG("*** copying previous write into newly alloc'd memory\n");
        memcpy( (void *) l_ent->buffptr, g_ent->buffptr, g_ent->size);
        l_ent->size = g_ent->size;        // this is how many bytes we copied into l_ent 
        //PDEBUG("*** after memcpy l_ent->size = %zu\n", l_ent->size);
        kfree( g_ent->buffptr );
        g_ent->buffptr = NULL;
        g_ent->size = 0;
    }


    // copy data from user
    // note that we need to account for the size of any data from g_ent that was copied into l_ent
    // by starting the write at that offset
    size_t uncopied_count = count;
    uncopied_count = copy_from_user( (void *) (l_ent->buffptr + g_ent_start_size), buf, count);
    //PDEBUG("copy_from_user copied %zu bytes\n", count - uncopied_count);
    l_ent->size += (count - uncopied_count);
    //PDEBUG("*** after copy_from_user l_ent->size = %zu\n", l_ent->size);
    retval = count - uncopied_count;

    // loop through buffer to see if '\n' character is found
    char c;
    char newline_found = 0;
    for(int i=0; i <= l_ent->size; i++ ) {
        c = *(l_ent->buffptr + i);   
        if ( c == '\n' ) {
            newline_found = 1;
            //PDEBUG("newline_found at index=%i\n", i);
            break;
        }
    }

    // if newline found, write to circular buffer
    if ( newline_found ) {
        // Place value into the circular buffer
        // return value will point to memory that is overwritten
        //      Or be NULL if nothing overwritten
        const char *ret_ptr = NULL;
        ret_ptr = aesd_circular_buffer_add_entry( &dev->circ_buff, l_ent );
        //PDEBUG("circular buffer entry added\n");
        if ( ret_ptr != NULL ) {
            //PDEBUG("freeing overwritten buffer entry\n");
            kfree(ret_ptr);
        }

    }
    else {
        // No newline was found so we need to keep this data
        // we will append to it on next write
        // save it to g_ent for use with next write
        g_ent->buffptr = l_ent->buffptr;
        g_ent->size = l_ent->size;
        //PDEBUG("No newline found. Data stored in g_ent. g_ent->size = %zu\n", g_ent->size);
    }

    // Because the aesd_circular_buffer_add_entry() function simply copies
    // the pointer and size into a statically allocated array of entries,
    // we can free and reset l_ent here
    // PDEBUG("freeing l_ent\n");
    kfree(l_ent);
    l_ent = NULL;

    /************************************************************/

    out:
        mutex_unlock(&dev->lock);
        return retval;
}


/************************************************************
* aesd_adjust_file_offset
*   
*   Adjusts the file offset parameter (f_pos) of filp based on the location specified by write_cmd (zero-referenced
*   command to locate) and write_cmd_offset (zero referenced offset into the command)
*   return zero if successful, negative if error occurred:
*       -ERSTARTSYS if mutex could not be obtained
*       -EINVAL if write_cmd or write_cmd_offset are out of range
*/
static long aesd_adjust_file_offset( struct file *filp, unsigned int write_cmd, unsigned int write_cmd_offset ) {

    /*
        - Check for valid write_cmd and write_cmd_offset values 
            - haven’t written this command yet
            - out of range cmd (11)
            - write_cmd_offset is >= size of command
        - Calculate the start offset to write_cmd
            - Add length of each write between the output pointer and write_cmd
        - Add write_cmd_offset
        - Save as filp->f_pos
    */

    PDEBUG("starting aesd_adjust_file_offset() function");
    PDEBUG("write_cmd %i with write_cmd_offset %i", write_cmd, write_cmd_offset);

    long retval = 0;

    struct aesd_dev *dev = filp->private_data;

    // TODO check all indices for overrun

    //size_t ret_offset;                      // holds the offset of the first char in the returned buffer entry
    //struct aesd_buffer_entry *ret_ent;      // will hold the entry value found at f_pos or NULL if none
    // ret_ent = aesd_circular_buffer_find_entry_offset_for_fpos( &dev->circ_buff, *f_pos, &ret_offset ); 

    // check for a valid cmd range
    if ( write_cmd > AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) {
        PDEBUG("write_cmd is out of range. Returing %i\n", -EINVAL);
        return -EINVAL;
    }

    // Loop through the buffer
    // As we go sum up the total bytes in total_offset
    // When we reach the desired command we need to first check that the command is not NULL and 
    // that the offset is within range of the size of that command
    // if those are okay, then we can add the write_cmd_offset to the total_offset
    // TODO rewrite loop to improve logic
    uint8_t index;
    loff_t total_offset = 0;
    struct aesd_buffer_entry *entryptr;
    AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &dev->circ_buff, index) {
        if (entryptr->buffptr != NULL) 
            total_offset +=  entryptr->size;

        if ( index == write_cmd ) {    
            if (entryptr->buffptr != NULL) {
                PDEBUG("write_cmd position in circ_buff is NULL. Returing %i\n", -EINVAL);
                return -EINVAL;
            } else if ( entryptr->size <= write_cmd_offset ) {
                PDEBUG("write_cmd_offset is >= size of the command. Returing %i\n", -EINVAL);
                return -EINVAL;
            } else {
                PDEBUG("write_cmd and write_cmd_offset are valid\n");
                total_offset += write_cmd_offset;
            }   
            break;
        }
    }

    PDEBUG("Exited circ_buff loop with total_offset = %lld\n", total_offset);

    // Lock data
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Call llseek. 
    // TODO All seeks are relative to SEEK_SET ??
    // TODO will this handle negative offsets / commands?
    // TODO check returned value here
    // loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
    retval = aesd_llseek(filp, total_offset, SEEK_SET);

    mutex_unlock(&dev->lock);

    return retval;

}

/************************************************************
* aesd_llseek
*   repositions the file offset of filp to the argument offset 
*   according to the directive whence as follows:
*   SEEK_SET: The file offset is set to offset bytes.
*   SEEK_CUR: The file offset is set to its current location plus offset bytes.
*   SEEK_END: The file offset is set to the size of the file plus offset bytes.
*/
loff_t aesd_llseek(struct file *filp, loff_t offset, int whence) {

    /*
    - If the llseek method is missing from the device’s
    operations, the default implementation in the kernel
    performs seek by modifying filp->fpos.
    - For the lseek system call to work correctly the read
    and write methods must cooperate by using and
    updating the offset item they receive as an argument
    */

    /** Circular buffer implementation
    * The caller will send a struct via the ioctl system call
    * It will have a cmd member which is the number of the command in the buffer (0-10)
    *   that is seeks
    * It will have an offset member which is the numerical byte offset of the char
    *   within the command that it seeks
    *   example: 
    *       buffer contents:
    *       Grass       (size = 5)
    *       Sentosa     (size = 7)
    *       Singapore   (size = 9)
    *
    *       struct seekto.cmd = 2, seekto.cmd_offset = 3
    * 
    *       We need to set filp->f_pos to char 'n' which is f_pos = 15
    */

    PDEBUG("starting aesd_llseek() function\n");
    loff_t retval = 0;
    struct aesd_dev *dev = filp->private_data;

    // Loop through to find total size of buffer
    uint8_t index;
    loff_t total_buffer_size = 0;
    struct aesd_buffer_entry *entryptr;
    AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &dev->circ_buff, index) {
        if (entryptr->buffptr != NULL)
            total_buffer_size += entryptr->size;
    } 

    PDEBUG("Calling fixed_size_llseek with total_buffer_size = %lld\n", total_buffer_size);

    // loff_t fixed_size_llseek(struct file *file, loff_t offset, int whence, loff_t size);
    retval = fixed_size_llseek(filp, offset, whence, total_buffer_size);

    return retval;
}

/************************************************************
* aesd_ioctl
*   
*/
long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {

    PDEBUG("starting aesd_ioctl() function\n");

    long retval = 0;

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != AESD_IOC_MAGIC) {
        PDEBUG("incorrect _IOC_TYPE(cmd): %i\n", _IOC_TYPE(cmd));
        return -ENOTTY;
    }
    if (_IOC_NR(cmd) > AESDCHAR_IOC_MAXNR) {
        PDEBUG("_IOC_NR(cmd) out of range: %i\n", _IOC_NR(cmd));
        return -ENOTTY;
    }

    switch (cmd) {

        case AESDCHAR_IOCSEEKTO: 
            struct aesd_seekto seekto;
            if ( copy_from_user( &seekto, (const void __user *) arg, sizeof(seekto) ) != 0 ) {
                retval = EFAULT;
            } else {
                PDEBUG("userspace calling aesd_ioctl with seekto.write_cmd: %i\n", seekto.write_cmd);
                PDEBUG("userspace calling aesd_ioctl with seekto.write_cmd_offset: %i\n", seekto.write_cmd_offset);
                retval = aesd_adjust_file_offset(filp, seekto.write_cmd, seekto.write_cmd_offset);
            }
            break;
        
        default:  /* redundant, as cmd was checked against MAXNR */
            return -ENOTTY;
    }   // switch

    PDEBUG("aesd_ioctl returning %ld\n", retval);
    return retval;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .unlocked_ioctl = aesd_ioctl,
    .llseek = aesd_llseek,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}

/************************************************************
* aesd_init_module
*   This is called when the module is loaded by the kernel  
*/
int aesd_init_module(void)
{
    PDEBUG("Starting aesd_init_module()\n");
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    /**********************************************************
     * TODO: initialize the AESD specific portion of the device
     */
    PDEBUG("&aesd_device: %p\n", (void *) &aesd_device);
    PDEBUG("&aesd_device.circ_buff: %p\n", (void *) &aesd_device.circ_buff );
    PDEBUG("&aesd_device.lock: %p\n", (void *) &aesd_device.lock );
    PDEBUG("&aesd_device.g_ent: %p\n", (void *) &aesd_device.g_ent );
    PDEBUG("aesd_device.g_ent: %p\n", aesd_device.g_ent );

    // initialize buffer
    aesd_circular_buffer_init( &aesd_device.circ_buff );
    
    // initialiaze mutex
    mutex_init( &aesd_device.lock );

    // initialize statically allocatted aesd_buffer_entry g_ent 
    // WRONG!! the value of aesd_device.g_ent was set to zero, so it points to location 0
    // we cannot dereference it
    // aesd_device.g_ent->buffptr = NULL;
    // aesd_device.g_ent->size = 0;

    // dynamically allocate aesd_buffer_entry and point g_ent to it
    struct aesd_buffer_entry *ent = kmalloc( sizeof( struct aesd_buffer_entry ), GFP_KERNEL );
    if (ent == NULL) {
        PDEBUG("error allocating for g_ent. \n");
        result = -ENOMEM;
        goto init_fail;       
    }
    PDEBUG("ent: %p\n", (void *) ent );
    aesd_device.g_ent = ent;
    PDEBUG("aesd_device.g_ent: %p\n", (void *) aesd_device.g_ent );

    // initialize values in g_ent structure
    aesd_device.g_ent->buffptr = NULL;
    aesd_device.g_ent->size = 0;


    /**********************************************************/

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }

    PDEBUG("Exiting aesd_init_module() with return = %i\n", result);
    return result;

    init_fail:
        printk(KERN_ERR "Error initializing aesdchar driver. Exiting\n");
        aesd_cleanup_module();
        return result;
}

/************************************************************
* aesd_cleanup_module
*   This is called when the module is unloaded by the kernel  
*/
void aesd_cleanup_module(void)
{
    PDEBUG("Starting aesd_cleanup_module()\n");
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**********************************************************
     * TODO: cleanup AESD specific poritions here as necessary
     */

    struct aesd_dev *dev = &aesd_device;

    uint8_t index;
    struct aesd_buffer_entry *entryptr;
    AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &dev->circ_buff, index) {
        if (entryptr->buffptr != NULL) {
            PDEBUG("freeing aesd_buffer_entry: %s\n", entryptr->buffptr);
            kfree((void *) entryptr->buffptr);            
        }
    }

    PDEBUG("Freeing aesd_device.g_ent\n");
    kfree( aesd_device.g_ent );

    /**********************************************************/

    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
