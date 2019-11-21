#ifndef SOCK_SERVER_H_
#define SOCK_SERVER_H_


/*-----------------------------------------------------------------------------------------
 *File Name 	: sockserver.h
 *Author 	: Shubham Jaiswal
 *Description	: Program to start a socket on port 9000
 *Modified Date	:
 *Result/Return	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: 
 *
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include <signal.h>
#include "socket.h"

/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			

/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Definitions---------------------------------*/

static void usage(char *);
void sig_handler(int signo);

#endif
