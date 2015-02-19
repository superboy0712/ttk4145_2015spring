#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define PORT_NUM "4950"
struct addrinfo *servinfo;
struct sockaddr_storage their_addr;
socklen_t addr_len;
struct addrinfo hints, *servinfo;
long int buf = 0;
int recv_bytes = 0;
// listen and create thread
pthread_t listen_and_spawn_th;
pthread_t broadcast_alive_th;
pthread_mutex_t lock_AM_i_MASTER = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t become_master_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t become_slave_cv = PTHREAD_COND_INITIALIZER;
//
typedef enum {
	undefined = 0, master_alive = 4, slave_alive = 255
} aliveness_t;

int am_I_master = 0;
//
long int slave_alive_fd;    // for broadcasting and receiving alive message
long int master_alive_fd;
//
// wrap two fd to pass to broadcast thread
typedef struct {
	int slave;
	int master;
} fd_t;

void * broadcast_alive(void * fd) {
	fd_t * fd_temp = (fd_t *) fd;
	aliveness_t aliveness = undefined;
	char tempbuff[101];
	while (1) {

		pthread_mutex_lock(&lock_AM_i_MASTER);
		int isMaster = am_I_master;
		pthread_mutex_unlock(&lock_AM_i_MASTER);

		if (isMaster) {
			aliveness = master_alive;
			snprintf(tempbuff, 100, "Pid: %d, master alive\n", getpid());
			if ((sendto(fd_temp->master, tempbuff , 100, 0,
					servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
				perror("sendto in broadcast_alive:");
			}
		}
		else {
			aliveness = slave_alive;
			snprintf(tempbuff, 100, "Pid: %d, slave alive\n", getpid());
			if ((sendto(fd_temp->slave, tempbuff, 100, 0,
					servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
				perror("sendto in broadcast_alive:");
			}
		}

		usleep(10000);
	}
	return NULL;
}
int socket_create_wrapper(const char * port_num, time_t tv_sec, suseconds_t tv_usec) {

	int sockfd;


	//Sockets variablesai_addr
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;          // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;    // use my IP

	int yes = 1, rv;

	//struct for timeout part

	struct timeval tv;
	tv.tv_sec = tv_sec;
	tv.tv_usec = tv_usec;

	//Fill IP address struct
	if ((rv = getaddrinfo(NULL, port_num, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	//create socket descriptor
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
		perror("listener: socket");
		return -1;
	}

	//set socket options, reusable and timeout
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt_reuse sockfd:");
		return(-1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("setsockopt_timeout :");
		return(-1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(tv)) < 0) {
		perror("setsockopt_ broadcast :");
		return(-1);
	}


	//now bind socket to PORT..
	if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(sockfd);
		perror("listener: bind");
		return(-1);
	}

	return sockfd;
}
char strbuf[100]={0};
pthread_t slave_listen_th, master_listen_th;
typedef struct {
	pthread_t thid;
	long int fd;
} pass_to_listner_t;
void * listener(void * data){
	pass_to_listner_t * s = (pass_to_listner_t *)data;
	char buf[101];
	while(1){
		int am_I_slave = (s->thid == slave_listen_th)? 5:0;
		while(
				(am_I_master != 0)&&(am_I_slave)
			){


		}
		memset(buf, 0, 100);
		int recv_bytes = recvfrom(s->fd, buf, 100, 0,
				(struct sockaddr *) &their_addr, &addr_len);

		if (recv_bytes < 0 ) {
			perror("\nrecvfrom ");
			printf("fd is %ld\n",s);
			// not receiving anything, means nobody alive?
		} else {
			printf("master buff got %s , my pid is %d, socket got %ld \n", buf, getpid(), s);
		}

		usleep(100000);
	}
	return NULL;
}
int main(int argc, char ** argv) {

	if(argc == 2)
		if(argv[1][0] == 's')
		{
			am_I_master = 0;
		}
		else
		{
			am_I_master = 1;
		}

	slave_alive_fd = socket_create_wrapper("4000", 1, 500000);
	master_alive_fd = socket_create_wrapper("5000",  1, 500000);
//************************************************************
	printf("Everything OK with Socket part....\n");
	/**
	 *  creating threads
	 */
	fd_t fd_to_pass = { .slave = slave_alive_fd, .master = master_alive_fd };
	int rc = pthread_create(&broadcast_alive_th, 0, broadcast_alive,
			(void *) &fd_to_pass);
	if (rc) {
		perror("pthread create");
		exit(-1);
	}

	rc = pthread_create(&slave_listen_th, 0, listener,
			(void *)slave_alive_fd);
	if (rc) {
		perror("pthread create");
		exit(-1);
	}
	rc = pthread_create(&master_listen_th, 0, listener,
			(void *)master_alive_fd);
	if (rc) {
		perror("pthread create");
		exit(-1);
	}

	pthread_join(broadcast_alive_th, NULL);
	pthread_join(master_listen_th, NULL);
	pthread_join(slave_listen_th, NULL);
	system("gnome-terminal -e ./ex6");
	printf("I am master now.....\n");

	/**
	 * join
	 */

	return 0;
}
