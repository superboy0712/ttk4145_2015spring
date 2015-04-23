/**
 * @file NEW.h
 * @author Group 12
 * @date 21 April 2014
 * @brief top level Network Module 
 *
 * Network to level module is used by following sub-modules;
 *
 * 1) disocver_and_connect() This sub-module handles two threds and is responsible for establisihing connection
 * using TCP or UDP 
 *
 * 2) send_status() This sub-module is composed of one thread.
 * The primary function of this sub-module is to send_status when requested by reomte
 * system. Apart from this, secondary function  of this submodule is to close sockets when remote system is
 * disconnected .
 * 
 * 3) accept_order() This sub-module also responsbile for single thread. Function of this modules is to 
 * accept orders from remote systems and add to local order queue 
 *
 * 4) resolve_order() This sub-module handles single thread. 
 * This sub-module waits for a button press/new order and than decides which system will serve this order depending upon cost
 * function inputs.
 *
 * 5) new() This sub-module is used for spawning all threads so that control module has to initialize ONE thread only
 * 
 */
#ifndef NEW_H
#define NEW_H		

//libraries rquired by module
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "network_module_tcp.h"
#include "network_module_udp.h"

//Macros for external jobs flag

#define 	STATUS_PORT		"2500"
#define 	DATA_PORT		"3500"
#define 	ORDER_PORT		"4500"
#define 	UDP_PORT		"49566"
#define 	SEND_SIZE		100
#define 	N_CLIENT		20
#define 	TRUE			1
#define 	FALSE			0
#define 	MS				1000
#define 	DELAY			150

/**
 * @brief Structure for adding and maintaining connections.
 *
 * This struct is used for adding a new network connection or deleting an existing connection.
 * Sub-modules using of this struct are;
 * disocver_and_connect()
 * send_status()
 * accept_order()
 */

typedef struct {

	pthread_mutex_t ip_mutex; /**< Mutex for protecting a shared struct*/

	int sock_status; /**< Descriptor for sokcet channel*/

	int sock_data; /**< Descriptor for sokcet channel*/

	int sock_order; /**< Descriptor for sokcet channel*/

	char new_ip[INET6_ADDRSTRLEN]; /**< IP address of remote system*/

	int ip_flag; /**< Flag used for adding IP of a new connection*/

	fd_set sock_set; /**< SET variables; required for select() call */

	fd_set master_set; /**< SET variables; required for select() call */

	int fdmax; /**< Max value of set variables; required for select() call */

} ip_struct_t;

/**
 * @brief Structure defining basic parameter required by every remote system (also termed as client).
 *
 * This struct is used for defining basic architecture of a remote system (client). 
 * This struct is used in two different ways inside sub-modules;
 * 1) For making local copy of data of connected clients.
 * 2) Used as array in Node_t struct and shared between all threads as global remote systems connected. 
 */

struct client_t {

	int sock_status; /**< status channel socket descriptor for client; This channel is used for sending status request to others */

	int sock_data; /**< data channel socket descriptor for client; This is channel on which data is sent e.g. status value */

	int sock_order; /**< order channel socket descriptor for client; This is the channel on which order is sent to others*/

	char my_ip[INET6_ADDRSTRLEN]; /**< IP addres of client */

	char buf_read[SEND_SIZE]; /**< Read buffer for client */

	char buf_send[SEND_SIZE]; /**< Send buffer for client */

};

/**
 * @brief This is the global structure shared by all sub-modules 
 *
 * This struct contains information related with connected clients.
 * This struct uses client_t struct as its member.
 * 
 */

typedef struct {

	pthread_mutex_t node_mutex; /**< Mutex for protecting a shared struct */

	struct client_t clients[N_CLIENT]; /**< Array of client_t structs, contains all info related with connected clients*/

} Node_t;

/**
 * @brief Struct used by cost function 
 *
 * This struct contains information for calculating cost.
 * This struct is used by resolve_order() sub-module.
 * At location 0 of every array member, local/own status is present.
 */
struct cost_param_t {

	int floor[N_CLIENT]; /**< Floors status of clients + myself*/

	char direction[N_CLIENT]; /**< Direction status of clients + myself*/

	int index[N_CLIENT]; /**< No. of currently connected clients*/

	char temp_floor[2]; /**< Temporay variable used for conversion from string to int*/

	int stop[N_CLIENT]; /**< STOP button status*/

	int obstrukt[N_CLIENT]; /**< OBSTRUKT button status*/

	float floor_position[N_CLIENT]; /**< In-between floor position*/

	int moving_vector[N_CLIENT]; /**< Direction Vector of motor*/

	int timeout_status[N_CLIENT]; /**< Timeout status of client*/

	int max_index; /**< Maximum value of index*/

};

/**
 * @brief Interface struct between main and comm module
 *
 * This struct provides interface between network module and control moduel.
 * This struct is used by following sub-modules;
 * discover_and_connect()
 * send_status()
 * resolve_order()
 * accept_order()
 */

typedef struct {

	pthread_mutex_t interface_mutex; /**< Mutex for protecting shared structure*/

	char interface_status_buffer[SEND_SIZE]; /**< Buffer used for exchange of status*/

	int received_floor; /**< Received order floor from other client*/

	char received_direction; /**< Received order direction from other client*/

	int received_floor_flag; /**<Flag used for received order; comm. module blocks on this until control module clears this flag*/

	int order_floor; /**< New order floor; order generated at my panel*/

	char order_direction; /**< New order dir; order generated at my panel*/

	int order_floor_flag; /**<Flag used for new order generated at my panel; control module blocks on this until comm. module clears this flag*/

} my_interface_t;

/**
 * @brief Global variables used by communication sub-modules;
 *
 */

extern ip_struct_t new_connection_data; /**< Data for new connection*/
extern Node_t connected_nodes_data; /**< Data of all connected clients in the system*/

extern char gloabl_ipArray[N_CLIENT][INET6_ADDRSTRLEN]; /**< Array of IP addresses of connected clients*/
extern int global_ip_index; /**< No. of connected clients*/

/** 
 * @brief Threads implemented by communication module, each thread is a sub-modules;
 *
 */

void* discover_udp_function(); /**< Implemented by discover_and_connect sub-module, used for discovering clients */

void* tcp_accept_function(); /**< Implemented by discover_and_connect sub-module, used for accepting TCP requests*/

void* send_status_function(void *); /**< Implemented by send_status sub-module, used for sending status*/

void* resolve_order_function(void *); /**< Implemented by resolve_order sub-module, used for resolving local order*/

void* accept_order_function(void *); /**< Implemented by accept_order sub-module, used for accepting remote orders */

void* initialzie_function(void *); /**< Implemented by new sub-module, used for spawning all threads*/

/** 
 * @brief Functions used by communication module;
 *
 */

int ip_check_function(char *ip); /**< Check IP already exists in global array*/
int connect_function(char *ip, char *port); /**< send TCP connect request*/
void my_ip_function(char *ip); /**< get OWN ip address*/
int ip_delete_function(char *ip); /**< Delete IP address of a client on disconnection*/
char* subString(const char* input, int offset, int len, char* dest); /**< Extract substring from a string */
void makeStatus(char *output, int floor, char dir); /**< Make status to send*/
int cost_function(struct cost_param_t, int temp_order_floor,
		char temp_order_direction); /**< Calculate cost of clients plus myself*/

#endif
