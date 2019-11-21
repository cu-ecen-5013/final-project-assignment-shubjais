/*-----------------------------------------------------------------------------------------
 *File Name 	: sock_server.c
 *Author 	: Shubham Jaiswal
 *Description	: Program to start a socket on port 9000
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: https://beej.us/guide/bgnet/html/single/bgnet.html#simpleserver
 * 		: Assignments 5 & 6
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/

#include "../inc/sock_server.h"
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			
volatile sig_atomic_t  sigint_flag = FALSE;
volatile sig_atomic_t sigterm_flag = FALSE;
/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/
int main(int argc, char *argv[])
{
	int opt;
	int daemon_flag = FALSE;

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
				daemon_flag = TRUE;
				break;
			
			case '?':
				syslog(LOG_ERR, "%s: Unknown option specified. Exiting!\n", __FILE__);
				usage(argv[0]);
				return SOCK_ERROR;
			
			default:
				usage(argv[0]);
				return SOCK_ERROR;
		}
	}

	if(argc > optind)
	{
		syslog(LOG_ERR, "%s: Non-option arguments passed. Exiting!\n", __FILE__);
		usage(argv[0]);
		return SOCK_ERROR;	
	}
	else
	{

		if (signal (SIGINT, sig_handler) == SIG_ERR) 
		{
			printf ("Cannot handle SIGINT!\n");
			exit (SOCK_ERROR);
		}

		if (signal (SIGTERM, sig_handler) == SIG_ERR) 
		{
			printf("Cannot handle SIGTERM!\n");
			exit (SOCK_ERROR);
		}
		
		syslog(LOG_DEBUG, "%s: Initializing socket server.\n", __FILE__);
		
		if (socket_init(daemon_flag) == -1)
		{
			return SOCK_ERROR;
		}
	}

return SOCK_SUCCESS;
}


static void usage(char *exec_name)
{
	syslog(LOG_INFO, "Usage: %s [-d]\n", exec_name);
	printf("Usage: %s [-d]\n", exec_name);
}

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		syslog(LOG_INFO, "Interrupt Signal Received!\n");
		sigint_flag = TRUE;
		printf("\n");
	}
	else if (signo == SIGTERM)
	{
		syslog(LOG_INFO, "Terminate signal Received!\n");
		sigterm_flag = TRUE;
	}
	else 
	{
		syslog(LOG_ERR, "Unexpected signal occured in handle!\n");
		exit(SOCK_ERROR);
	}
}
