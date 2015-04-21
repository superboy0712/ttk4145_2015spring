/*****************************************************************
 * socket.c
 * memory_mapping_and_data_store
 *
 *  Created on		: Nov 7, 2014 
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
#include "server_thread.h"
#include "config_def.h"

extern pthread_mutex_t signal_master_dead_mtx;
extern pthread_cond_t signal_master_dead_cv;
extern pthread_cond_t signal_server_bind_failed_cv;
typedef void *(*func_ptr)(void *data);
static func_ptr payload = NULL;
static void *payload_para = NULL;
#define error_handle( true , s) \
		do {\
		if(true){ \
		perror(s);\
		exit(EXIT_FAILURE);\
		} } while(0)

typedef struct  {
	pthread_t * tid_p;// to be freed
	int client_fd; //
} th_data_t;
/* number of thread created */
static int n_client = 0;
pthread_mutex_t mutex_n_th = PTHREAD_MUTEX_INITIALIZER;
static const ssize_t buffer_length = 30;
/**
 *
 * @param th_p
 */
void pay_load_register( void *payload_thread(void *), void *parameter){
	payload = payload_thread;
	payload_para = parameter;
}
static void exit_routine(th_data_t * th_p){
	pthread_mutex_lock(&mutex_n_th);
	n_client --;
	pthread_mutex_unlock(&mutex_n_th);
	// print sth to the server
	printf("client %d leaved, %d clients online now!\n",th_p->client_fd,n_client);
	/* close the connection */
	close(th_p->client_fd);
	free(th_p->tid_p);
	free(th_p);
	if(n_client == 0){ /** ensure at least one client **/
		system("gnome-terminal -e ./combine.out");
	}
	pthread_exit(NULL);
}
static void * server( void * th_dp){
	// indicate num ++
	pthread_mutex_lock(&mutex_n_th);
		n_client ++;
	pthread_mutex_unlock(&mutex_n_th);
	// fetch the 'packet'
	th_data_t * p = (th_data_t *)th_dp;
	// print something to the server
	printf("client %d joined, %d clients online now!\n",p->client_fd,n_client);
	// allocating buffer
	char * text;
	text = (char *)malloc(buffer_length);
	if(!text){
		perror("malloc");
		exit_routine(p);
	}
	while(1){
		// read the data
		int rc_lth = read(p->client_fd, text, buffer_length - 1);
		if(rc_lth == 0){
			printf("client close the socket %d\n", p->client_fd);
			free(text);
			exit_routine(p);
		} else if(rc_lth == -1){
			perror("readline");
			free(text);
			exit_routine(p);
		}
		rc_lth = send(p->client_fd, anwser_msg, strlen(anwser_msg)+1, MSG_NOSIGNAL );
		if(rc_lth == 0){
			printf("client close the socket %d\n", p->client_fd);
			free(text);
			exit_routine(p);
		} else if(rc_lth == -1){
			perror("write");
			free(text);
			exit_routine(p);
		}
		printf("user %d: %s\n",p->client_fd, text);

		if(!strcmp(text, "quit\n")){
			free(text);
			exit_routine(p);
		}

	}
	pthread_exit(NULL);//never happen
}

static int server_thread_create_wrapper(int new_client_fd){
	/* create pthread, and aquire th_data */
	th_data_t * th_dp = (th_data_t *)malloc(sizeof(th_data_t));
	error_handle(!th_dp,"malloc:th_dp" );
	th_dp->client_fd = new_client_fd;
	th_dp->tid_p = (pthread_t *)malloc(sizeof(pthread_t));
	if(th_dp->tid_p == NULL){
		perror("malloc:tid_p");
		free(th_dp);
		return -1;
	}
	int rc = pthread_create(th_dp->tid_p, NULL, server, (void *)th_dp);
	if(rc){
		perror("pthread_create");
		free(th_dp->tid_p);
		free(th_dp);
		return -1;
	}
	/* set detach to release resource of thread when client close connection */
	rc = pthread_detach(*(th_dp->tid_p));
	if(rc){
		perror("pthread_detach");
		free(th_dp->tid_p);
		free(th_dp);
		return -1;
	}
	return 1;
}

void *server_main(void * data){
	data = NULL;// keep compiler happy
START:
	pthread_mutex_lock(&signal_master_dead_mtx);
	pthread_cond_wait(&signal_master_dead_cv, &signal_master_dead_mtx);
	pthread_mutex_unlock(&signal_master_dead_mtx);
	struct addrinfo hints, *res_ai;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rc = getaddrinfo(NULL, server_port, &hints, &res_ai);
	error_handle(rc != 0, "getaddrinfo");
	int sock_fd = socket(res_ai->ai_family, res_ai->ai_socktype, res_ai->ai_protocol);
	error_handle(sock_fd == -1, "socket");
	/* bind the address */
	rc = bind(sock_fd, res_ai->ai_addr, res_ai->ai_addrlen);
	if(rc == -1){
		perror("bind");
		sleep(1);
		pthread_mutex_lock(&signal_master_dead_mtx);
		pthread_cond_signal(&signal_server_bind_failed_cv);
		pthread_mutex_unlock(&signal_master_dead_mtx);
		goto START;
	}
	/** the right position to call */
	system("gnome-terminal -e ./combine.out");
	/** the right position to init/run payload */
	if(payload){
		payload(payload_para);
	}else{
		error_handle(1, "payload in server_thread");
	}
	char dst[50];
	const char * temp = inet_ntop(res_ai->ai_family, res_ai->ai_addr, dst, res_ai->ai_addrlen);
	printf("my ip: %s\n ip2: %s\n", temp, dst);
	/*listen! indicate that i am the server*/
	rc = listen(sock_fd, 5);
	if(rc == -1){
		perror("listen");
		pthread_exit((void *)EXIT_FAILURE);
	}
	do{
		struct sockaddr_storage client_name;
		socklen_t client_name_len = sizeof client_name;
		int new_client_fd;

		/* accept a connection */
		new_client_fd = accept(sock_fd, (struct sockaddr *)&client_name, &client_name_len);
		error_handle(new_client_fd == -1, "accept" );
		/* write some welcome message */
		char welcome[108];
		sprintf(welcome,"Welcome! your client fd is %d \n" \
				, new_client_fd);
		write(new_client_fd, welcome, strlen(welcome));

		rc = server_thread_create_wrapper(new_client_fd);
		error_handle(rc == -1, "server_thread_create_wrapper");
		/* handle the connection */
		//is_client_quit = server(client_socket_fd);
		/* close the connection */
		//close(client_socket_fd);

	} while(1);

	/**
	 * never reach here
	 */

	pthread_exit((void *)EXIT_SUCCESS);
}


