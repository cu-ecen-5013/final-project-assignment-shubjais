/**
 *   @file : client_socket.h
 *   @function : headers and defines for creating a client for socket communication over port    
 *
 *   @author : Ayush Dhoot
 *   @references : https://beej.us/guide/bgnet/html/#client-server-background
 **/

#ifndef CLIENT_SOCKET_H_
#define CLIENT_SOCKET_H_

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <time.h>
#include "tempsens.h"

#define PORT                "9000" // the port client will be connecting to 
#define MAXSIZE             1000 // max number of bytes we can get at once 
#define EXIT_FAIL           -1    //error code
#define LOG_ERR_AESD        "1:"
#define LOG_INFO_AESD       "2:"
#define LOG_ALERT_AESD      "3:"
#define TEMP_THRESHOLD_HIGH (30)
#define TEMP_THRESHOLD_LOW  (20)

#define SENSOR_NAME         "TMP102"

int temp_raw;
float C,F;

#endif
