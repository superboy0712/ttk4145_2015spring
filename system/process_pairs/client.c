/*****************************************************************
 * client.c
 * process_pairs
 *
 *  Created on		: Feb 20, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  Description		:
 *****************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
/* some constants */
const useconds_t inquery_interval = 100000;
const char * inquery_msg = "Are u alive?\n";
const char * server_addr = "localhost";
const char * server_port = "50000";
const int buffer_size = 100;
int main(void) {
	struct addrinfo hints, *res;
	int sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(server_addr, server_port, &hints, &res);
	char buffer[buffer_size];
	/* reconnect until server established */

	// make a socket:
	int new_socket_count = 0;/* indicates times of reconnection */
NEW_SOCKET:
	/**
	 *  if( new_socket_count > limit ) transit to master/server state
	 */
	new_socket_count++;
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	do {
		// connect!
		int rc = connect(sockfd, res->ai_addr, res->ai_addrlen);
		if(rc != 0){
			perror("connect");
		}else{
			break;
		}
		usleep(inquery_interval);
	} while(1);
	read(sockfd, buffer, buffer_size);
	puts(buffer);
	int rc;
	while(1){
		rc = send(sockfd, inquery_msg, strlen(inquery_msg)+1, MSG_NOSIGNAL);
		if(rc <= 0){
			close(sockfd);
			perror("write");
			goto NEW_SOCKET;
		}
		rc = read(sockfd, buffer, buffer_size);
		if(rc <= 0){
			close(sockfd);
			perror("read");
			goto NEW_SOCKET;
		}
		puts(buffer);

		usleep(inquery_interval);
	}
	return 1;
}
