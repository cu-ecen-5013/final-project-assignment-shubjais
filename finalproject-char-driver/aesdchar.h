/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes, Ayush Dhoot and Shubham Jaiswal
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#else
     /* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#  define PDEBUG(fmt, args...)/* not debugging: nothing */
#endif

#define BUFF_SIZE (1024)
#define NUM_DEVICES (3)

struct c_buff
{
	unsigned int buf_of;
	unsigned int buf_head;
	unsigned int buf_tail;
	unsigned int buf_count;
	char * temp;
	unsigned int temp_count;
	char *buf_data[BUFF_SIZE];
	int buf_data_size[BUFF_SIZE];
};

struct aesd_dev
{
	unsigned long size;
	struct c_buff c_buffer;
	struct mutex lock;
	struct cdev cdev;	  /* Char device structure		*/
};

void aesd_cleanup_module(void);
#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */
