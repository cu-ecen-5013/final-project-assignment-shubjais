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

#define LG_SUCCESS (0)
#define LG_ERROR (1)

#define LOG_ERR     0
#define LOG_DATA    1
#define LOG_ALERT   2

FILE *log_file;

static void usage(char *);

void data_logging(struct );

#endif
