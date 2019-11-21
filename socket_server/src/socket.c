/*-----------------------------------------------------------------------------------------
 *File Name 	: socket.c
 *Author 	: Shubham Jaiswal
 *Description	: 
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
 *
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include "../inc/socket.h"
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			

/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/

/*Function to get a socket descriptor, make the socket non-blocking, 
 * set socket options, bind the socket to port and start the daemon
 * */

int socket_init(int d_flag)
{
	int sock_fd, ret;
	int status, activate = 1;
	struct addrinfo hints, *res, *addr_ptr;
		
	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE;

	if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) //point the res to head of structure linked list filled with possible addresses
	{
		printf("%s: getaddrinfo error: %s\n", __FILE__,  gai_strerror(status));
    		syslog(LOG_ERR, "%s: getaddrinfo error: %s\n", __FILE__,  gai_strerror(status));
    		return SOCK_ERROR;
	}	
	
	addr_ptr = res;
	while(addr_ptr != NULL) //Iterate over the linked list
	{
		if ((sock_fd = socket(addr_ptr->ai_family, addr_ptr->ai_socktype, addr_ptr->ai_protocol)) == -1) //create a socket
		{
            		syslog(LOG_INFO, "%s: server:socket:%s.\n", __FILE__, strerror(errno));
            		continue;
        	}

		status = fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK); // Make the socket non-blocking
		if (status == -1)
		{
  			syslog(LOG_ERR, "%s: Error while making socket non blocking:%s\n", __FILE__, strerror(errno));
  			exit(SOCK_ERROR);
		}

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &activate, sizeof(int)) == -1)  // Set socket options to allow bind calls to reuse address
		{
            		syslog(LOG_ERR, "%s: setsockopt:%s.\n", __FILE__, strerror(errno));
            		return SOCK_ERROR;
        	}

        	if (bind(sock_fd, addr_ptr->ai_addr, addr_ptr->ai_addrlen) == -1) // check if socket is able to bind on the address passed
		{
            		close(sock_fd);
            		syslog(LOG_ERR, "%s: server: bind:%s\n", __FILE__, strerror(errno));
            		continue;
        	}

        break;
		
		addr_ptr = addr_ptr->ai_next;
	}
	
	freeaddrinfo(res);// free the dynamically created linked list

    	if (addr_ptr == NULL)  
	{
        	syslog(LOG_ERR, "%s: server: failed to bind\n", __FILE__);
        	exit(SOCK_ERROR);
    	}
	else if(d_flag)
	{
		pid_t pid;
		pid = fork ();
		if (pid > 0) //Kill Parent
		{
			exit(SOCK_SUCCESS);
		}	
		else if (!pid) //Start Daemon
		{
			if (setsid () == -1)
			{
				close(sock_fd);
				return SOCK_ERROR;
			}

			if (chdir ("/") == -1)
			{
				close(sock_fd);
				return SOCK_ERROR;
			}
			
			ret = socket_io(sock_fd, BACKLOG);
		}	
		else if (pid == -1)
		{
			close(sock_fd);
			syslog(LOG_ERR, "%s: fork:%s\n", __FILE__, strerror(errno));
			exit(SOCK_ERROR);
		}
	}
	else
	{
		ret = socket_io(sock_fd, BACKLOG);
	}

	return ret;
}
