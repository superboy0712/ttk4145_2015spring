#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"
#include <math.h>
char global_ipArray[N_CLIENT][INET6_ADDRSTRLEN];	//shared between functios

void* initialzie_function(void * shared_interface_data) {

	pthread_t udp_discover_thread;
	pthread_t tcp_accept_thread;
	pthread_t send_status_thread;
	pthread_t accept_order_thread;
	pthread_t resolve_order_thread;

	//create interface_t struct for sharing with comm. moduel
	my_interface_t* interface_in_init_thrd =
			(my_interface_t*) shared_interface_data;

	//initialize struct to desired state
	pthread_mutex_init(&interface_in_init_thrd->interface_mutex, NULL);

	interface_in_init_thrd->received_floor_flag = FALSE;	//add order in queue and clear in main
	interface_in_init_thrd->order_floor_flag = FALSE;
	interface_in_init_thrd->received_floor = 0;
	interface_in_init_thrd->order_floor = 0;
	interface_in_init_thrd->order_direction = 'U';
	memset(interface_in_init_thrd->interface_status_buffer, 0, SEND_SIZE);

	//spawn comm. module threads....
	pthread_create(&tcp_accept_thread, NULL, tcp_accept_function, NULL);	//maybe some delay to let it settle down....
	usleep(100 * MS);
	pthread_create(&udp_discover_thread, NULL, discover_udp_function, NULL);
	pthread_create(&send_status_thread, NULL, send_status_function,
			interface_in_init_thrd);
	pthread_create(&accept_order_thread, NULL, accept_order_function,
			interface_in_init_thrd);
	pthread_create(&resolve_order_thread, NULL, resolve_order_function,
			interface_in_init_thrd);

	while (1) {
		sleep(10);
	}

	pthread_join(udp_discover_thread, NULL);
	pthread_join(tcp_accept_thread, NULL);
	pthread_join(send_status_thread, NULL);
	pthread_join(accept_order_thread, NULL);
	pthread_join(resolve_order_thread, NULL);

}

int ip_check_function(char *ip) {    //if new connectio, it will return -1 otherwise it will return 0 to N_CLIENT i.e. ip_index of that client
	int i;

	for (i = 0; i < N_CLIENT; i++) {

		if (!strncmp(ip, global_ipArray[i], INET6_ADDRSTRLEN))
			return i;		//return index; to be used in accept function!!!!

	}

	return -1;    //-1 means not in list
}

int connect_function(char *ip, char *port_num) {

	//socket structs and variables
	struct addrinfo *client_info_tcp;
	client_info_tcp = malloc(sizeof(struct addrinfo));

	//GP variables
	int sock_desc = 0;

	sock_desc = init_network_tcp(ip, port_num, &client_info_tcp);

	if (sock_desc <= 0) {
		return -1;
	}
	else {
		return sock_desc;
	}
}

void my_ip_function(char *ip) {    //get your own IP address
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	strncpy(ip, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr),
	INET_ADDRSTRLEN);

}

int ip_delete_function(char *ip) {

	int i;

	for (i = 0; i < N_CLIENT; i++) {
		if (!strncmp(ip, global_ipArray[i], INET6_ADDRSTRLEN)) {
			memset(global_ipArray[i], 0, INET6_ADDRSTRLEN);
			return 1;
		}    //if ip matches, delete ip from array

	}

	return 0;

}

char* subString(const char* input, int offset, int len, char* dest) {
	int input_len = strlen(input);

	if (offset + len > input_len) {
		return NULL;
	}

	strncpy(dest, input + offset, len);
	return dest;
}

void makeStatus(char *status, int floor, char dir) {

	sprintf(status, "MY_STATUS_%d_%c", floor, dir);

}
#define N_FLOORS 4
int cost_function(struct cost_param_t cost_values, int order_floor,
		char temp_order_dircetion) {
	int the_opt_idx = 0;
	float temp_order_floor = (float)order_floor;
	double cost_array[N_CLIENT];
	int dir = (temp_order_dircetion == 'U')? 1 : -1;
	puts("cost  orig values: ");
	for (int i = 0; i < cost_values.max_connected_nodes; i++){
		printf("[%d] pos: %f, mv: %d, last_dir %c, floor %d\n",
				i,
				cost_values.floor_position[i],
				cost_values.moving_vector[i],
				cost_values.direction[i],
				cost_values.floor[i]);
	}
	/* calculate cost of each connected node */
	for (int i = 0; i < cost_values.max_connected_nodes; i++) {
		/* basis cost */
		cost_array[i] = fabs(temp_order_floor - cost_values.floor_position[i]);

		/* if the cage is moving and moving away from order floor, then add extra cost */
//		if(cost_values.moving_vector[i] != 0){
//			if(cost_values.moving_vector[i]*(temp_order_floor - cost_values.floor_position[i]) <= 0){
//				/* order vector and current moving vector are in opposite direction */
//
//				if(cost_values.moving_vector[i]*dir < 0){
//					/* calling in different direction */
//					if(dir > 0){
//						cost_array[i] = cost_values.floor_position[i] + temp_order_floor;
//					}else{
//						cost_array[i] = 2*(N_FLOORS - 1) - cost_values.floor_position[i] - temp_order_floor;
//					}
//				}else{
//					cost_array[i] = 2*(N_FLOORS - 1) - fabs(temp_order_floor - cost_values.floor_position[i]);
//				}
//			}
//		}

		/* if exception,  add a large cost */
		if(cost_values.stop[i] == 1 || cost_values.obstrukt[i] == 1) {
			cost_array[i] += 1000;/* give the exceptional ones a  large difference */
		}

	}

    /* find the first minumum one, and return its index in the connected array */
	float minimum = cost_array[0];

	for(int i = 0; i < cost_values.max_connected_nodes; i++) {
		if (cost_array[i] < minimum) {
			minimum = cost_array[i];
			the_opt_idx = i;
		}
	}
	/* print */
	printf("The cost_array: ");
	for(int i = 0; i < cost_values.max_connected_nodes; i++){
		printf(" {[%d]%4.1lf},", cost_values.index[i], cost_array[i]);
	}
	puts("");

	printf("Order is %d_%c, the minimum cost is {%4.1lf from index [%d]}\n",
			order_floor, temp_order_dircetion, cost_array[the_opt_idx], cost_values.index[the_opt_idx]);
	if(the_opt_idx == 0)/*< myself */
		return 0;

	return cost_values.index[the_opt_idx];
}

