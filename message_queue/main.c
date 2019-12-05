/*-----------------------------------------------------------------------------------------
 *File Name 	: main.c
 *Author 	: Shubham Jaiswal
 *Description	: Implements message queues to transfer data from socket utility to message queues
		: which can then be read by logger utility to pass to char driver. 
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: Linux Programming Interface-Michael Kerrisk-Chapter 52
 *		: https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include "main.h"
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			

/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/

int main(int argc, char *argv[])
{
	int flags, opt;
	mode_t permissions;
	mqd_t mq_des;
	struct mq_attr attr, *attrp = NULL;
	char *mq_name = MQ_NAME;

	attr.mq_maxmsg = MQ_SIZE;
	attr.mq_msgsize = MSG_SIZE;
	flags = O_RDWR | O_CREAT;
	
	while(1)
	{
		opt = getopt(argc, argv, "m:s:");
		
		if(opt == -1)
		{
			break;
		}
		
		switch(opt)
		{
			case 'm':
				syslog(LOG_DEBUG, "%s: Option -m specified. Changing message queue size to argument provided.\n", __FILE__);
				attr.mq_maxmsg = atoi(optarg);
				break;

			case 's':
				syslog(LOG_DEBUG, "%s: Option -s specified. Changing allowed size of messages to argument provided.\n", __FILE__);
				attr.mq_msgsize = atoi(optarg);
				break;

			case 'n':
				syslog(LOG_DEBUG, "%s: Option -n specified. Using provided message queue name.", __FILE__);
				mq_name = optarg;
				break;
			
			case '?':
				syslog(LOG_ERR, "%s: Unknown option specified. Exiting!\n", __FILE__);
				usage(argv[0]);
				return MQ_ERROR;
			
			default:
				usage(argv[0]);
				return MQ_ERROR;
		}
	}

	if(argc > optind)
	{
		syslog(LOG_ERR, "%s: Non-option arguments passed. Exiting!\n", __FILE__);
		usage(argv[0]);
		return MQ_ERROR;	
	}
	else
	{
		permissions = S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP;
		attrp = &attr;
		mq_des = mq_open(mq_name, flags, permissions, attrp);
		if(mq_des == (mqd_t)-1)
		{
			syslog(LOG_ERR, "%s:mq_open:%s\n", __FILE__, strerror(errno));
			exit(MQ_ERROR);
		}
	}

return MQ_SUCCESS;
}

static void usage(char *exec_name)
{
	syslog(LOG_INFO, "Usage: %s [-m maxmsg] [-s msgsize] [-n mq-name]\n", exec_name);
	printf("Usage: %s [-m maxmsg] [-s msgsize] [-n mq-name]\n", exec_name);
}

