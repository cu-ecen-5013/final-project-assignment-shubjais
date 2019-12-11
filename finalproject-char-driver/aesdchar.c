/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @Updated By: Shubham Jaiswal and Ayush Dhoot
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

MODULE_AUTHOR("Shubham and Ayush"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev *aesd_device;

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
	/**
	 * TODO: handle release
	 */
	return 0;
}



ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct aesd_dev *dev = filp->private_data;
	ssize_t retval = 0;
	int i = dev->c_buffer.buf_head, ind = 0; //tot_size = 0;
	int skipped = 0;
	unsigned int buf_count = dev->c_buffer.buf_count;
	loff_t offset = (*f_pos);
	//size_t c_count = count;
	size_t read_count = 0;
	PDEBUG("pointer offset=%lld",filp->f_pos);
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	if(*f_pos >= dev->size)
		goto out;

	if(*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if(offset > 0)
	{
		while(1)
		{
			offset -= dev->c_buffer.buf_data_size[i];
			if(offset >= 0)
			{
				i = (((i+1) % BUFF_SIZE) == 0)? 0: i + 1;
				skipped += 1;
			}
			else
			{
				PDEBUG("index to read from: %d\n", i);
				offset += dev->c_buffer.buf_data_size[i];
				break;	
			}	
		}

	}

	buf_count -= skipped;
	while(buf_count > 0 && read_count < count)
	{
		PDEBUG("Buffer index = %d\n", i);
		if(dev->c_buffer.buf_data[i] == NULL)
		{
			goto out;
		}

		PDEBUG("User Buffer Index = %d\n", ind);
		if(copy_to_user(buf + ind, (dev->c_buffer.buf_data[i] + offset), (dev->c_buffer.buf_data_size[i] - offset)))
		{
			retval = -EFAULT;
		}

		read_count = read_count + (dev->c_buffer.buf_data_size[i] - offset); 
		ind = ind + (dev->c_buffer.buf_data_size[i] - offset);
		offset = 0;
		i = (((i+1) % BUFF_SIZE) == 0)? 0: i + 1;	
		buf_count--;
	}

	//if(read_count >= count)
	//{
	//*f_pos = 0;
	//retval = 0;
	//}
	//else
	//{
	*f_pos += count;
	retval = count;
	//}

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
	char *temp = NULL;

	if(mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;

	temp = (char *)kmalloc(sizeof(char) * count, GFP_KERNEL);
	if(temp == NULL)
	{
		goto out;
	}

	if(copy_from_user(temp, buf, count))
	{
		retval = -EFAULT;
		goto out;
	}

	if(dev->c_buffer.temp == NULL)
	{
		if(temp[count - 1] == '\n')
		{
			if(dev->c_buffer.buf_data[dev->c_buffer.buf_tail] == NULL)
			{	 
				dev->c_buffer.buf_data[dev->c_buffer.buf_tail] = temp;
			}
			else
			{
				kfree(dev->c_buffer.buf_data[dev->c_buffer.buf_tail]);
				PDEBUG("Freed Over written index: %d\n", dev->c_buffer.buf_tail);
				dev->c_buffer.buf_data[dev->c_buffer.buf_tail] = temp;
			}

			dev->c_buffer.buf_data_size[dev->c_buffer.buf_tail] = count;

			if((dev->c_buffer.buf_tail + 1) % BUFF_SIZE == 0)
			{
				dev->c_buffer.buf_tail = 0;
				if(dev->c_buffer.buf_of)
					dev->c_buffer.buf_head = dev->c_buffer.buf_tail;
				dev->c_buffer.buf_of = 1;
			}
			else
			{
				if(dev->c_buffer.buf_of)
				{
					dev->c_buffer.buf_head = dev->c_buffer.buf_tail + 1; 
				}
				dev->c_buffer.buf_tail = (dev->c_buffer.buf_tail + 1);
			}
			if(dev->c_buffer.buf_count < BUFF_SIZE)
			{
				dev->c_buffer.buf_count++;	
			}
			PDEBUG("Buffer Count: %d\n", dev->c_buffer.buf_count);
			PDEBUG("Buffer Head: %d\n", dev->c_buffer.buf_head);
			PDEBUG("Buffer tail: %d\n", dev->c_buffer.buf_tail);
		}
		else
		{
			dev->c_buffer.temp = temp;
			dev->c_buffer.temp_count += count;

		}
	}
	else
	{
		int ind = dev->c_buffer.temp_count, i = 0;
		dev->c_buffer.temp_count += count;
		dev->c_buffer.temp = (char *)krealloc(dev->c_buffer.temp, dev->c_buffer.temp_count, GFP_KERNEL);
		while(i < count)
		{
			dev->c_buffer.temp[ind + i] = temp[i];
			i++;
		}

		if((char)temp[count - 1] == '\n')
		{	
			if(dev->c_buffer.buf_data[dev->c_buffer.buf_tail] == NULL)
			{	 
				dev->c_buffer.buf_data[dev->c_buffer.buf_tail] = dev->c_buffer.temp;
			}
			else
			{
				kfree(dev->c_buffer.buf_data[dev->c_buffer.buf_tail]);
				PDEBUG("Freed Over written index: %d\n", dev->c_buffer.buf_tail);
				dev->c_buffer.buf_data[dev->c_buffer.buf_tail] = dev->c_buffer.temp;
			}

			dev->c_buffer.temp = NULL;
			dev->c_buffer.buf_data_size[dev->c_buffer.buf_tail] = dev->c_buffer.temp_count;
			dev->c_buffer.temp_count = 0;

			if((dev->c_buffer.buf_tail + 1) % BUFF_SIZE == 0)
			{
				dev->c_buffer.buf_tail = 0;
				if(dev->c_buffer.buf_of)
					dev->c_buffer.buf_head = dev->c_buffer.buf_tail;
				dev->c_buffer.buf_of = 1;
			}
			else
			{
				if(dev->c_buffer.buf_of)
				{
					dev->c_buffer.buf_head = dev->c_buffer.buf_tail + 1; 
				}
				dev->c_buffer.buf_tail = (dev->c_buffer.buf_tail + 1);
			}
			if(dev->c_buffer.buf_count < BUFF_SIZE)
			{
				dev->c_buffer.buf_count++;	
			}
			PDEBUG("Buffer Count: %d\n", dev->c_buffer.buf_count);
			PDEBUG("Buffer Head: %d\n", dev->c_buffer.buf_head);
			PDEBUG("Buffer tail: %d\n", dev->c_buffer.buf_tail);
		}

		kfree(temp);	
	}

	for(i =0; i < BUFF_SIZE ; i++)
	{
		if(dev->c_buffer.buf_data[i] != NULL)
		{
			int j = 0;
			while(1)
			{
				PDEBUG("data[%d] = %c", i, dev->c_buffer.buf_data[i][j]);
				if(dev->c_buffer.buf_data[i][j] == '\n')
					break;
				j++;
			}
			PDEBUG("size = %d\n", dev->c_buffer.buf_data_size[i]);
			tot_size += dev->c_buffer.buf_data_size[i];
		}

		if(i == (BUFF_SIZE - 1) || dev->c_buffer.buf_data[i] == NULL)
		{
			dev->size = tot_size;
			PDEBUG("file size = %ld\n", dev->size);
			break;
		}
	}

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

	unsigned int i = dev->c_buffer.buf_head;

	if((write_cmd > (dev->c_buffer.buf_count - 1)) || (write_cmd < 0))
	{
		retval = -EINVAL;
		goto out_fail;
	}
	else
	{
		while(write_cmd > 0)
		{
			new_pos += dev->c_buffer.buf_data_size[i];
			i = (((i+1) % BUFF_SIZE) == 0)? 0: i + 1;
			write_cmd--;
		}

		if((write_cmd_offset > (dev->c_buffer.buf_data_size[i] - 1)) || (write_cmd_offset < 0))
		{
			retval = -EINVAL;
			goto out_fail;
		}
		else
		{
			new_pos += write_cmd_offset;
		}
	}

	filp->f_pos = new_pos;
	PDEBUG("File Pointer after ioctl:%lld\n", filp->f_pos);
out_fail:
	mutex_unlock(&dev->lock);
	return retval;		
}


struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
	.llseek = aesd_llseek,
	.unlocked_ioctl = aesd_ioctl,
};

static int aesd_setup_cdev(struct aesd_dev *dev, int index)
{
	int err, devno = MKDEV(aesd_major, aesd_minor + index);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev %d", err, index);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result, i;
	result = alloc_chrdev_region(&dev, aesd_minor, NUM_DEVICES, "aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) 
	{
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}

	aesd_device = kmalloc(NUM_DEVICES * sizeof(struct aesd_dev), GFP_KERNEL);
	if (!aesd_device) 
	{
		result = -ENOMEM;
		aesd_cleanup_module();
		return result;
	}
	memset(aesd_device, 0, NUM_DEVICES * sizeof(struct aesd_dev));

	for(i = 0; i < NUM_DEVICES; i++ )
	{
		aesd_device[i].c_buffer.temp = NULL;
		aesd_device[i].c_buffer.buf_tail = 0;
		aesd_device[i].c_buffer.buf_head = 0;
		aesd_device[i].c_buffer.buf_count = 0;
		aesd_device[i].c_buffer.temp_count = 0;
		aesd_device[i].size = 0;
		aesd_device[i].c_buffer.buf_of = 0;
		mutex_init(&aesd_device[i].lock);

		result = aesd_setup_cdev(&aesd_device[i], i);
		if(result) 
		{
			unregister_chrdev_region(dev, NUM_DEVICES);
		}
	}
	PDEBUG("Successful");
	return result;

}

void aesd_cleanup_module(void)
{
	int i, j;
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	if(aesd_device)
	{
		for(i = 0; i < NUM_DEVICES; i++)
		{
			cdev_del(&aesd_device[i].cdev);
			for(j = 0; j < BUFF_SIZE; j++)
			{
				if(aesd_device[i].c_buffer.buf_data[j] != NULL)
				{
					kfree(aesd_device[i].c_buffer.buf_data[j]);
					PDEBUG("Freed index : %d", j);
				}
			}
		}
		kfree(aesd_device);
	}
	unregister_chrdev_region(devno, NUM_DEVICES);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
