/**
*   @file : client_socket.c
*   @function : create a client for socket communication over port    
*
*   @author : Ayush Dhoot
*   @references : https://beej.us/guide/bgnet/html/#client-server-background
**/

#include "client_socket.h"
#include "tempsens.h"

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

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(EXIT_FAIL);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        //IPv4 or IPv6    
    hints.ai_socktype = SOCK_STREAM;    //TCP

    //getaddrinfo is used to set up the structs 
    if ((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        syslog(LOG_ERR, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAIL);
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            syslog(LOG_ERR, "client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            syslog(LOG_ERR, "client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        syslog(LOG_ERR, "client: failed to connect");
        exit(EXIT_FAIL);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            ipstr, sizeof ipstr);
    printf("client: connecting to %s\n", ipstr);
    syslog(LOG_DEBUG, "client: connecting to %s\n", ipstr);


    freeaddrinfo(servinfo); // all done with this structure

    while(1) {
    //BUF = "Hello! Client here :)";
        temp_raw = temp_sensor_data();

        C = temp_raw*0.0625;
        F = (1.8 * C) + 32;

        sprintf(buf,"Temperature in F:%f and in C: %f\n", F, C);
        
        len = strlen(buf);

        if ((numbytes = send(sockfd, BUF, len, 0)) == -1) {
            perror("send");
            syslog(LOG_ERR, "client: send");
            exit(EXIT_FAIL);
            }

        if ((numbytes = recv(sockfd, buf, MAXSIZE-1, 0)) == -1) {
            perror("recv");
            syslog(LOG_ERR, "client: recv");
            exit(EXIT_FAIL);
        }

        buf[numbytes] = '\0';

        printf("client: received '%s'\n",buf);  

        sleep(3);
    }
    close(sockfd);

    return 0;
}