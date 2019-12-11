/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @Updated By: Shubham Jaiswal, Ayush Dhoot
 * @date 2019-12-09
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "../aesd_ioctl.h"
#include "aesdchar.h"

static long aesd_adjust_file_offset(struct file *filp, unsigned int write_cmd, unsigned int write_cmd_offset);

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Shubham and Ayush");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	PDEBUG("open");
	struct aesd_dev *dev;
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	return 0;
}


ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct aesd_dev *dev = filp->private_data;
	ssize_t retval = 0;
	loff_t offset = (*f_pos);

	size_t read_count = 0;
	PDEBUG("pointer offset=%lld",filp->f_pos);
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	if(*f_pos >= dev->size)
		goto out;

	if(*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if(copy_to_user(buf, dev.buff, count)
	{
		retval = -EFAULT;
	}
		
	*f_pos += count;
	retval = count;

	out:
	mutex_unlock(&dev->lock);	
	PDEBUG("Return Value = %ld,pos=%lld\n", retval, *f_pos);
	return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	struct aesd_dev *dev = filp->private_data;
	int i, tot_size = 0;	
	//char *temp = NULL;

	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;


	dev.buff = kmalloc(count, GFP_KERNEL);
		if (dev.buff == NULL){
			PDEBUG("ERROR:kmalloc");
			return -1;
		}

	if(copy_from_user(dev.buff, buf, count))
	{
		retval = -EFAULT;
		goto out;
	}

	dev.size += count;

	retval = count;
	out:
	mutex_unlock(&dev->lock);
	return retval;
}

loff_t aesd_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t new_off;
	struct aesd_dev *dev = filp->private_data;
	
	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;
		
	new_off = fixed_size_llseek(filp, off, whence, dev->size);
	
	mutex_unlock(&dev->lock);
	return new_off;
}


long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long retval;
	switch(cmd)
	{
		case AESDCHAR_IOCSEEKTO:
			{
		       		struct aesd_seekto seekto;
				if(copy_from_user(&seekto, (const void __user*) arg, sizeof(seekto)) != 0)
				{
					retval = -EFAULT;
				}
				else
				{
					retval = aesd_adjust_file_offset(filp, seekto.write_cmd, seekto.write_cmd_offset);
				}
				break;
			}

		default:
			{
				retval = -EINVAL;
			}
	}

	return retval;
}


static long aesd_adjust_file_offset(struct file *filp, unsigned int write_cmd, unsigned int write_cmd_offset)
{
	long retval = 0;
	loff_t new_pos = 0;
	struct aesd_dev *dev = filp->private_data;
	
	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	new_pos += write_cmd_offset;
	

	filp->f_pos = new_pos;
	PDEBUG("File Pointer after ioctl:%lld\n", filp->f_pos);
	out_fail:
		mutex_unlock(&dev->lock);
		return retval;		
}


struct file_operations aesd_fops = {
	.owner =    			THIS_MODULE,
	.read =     			aesd_read,
	.write =   				aesd_write,
	.open =     			aesd_open,
	.release =  			aesd_release,
	.llseek = 				aesd_llseek,
	.unlocked_ioctl = 		aesd_ioctl,
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



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));
	
	mutex_init(&aesd_device.lock);

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	PDEBUG("Successful");
	return result;

}

void aesd_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);

	kfree(aesd_device.buff);
	
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
