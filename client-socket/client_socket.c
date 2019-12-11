/**
 *   @file : client_socket.c
 *   @function : create a client for socket communication over port    
 *
 *   @author : Ayush Dhoot
 *   @references : https://beej.us/guide/bgnet/html/#client-server-background
 **/

#include "client_socket.h"

#define DEBUG

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXSIZE];
	char *BUF = buf;
	struct addrinfo hints, *servinfo, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	int len;
	time_t curr_time;
	struct tm *curr_time_tm;
	char curr_time_s[64];

	if (argc != 2) 
	{
		fprintf(stderr,"usage: client hostname\n");
		exit(EXIT_FAIL);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6    
	hints.ai_socktype = SOCK_STREAM;    //TCP

	//getaddrinfo is used to set up the structs 
	if ((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
	{
		//fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		syslog(LOG_ERR, RED "%s: getaddrinfo: %s\n" RESET , __FILE__, gai_strerror(status));
		exit(EXIT_FAIL);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			//perror("client: socket");
			syslog(LOG_ERR, RED "%s: client: socket: %s.\n" RESET , __FILE__, strerror(errno));
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			//perror("client: connect");
			syslog(LOG_ERR, RED "%s: client: connect: %s.\n" RESET , __FILE__, strerror(errno));
			continue;
		}

		break;
	}

	if (p == NULL) 
	{
		//fprintf(stderr, "client: failed to connect\n");
		syslog(LOG_ERR, RED "%s: client: failed to connect: %s" RESET , __FILE__, strerror(errno));
		exit(EXIT_FAIL);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			ipstr, sizeof ipstr);
	//printf("client: connecting to %s\n", ipstr);
	syslog(LOG_DEBUG, GRN "%s: client: connecting to %s\n" RESET , __FILE__, ipstr);


	freeaddrinfo(servinfo); // all done with this structure

	while(1) 
	{
		//strcpy(BUF, "Hello! Client here :)\n");
		temp_raw = temp_sensor_data();
		curr_time = time(NULL);
		curr_time_tm = localtime(&curr_time);
		strftime(curr_time_s , sizeof(curr_time_s), "%c", curr_time_tm);

		if(temp_raw == SENSOR_FAILURE)
		{
			snprintf(buf, MAXSIZE, LOG_ERR_AESD" %s: Cannot Communicate with sensor.\n", curr_time_s);
		} 
		else if (temp_raw == READ_ERROR)
		{
			snprintf(buf, MAXSIZE, LOG_ERR_AESD" %s: Unable to read from sensor.\n", curr_time_s);
		} 
		else
		{
			C = temp_raw * 0.0625;
			F = (1.8 * C) + 32;

			if(C > TEMP_THRESHOLD)
			{
				snprintf(buf, MAXSIZE, LOG_ALERT_AESD" %s: Temperature in F:%f and in C:%f\n",curr_time_s, F, C);
			}
			else
			{
				snprintf(buf, MAXSIZE, LOG_INFO_AESD" %s: Temperature in F:%f and in C: %f\n", curr_time_s, F, C);
			}
		}
		len = strlen(buf);

#ifdef DEBUG
		syslog(LOG_INFO, YEL "%s: string length: %d\n" RESET, __FILE__, len);
#endif
		if ((numbytes = send(sockfd, BUF, len, 0)) == -1) 
		{
			//perror("send");
			syslog(LOG_ERR, RED "%s: client: send:%s.\n" RESET , __FILE__, strerror(errno));
			exit(EXIT_FAIL);
		}

#ifdef DEBUG
		syslog( LOG_INFO, YEL "%s: Sent data to client. Waiting for response.\n" RESET , __FILE__);
#endif

		if ((numbytes = recv(sockfd, buf, MAXSIZE-1, 0)) == -1) 
		{
			//perror("recv");
			syslog(LOG_ERR, RED "%s: client: recv: %s\n" RESET, __FILE__, strerror(errno));
			exit(EXIT_FAIL);
		}

		buf[numbytes] = '\0';

		syslog( LOG_INFO, YEL "%s: client: received %s\n" RESET, __FILE__, buf);  

		sleep(5);
	}
	close(sockfd);

	return 0;
}
