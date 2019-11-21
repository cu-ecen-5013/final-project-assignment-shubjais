#ifndef SOCKET_IO_H_
#define SOCKET_IO_H_

/*-----------------------------------------------------------------------------------------
 *File Name 	: socket_io.h
 *Author 	: Shubham Jaiswal
 *Description	: 
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD
 *Referneces	: 
 *
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include <pthread.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define SOCK_ERROR (-1)
#define SOCK_SUCCESS (0)
#define TRUE (1)
#define FALSE (0)
#define SIZE (100)
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			
extern volatile sig_atomic_t sigint_flag;
extern volatile sig_atomic_t sigterm_flag;

typedef struct data
{
	pthread_t thread_id;
	int s_completion_flag;
	int s_new_fd;
	char ip[INET6_ADDRSTRLEN];
}data_t;

typedef struct node
{
	data_t data;
	SLIST_ENTRY(node) nodes;
}node_t;

#ifndef USE_AESD_CHAR_DEVICE
pthread_mutex_t lock;
#endif
/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/
int socket_io(int sock_fd, int backlog);
void *get_in_addr(struct sockaddr *sa);
void prc_closure(int );
void *thread_sock(void *);
#endif
