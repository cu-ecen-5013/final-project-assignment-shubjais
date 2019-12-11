#ifndef LOGGING_H_
#define LOGGING_H_

/**
*   @file : logging.h
*   @function : file contains variable and function declarations for logging.c      
*
*   @author : Ayush Dhoot
*   @references : 
**/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LG_SUCCESS (0)
#define LG_ERROR (1)

#define LOG_ERR_AESD    "1:"
#define LOG_INFO_AESD   "2:"
#define LOG_ALERT_AESD  "3:"

#define ERR_LEVEL       1
#define INFO_LEVEL      2
#define ALERT_LEVEL     3
#define TRUE            0
#define FALSE           1


#define MQ_SIZE (20)
#define MSG_SIZE (1024)
#define MQ_NAME "/aesdqueue"
#define MQ_SUCCESS (0)
#define MQ_ERROR (1)

//#define log_file        "/var/tmp/aesdchar"
#define log_err        "/dev/aesdchar0"
#define log_info       "/dev/aesdchar1"
#define log_alert      "/dev/aesdchar2"

char msg[MSG_SIZE];
char *msg_ptr;
void usage(char *);

void data_logging(int mode);

#endif
