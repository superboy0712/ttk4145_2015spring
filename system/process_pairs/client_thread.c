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
#include "client_thread.h"
#include "config_def.h"
const int buffer_size = 100;
const int retry_times = 50;
extern pthread_mutex_t signal_master_dead_mtx;
extern pthread_cond_t signal_master_dead_cv;
extern pthread_cond_t signal_server_bind_failed_cv;
char init_status_and_backup_buffer[buffer_size];
void * client_main(void * data) {
	struct addrinfo hints, *res;
	int sockfd;
	data = NULL;
	sprintf(init_status_and_backup_buffer, "tasks: 0, 0, 0, 0. stop: 0. dir: -1\n");
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(server_addr, server_port, &hints, &res);

	/* reconnect until server established */

	// make a socket:
	int new_socket_count = 0;/* indicates times of reconnection */
NEW_SOCKET:
	/**
	 *  if( new_socket_count > limit ) transit to master/server state
	 */
		if( new_socket_count > retry_times ){
			new_socket_count = 0;
			pthread_mutex_lock(&signal_master_dead_mtx);
			pthread_cond_signal(&signal_master_dead_cv);
			pthread_mutex_unlock(&signal_master_dead_mtx);
			// avoid multiple slaves compete to be master, only the quickest win, the others transit back to slaves
			/**
			 * waiting my server thread signal back. if my server thread failed to be the first,
			 * then server sleep again, clients keep connect to the new spawned server.
			 *
			 * If my server wins, then I am gonna stuck here until the whole program to be killed.
			 *
			 */

			pthread_mutex_lock(&signal_master_dead_mtx);
			pthread_cond_wait(&signal_server_bind_failed_cv, &signal_master_dead_mtx);
			pthread_mutex_unlock(&signal_master_dead_mtx);
			//pthread_exit(EXIT_SUCCESS);
		}
	/**
	 *
	 */
	new_socket_count++;
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd == -1) {
		perror("socket");
		pthread_exit((void *)EXIT_FAILURE);
	}

	int rc = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if(rc != 0){
		perror("connect");
		close(sockfd);
		goto NEW_SOCKET;
	}
	read(sockfd, init_status_and_backup_buffer, buffer_size);
	puts(init_status_and_backup_buffer);
	while(1){
		rc = send(sockfd, inquery_msg, strlen(inquery_msg)+1, MSG_NOSIGNAL);
		if(rc <= 0){
			close(sockfd);
			perror("write");
			goto NEW_SOCKET;
		}
		rc = read(sockfd, init_status_and_backup_buffer, buffer_size);
		if(rc <= 0){
			close(sockfd);
			perror("read");
			goto NEW_SOCKET;
		}
		puts(init_status_and_backup_buffer);

		usleep(inquery_interval);
	}
	pthread_exit((void *)EXIT_SUCCESS);
}
