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

#define LOGGER_QUEUE "/aesdqueue"
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			
mqd_t log_mq_des;
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

	if((log_mq_des = mq_open(LOGGER_QUEUE, O_WRONLY)) == -1)
	{
		syslog(LOG_ERR, "%s: mq_open:%s\n", __FILE__, strerror(errno));
		return MQ_ERROR;
	}

	if (listen(sock_fd, backlog) == -1) 
	{
		syslog(LOG_ERR, "%s: listen:%s\n", __FILE__, strerror(errno));
		return SOCK_ERROR;
	}

	node_t *node_ptr = NULL;
	SLIST_HEAD(nodehead, node) head;
	SLIST_INIT(&head);

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

		if (mq_close (log_mq_des) == -1) {
			syslog(LOG_ERR, "%s:mq_close:%s\n", __FILE__, strerror(errno));
			exit (1);
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
	int send_size = 0;
	data_t *arg_data = (data_t *)args;
	char *s = arg_data->ip;
	int new_fd = arg_data->s_new_fd; 
	char *input_buffer_ptr = NULL, *read_buffer_ptr = NULL;
	ssize_t ret, size;

	syslog(LOG_DEBUG, "%s:Thread Id: %ld\n", __FILE__, arg_data->thread_id);
	syslog(LOG_INFO, "%s: MESSAGE QUEUE NAME: %s\n", __FILE__, LOGGER_QUEUE);

	while(1)
	{
		read_buffer_ptr = (char *)calloc(SIZE, sizeof(char));
		input_buffer_ptr = read_buffer_ptr;
		size = SIZE;
		while(1)
		{
			ret = 0;
			send_size = 0;
			ret = recv(new_fd, input_buffer_ptr, SIZE, 0);
			syslog(LOG_DEBUG, "%s:ret:%ld\n",__FILE__, ret);
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


		if(mq_send(log_mq_des, read_buffer_ptr, send_size, 0) == -1)
		{
			syslog(LOG_ERR, "%s: mq_send:%s", __FILE__, strerror(errno));

		}

		free(read_buffer_ptr);

		send_size = strlen("Received!");
		read_buffer_ptr = (char *)calloc(send_size, sizeof(char));

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
	}
	if(close(new_fd) == -1)
	{
		syslog(LOG_ERR, "%s: Error closing connection:%s\n", __FILE__, strerror(errno));
		exit(SOCK_ERROR);
	}

	syslog(LOG_INFO, "%s: Closed connection from %s.\n", __FILE__, s);

	arg_data->s_completion_flag = 1; //flag to denote completion
	return NULL;
}


