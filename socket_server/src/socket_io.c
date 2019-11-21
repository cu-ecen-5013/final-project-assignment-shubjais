/*-----------------------------------------------------------------------------------------
 *File Name 	: socket_io.c
 *Author 	: Shubham Jaiswal
 *Description	: 
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: Assignments 5 & 6 
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include <arpa/inet.h>
#include <sys/stat.h>
#include "../inc/socket_io.h"

#define FILE_PATH "/var/tmp/aesdsocketdata"

/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			

/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/

void *get_in_addr(struct sockaddr *sa)
{
    	if (sa->sa_family == AF_INET) 
	{
        	return &(((struct sockaddr_in*)sa)->sin_addr);
    	}

    	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int socket_io(int sock_fd, int backlog)
{
	int new_fd;
	struct sockaddr_storage peer_addr;
	char s[INET6_ADDRSTRLEN];
	socklen_t sin_size = sizeof peer_addr;
    	
	if (listen(sock_fd, backlog) == -1) 
	{
        	syslog(LOG_ERR, "%s: listen:%s\n", __FILE__, strerror(errno));
        	return SOCK_ERROR;
    	}

	node_t *node_ptr = NULL;
	SLIST_HEAD(nodehead, node) head;
    	SLIST_INIT(&head);
	
#ifndef USE_AESD_CHAR_DEVICE
	if(pthread_mutex_init(&lock, NULL) != 0)
    	{
        	syslog(LOG_ERR, "%s: mutex init failed. %s\n", __FILE__, strerror(errno));
        	return SOCK_ERROR;
    	}
#endif

	syslog(LOG_INFO, "%s: server: Waiting for connections.\n", __FILE__);
	while(1)
	{
	       	new_fd = accept(sock_fd, (struct sockaddr *)&peer_addr, &sin_size);
        	if (new_fd == -1) 
		{	
			SLIST_FOREACH(node_ptr, &head, nodes) 
			{
        			if(node_ptr->data.s_completion_flag == 1)
				{
					syslog(LOG_DEBUG, "%s: Thread cleared: %ld", __FILE__, node_ptr->data.thread_id);
					pthread_join(node_ptr->data.thread_id, NULL);
					SLIST_REMOVE(&head, node_ptr, node, nodes);
					free(node_ptr);
				}
    			}

			if(SLIST_EMPTY(&head))
			{
				//syslog(LOG_INFO, "%s: linked list emptied.\n", __FILE__);
            			prc_closure(sock_fd);
			}
			//syslog(LOG_ERR, "%s: accept:%s\n", __FILE__, strerror(errno));
            		continue;
        	}
	
		inet_ntop(peer_addr.ss_family,get_in_addr((struct sockaddr *)&peer_addr), s, sizeof s);
        	syslog(LOG_INFO, "%s: Accepted connection from %s\n",__FILE__, s);			
		
		node_ptr = (node_t *)malloc(sizeof(node_t));
		if(node_ptr == NULL)
		{
			syslog(LOG_ERR, "%s: malloc: Cannot allocate memory.%s\n", __FILE__, strerror(errno));
			exit(SOCK_ERROR);
		}

        	node_ptr->data.s_completion_flag = 0;
		node_ptr->data.s_new_fd = new_fd;
        	strcpy(node_ptr->data.ip, s); 
		SLIST_INSERT_HEAD(&head, node_ptr, nodes);
		
		pthread_create(&node_ptr->data.thread_id, NULL, thread_sock, (void *)&node_ptr->data);
		syslog(LOG_INFO, "%s: server: Waiting for connections.\n", __FILE__);
	}
}


void prc_closure(int sock_fd)
{
	if (sigint_flag == TRUE || sigterm_flag == TRUE)
        {
      		syslog(LOG_INFO, "%s:Closing Socket and deleting file!\n", __FILE__ );

               	if(remove(FILE_PATH) == 0)
             	{
 	                syslog(LOG_INFO, "%s: File deleted successfully.\n", __FILE__);
       	      	 }
               	else
               	{
                        syslog(LOG_INFO, "%s: Error deleting file:%s\n", __FILE__, strerror(errno));
                      	exit(SOCK_ERROR);
              	}

		if(close(sock_fd) == -1)
             	{
        	      	 syslog(LOG_ERR, "%s: Error closing Socket:%s\n", __FILE__, strerror(errno));
            		 exit(SOCK_ERROR);
             	}
		
               	exit(SOCK_SUCCESS);
       	}

}


void *thread_sock(void *args)
{
	int fd, send_size = 0;
	data_t *arg_data = (data_t *)args;
	char *s = arg_data->ip;
	int new_fd = arg_data->s_new_fd; 
	char *input_buffer_ptr = NULL, *read_buffer_ptr = NULL;
 	off_t f_size = 0;
	ssize_t ret, size;

	syslog(LOG_DEBUG, "%s:Thread Id: %ld\n", __FILE__, arg_data->thread_id);
	syslog(LOG_INFO, "%s: FILE PATH: %s\n", __FILE__, FILE_PATH);

	fd = open(FILE_PATH, O_RDWR | O_APPEND | O_CREAT, 0664);
	if(fd == -1)
	{
		syslog(LOG_ERR, "%s: Error in opening file: %s\n", __FILE__, strerror(errno));
		exit(SOCK_ERROR);
	}
	
	syslog(LOG_INFO, "%s: File Descriptor: %d\n", __FILE__, fd);
	read_buffer_ptr = (char *)calloc(SIZE, sizeof(char));
	input_buffer_ptr = read_buffer_ptr;
	size = SIZE;
	while(1)
	{
		ret = 0;
		ret = recv(new_fd, input_buffer_ptr, SIZE, 0);
		syslog(LOG_DEBUG, "ret:%ld\n", ret);
		if (ret  == -1) 
		{
			if(errno == EINTR)
				continue;
        		syslog(LOG_ERR, "%s: recv:%s\n", __FILE__, strerror(errno));
       	 		exit(SOCK_ERROR);
    		}
		else if((*(input_buffer_ptr + ret - 1) == '\n') || (ret == 0))
		{
			send_size += ret;
			break;
		}
		else if(ret == SIZE)
		{
			read_buffer_ptr = realloc(read_buffer_ptr, (size + SIZE));
			if(read_buffer_ptr == NULL)
			{
				syslog(LOG_ERR, "%s: realloc:%s.\n", __FILE__, strerror(errno));
				exit(SOCK_ERROR);
			}
			input_buffer_ptr = read_buffer_ptr + size;
			size += SIZE;
			send_size += ret;
		}
		else
		{
			input_buffer_ptr += ret;
		       	send_size += ret;	
		}
	}
			
	input_buffer_ptr = read_buffer_ptr;

	pthread_mutex_lock(&lock);
		while (send_size != 0 && (ret = write (fd, input_buffer_ptr, send_size)) != 0) 
		{
			if (ret == -1) 
			{
				if (errno == EINTR)
					continue;
				syslog(LOG_ERR, "%s: write:%s\n", __FILE__, strerror(errno));
					break;
			}
		send_size -= ret;
		input_buffer_ptr += ret;
		}
		
	free(read_buffer_ptr);
	
	off_t curr_pos = lseek(fd, (off_t)0, SEEK_CUR);
	syslog(LOG_INFO,"%s:Current Position:%ld\n", __FILE__, curr_pos);
	f_size = lseek(fd, (off_t) 0, SEEK_END);
	if(f_size == (off_t)-1)
	{
		syslog(LOG_ERR, "%s: lseek end:%s.\n", __FILE__, strerror(errno));
		exit(SOCK_ERROR);
	}
			
	send_size = (int)f_size - curr_pos;

	ret = lseek(fd, (off_t)curr_pos, SEEK_SET);
	if(ret == (off_t)-1)
	{
		syslog(LOG_ERR, "%s: lseek start:%s.\n", __FILE__, strerror(errno));
		exit(SOCK_ERROR);
	}

	syslog(LOG_INFO, "%s: Read size:%d.\n", __FILE__, send_size);

	read_buffer_ptr = (char *)calloc(send_size, sizeof(char));
	if(read_buffer_ptr == NULL)
	{
		syslog(LOG_ERR, "%s: Error allocating memory for read.%s\n", __FILE__, strerror(errno));
	}
	
	input_buffer_ptr = read_buffer_ptr;

	while (send_size != 0 && (ret = read (fd, input_buffer_ptr, send_size)) != 0) 
	{
		if (ret == -1) 
		{
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "%s: read:%s\n", __FILE__, strerror(errno));
			break;
		}
	
			send_size -= ret;
			input_buffer_ptr += ret;
	}

	pthread_mutex_unlock(&lock);
	
	if(close(fd) == -1)
	{
		syslog(LOG_ERR, "%s: Error closing %s file:%s\n", __FILE__, FILE_PATH, strerror(errno));
		exit(SOCK_ERROR);
	}

	send_size = f_size - curr_pos;

	syslog(LOG_INFO, "%s: Send Data Size: %d.\n", __FILE__, send_size);
	input_buffer_ptr = read_buffer_ptr;
	while (send_size != 0 && (ret = send(new_fd, input_buffer_ptr, send_size, 0)) != 0) 
	{
		if (ret == -1) 
		{
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "%s: read:%s\n", __FILE__, strerror(errno));
			break;
		}
	
		send_size -= ret;
		input_buffer_ptr += ret;
	}
	
	free(read_buffer_ptr);

	if(close(new_fd) == -1)
	{
		syslog(LOG_ERR, "%s: Error closing connection:%s\n", __FILE__, strerror(errno));
		exit(SOCK_ERROR);
	}

	syslog(LOG_INFO, "%s: Closed connection from %s.\n", __FILE__, s);

	arg_data->s_completion_flag = 1; //flag to denote completion
	return NULL;
}


