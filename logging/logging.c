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
		
	}

return LG_SUCCESS;
}


static void usage(char *exec_name)
{
	syslog(LOG_INFO, "Usage: %s [-d]\n", exec_name);
	printf("Usage: %s [-d]\n", exec_name);
}

void data_logging(struct log_data)
{
	switch (log_data.mode)
	{
	case LOG_ERR_AESD :
		{
			log_file = fopen(log_file ,"a");
			fprintf(log_file, "%s	%s", . , . ); //Date  Time
			fprintf(log_file, "	%s[%d]", . , . ); //Process name[pid]
			fprintf(log_file, "	%s", .);	//Value
			fprintf(log_file, "\n");
			fclose(log_file);
		}
		break;

	case LOG_INFO_AESD :
		{
			log_file = fopen(log_file ,"a");
			fprintf(log_file, "%s	%s", . , . ); //Date  Time
			fprintf(log_file, "	%s[%d]", . , . ); //Process name[pid]
			fprintf(log_file, "	%s", .);	//Value
			fprintf(log_file, "\n");
			fclose(log_file);
		}
		break;
	
	case LOG_ALERT_AESD :
		{
			log_file = fopen(log_file ,"a");
			fprintf(log_file, "%s	%s", . , . ); //Date  Time
			fprintf(log_file, "	%s[%d]", . , . ); //Process name[pid]
			fprintf(log_file, "	%s", .);	//Value
			fprintf(log_file, "\n");
			fclose(log_file);
		}
		break;

	default:
		break;
	}
}
