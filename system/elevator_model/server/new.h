#ifndef NEW_H
#define NEW_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "network_module_tcp.h"
#include "network_module_udp.h"

#define STATUS_PORT 	"2500"
#define DATA_PORT		"3500"
#define ORDER_PORT		"4500"

#define UDP_PORT 		"49566"
#define SEND_SIZE 		50
#define N_CLIENT		20
#define TRUE 			1
#define FALSE 			0
#define MS				1000
#define DELAY			200

//Macros for external jobs flag

//struct for sharing data between udp and accept thread

typedef struct {

	pthread_mutex_t ip_mutex;

	int sock_status;

	int sock_data;

	int sock_order;

	char new_ip[INET6_ADDRSTRLEN];

	int ip_flag;

	fd_set sock_set;

	fd_set master_set;

	int fdmax;

} ip_struct_t;

//struct for sharing data between TCP comm threads

struct client_t {

	int sock_status;

	int sock_data;

	int sock_order;

	char my_ip[INET6_ADDRSTRLEN];

	char buf_read[SEND_SIZE];

	char buf_send[SEND_SIZE];

};

typedef struct {

	pthread_mutex_t node_mutex;

	struct client_t clients[N_CLIENT];

} Node_t;

//struct for cost function
struct cost_param {

	int floor[N_CLIENT];

	char direction[N_CLIENT];

	int index[N_CLIENT];

	char temp_floor;

	int max_index;

};

//interface struct between main and comm module

typedef struct {

	pthread_mutex_t interface_mutex;

	char interface_status_buffer[SEND_SIZE];//ONLY FOR STRINGS WITH FORMAT MY_STATUS_X_Y		X=Floor, Y=Directio

	int received_floor;							//accept_order_thread

	int received_floor_flag;					//accept_order_thread

	int order_floor;							//resolve_order_thread

	char order_direction;						//resolve_order_thread

	int order_floor_flag;						//resolve_order_thread

} my_interface_t;

//threads

void* discover_udp_function();

void* tcp_accept_function();

void* send_status_function(void *);		//using interface struct

void* resolve_order_function(void *);		//using interface struct

void* accept_order_function(void *);		//using interface struct

void* initialzie_function(void *);		//spawn all threads......

//functions
int ip_check_function(char *ip);
int connect_function(char *ip, char *port);
void my_ip_function(char *ip);
int ip_delete_function(char *ip);
int update_status_function(int *sock_desc, char *my_ip, char *data_send,
		char *data_recv);
char* subString(const char* input, int offset, int len, char* dest);
void makeStatus(char *output, int floor, char dir);
int cost_function(struct cost_param, int temp_order_floor,
		char temp_order_direction);

#endif
