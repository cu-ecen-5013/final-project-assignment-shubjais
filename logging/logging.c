/**
*   @file : logging.c
*   @function : create a logging process   
*
*   @author : Ayush Dhoot
*   @references : 
**/

#include "logging.h"

int main(int argc, char *argv[])
{
	int opt;
	int d_flag = FALSE;
	int flags;
	int mode;

	mode_t permissions;
	mqd_t mq_des;
	struct mq_attr attr, *attrp = NULL;
	char *mq_name = MQ_NAME;

	attr.mq_maxmsg = MQ_SIZE;
	attr.mq_msgsize = MSG_SIZE;
	flags = O_RDONLY | O_CREAT;

	while(1)
	{
		opt = getopt(argc, argv, "d");
		
		if(opt == -1)
		{
			break;
		}
		
		switch(opt)
		{
			case 'd':
				syslog(LOG_DEBUG, "%s: Option -d specified. A daemon will be created\n", __FILE__);
				d_flag = TRUE;
				break;
			
			case '?':
				syslog(LOG_ERR, "%s: Unknown option specified. Exiting!\n", __FILE__);
				usage(argv[0]);
				return LG_ERROR;
			
			default:
				usage(argv[0]);
				return LG_ERROR;
		}
	}

	if(argc > optind)
	{
		syslog(LOG_ERR, "%s: Non-option arguments passed. Exiting!\n", __FILE__);
		usage(argv[0]);
		return LG_ERROR;	
	}
	else
	{	
		if(d_flag == TRUE)
		{
			pid_t pid;
			pid = fork ();
			if (pid > 0) //Kill Parent
			{
				exit(LG_SUCCESS);
			}	
			else if (!pid) //Start Daemon
			{
				if (setsid () == -1)
				{
					return LG_ERROR;
				}

				if (chdir ("/") == -1)
				{
					return LG_ERROR;
				}
			}

		else if (pid == -1)
		{
			syslog(LOG_ERR, "%s: fork:%s\n", __FILE__, strerror(errno));
			exit(EXIT_FAILURE);
		}
		}
		permissions = S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP;
		attrp = &attr;
		mq_des = mq_open(mq_name, flags, permissions, attrp);
		if(mq_des == (mqd_t)-1)
		{
			syslog(LOG_ERR, "%s:mq_open:%s\n", __FILE__, strerror(errno));
			exit(MQ_ERROR);
		}
		
		while(1)
		{
		int msg_size = mq_receive(mq_des, msg, MSG_SIZE, NULL); 
		if(msg_size == -1)
		{
			syslog(LOG_ERR, "%s:mq_receive:%s\n", __FILE__, strerror(errno));
            exit(MQ_ERROR);
        }

		msg[msg_size] = '\0';

		if(strstr(msg, LOG_ERR_AESD))
		{
			mode = ERR_LEVEL;
		}
		else if(strstr(msg, LOG_INFO_AESD))
		{
			mode = INFO_LEVEL;
		}
		else if(strstr(msg, LOG_ALERT_AESD))
		{
			mode = ALERT_LEVEL;
		}
		else
		{
			exit(EXIT_FAILURE);
		}

		msg_ptr = (msg + strlen(LOG_ERR_AESD));
		msg_size -= strlen(LOG_ERR_AESD); 
		data_logging(mode);
		}
	}

	return LG_SUCCESS;
}


void usage(char *exec_name)
{
	syslog(LOG_INFO, "Usage: %s [-d]\n", exec_name);
	printf("Usage: %s [-d]\n", exec_name);
}

void data_logging(int mode)
{
	switch (mode)
	{
	case ERR_LEVEL :
		{
			FILE *fp = fopen(log_err ,"a");
			fprintf(fp, "%s", msg_ptr);
			fclose(fp);
		}
		break;

	case INFO_LEVEL :
		{
			FILE *fp = fopen(log_info ,"a");
			fprintf(fp, "%s", msg_ptr);
			fclose(fp);
		}
		break;
	
	case ALERT_LEVEL :
		{
			FILE *fp = fopen(log_alert ,"a");
			fprintf(fp, "%s", msg_ptr);
			fclose(fp);
		}
		break;

	default:
		break;
	}

	return;
}
