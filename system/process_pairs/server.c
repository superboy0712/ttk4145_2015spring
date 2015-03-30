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
#define error_handle( true , s) \
		do {\
		if(true){ \
		perror(s);\
		exit(EXIT_FAILURE);\
		} } while(0)

const char * SERVER_PORT = "50000";
/**
 * read from file, read until a \n or max_length.
 * return : chars read, 0 indicates EOF. -1 means error
 */
int readline(int fd, void * buffer, ssize_t max_length){
	if(!buffer||max_length<0){
		perror("readline: bad buffer or length!");
		return -1;
	}
	char c;
	char * buffer_index = buffer;
	int read_bytes;
	int ret_count = 0;
	do{
		read_bytes = read(fd,&c,1);
		if(read_bytes == 1){
			sprintf(buffer_index,"%c",c);
			ret_count ++;
			buffer_index++;
		}
		else if(read_bytes < 0){
			/* error */
			perror("readline.read: bad read");
			return -1;
		}
		else if(read_bytes == 0){
			/* end of line */
			perror("nothing to read");
			return 0;
		}
		if(ret_count >= max_length)
			break;
	} while( c != '\n');
	*buffer_index = '\0';
	return ret_count;
}
const int MAX_Thread = 5;
typedef struct  {
	pthread_t * tid_p;// to be freed
	int client_fd; //
} th_data_t;
//th_data_t th_data;
/* number of thread created */
static int n_run_thread = 0;
pthread_mutex_t mutex_n_th = PTHREAD_MUTEX_INITIALIZER;
static const ssize_t buffer_length = 30;
static void exit_routine(th_data_t * th_p){
	pthread_mutex_lock(&mutex_n_th);
	n_run_thread --;
	pthread_mutex_unlock(&mutex_n_th);
	// print sth to the server
	printf("client %d leaved, %d clients online now!\n",th_p->client_fd,n_run_thread);
	/* close the connection */
	close(th_p->client_fd);
	free(th_p->tid_p);
	free(th_p);
	pthread_exit(NULL);
}
void * server( void * th_dp){
	// indicate num ++
	pthread_mutex_lock(&mutex_n_th);
		n_run_thread ++;
	pthread_mutex_unlock(&mutex_n_th);
	// fetch the 'packet'
	th_data_t * p = (th_data_t *)th_dp;
	// print something to the server
	printf("client %d joined, %d clients online now!\n",p->client_fd,n_run_thread);
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
		rc_lth = send(p->client_fd, "i am alive\n", 12, MSG_NOSIGNAL );
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

int server_thread_create_wrapper(int new_client_fd){
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
int main(void){

	struct addrinfo hints, *res_ai;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rc = getaddrinfo(NULL, SERVER_PORT, &hints, &res_ai);
	error_handle(rc != 0, "getaddrinfo");
	int sock_fd = socket(res_ai->ai_family, res_ai->ai_socktype, res_ai->ai_protocol);
	error_handle(sock_fd == -1, "socket");
	/* bind the address */
	rc = bind(sock_fd, res_ai->ai_addr, res_ai->ai_addrlen);
	error_handle(rc == -1, "bind");
	char dst[50];
	const char * temp = inet_ntop(res_ai->ai_family, res_ai->ai_addr, dst, res_ai->ai_addrlen);
	printf("my ip: %s\n ip2: %s\n", temp, dst);
	/*listen! indicate that i am the server*/
	rc = listen(sock_fd, 5);
	if(rc == -1){
		perror("listen");
		return -1;
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

	return 0;
}


