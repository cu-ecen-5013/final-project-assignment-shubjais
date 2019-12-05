#ifndef MAIN_H_
#define MAIN_H_

/*-----------------------------------------------------------------------------------------
 *File Name 	: main.h
 *Author 	: Shubham Jaiswal
 *Description	: 
 *Modified Date	:
 *Course	: Fall-2019|ECEN-5013|AESD|Final-Project
 *Referneces	: 
 *
------------------------------------------------------------------------------------------*/
/*------------------------------Preprocessor Statements----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define MQ_SIZE (50)
#define MSG_SIZE (2048)
#define MQ_NAME "/aesd_sensor_mq"
#define MQ_SUCCESS (0)
#define MQ_ERROR (1)
/*---------------------------------------------------------------------------------------*/
/*-----------------------------------Global Varibals-------------------------------------*/			

/*---------------------------------------------------------------------------------------*/
/*----------------------------------Function Prototypes---------------------------------*/

static void usage(char *exec_name);

#endif
