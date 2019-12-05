/**
*   @file : logging.c
*   @function : create a logging function      
*
*   @author : Ayush Dhoot
*   @references : 
**/

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

return SOCK_SUCCESS;
}


static void usage(char *exec_name)
{
	syslog(LOG_INFO, "Usage: %s [-d]\n", exec_name);
	printf("Usage: %s [-d]\n", exec_name);
}

