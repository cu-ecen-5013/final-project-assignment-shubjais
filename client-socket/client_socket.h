/**
*   @file : client_socket.h
*   @function : headers and defines for creating a client for socket communication over port    
*
*   @author : Ayush Dhoot
*   @references : https://beej.us/guide/bgnet/html/#client-server-background
**/

#ifndef CLIENT_SOCKET_H_
#define CLIENT_SOCKET_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <syslog.h>

#define PORT "9000" // the port client will be connecting to 

#define MAXSIZE 1000 // max number of bytes we can get at once 

#define EXIT_FAIL -1    //error code

int temp_raw;
float C,F;

#endif