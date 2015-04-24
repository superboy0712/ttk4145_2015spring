#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

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

int cost_function(struct cost_param_t cost_values, int temp_order_floor,
		char temp_order_dircetion) {

	int i = 0;
	int minimum;
	int temp_diff[N_CLIENT];
	int minimum_one_s_array_indexes[N_CLIENT];/* for saving the ones that are equally minimum */
	int mosai_index = 0;
	int the_opt_idx = 0;
	memset(minimum_one_s_array_indexes, 0xfff, N_CLIENT * sizeof(int));
	for (i = 0; i < cost_values.max_connected_nodes; i++) {
		/**
		 *  exclude those exceptional ones from candidates
		 */
		if (cost_values.stop[i] == 1 || cost_values.obstrukt[i] == 1) {
			temp_diff[i] = abs(temp_order_floor - cost_values.floor[i]) + 1000;/* give the exceptional ones a  large difference */
		}
		else {
			temp_diff[i] = abs(temp_order_floor - cost_values.floor[i]);
		}

	}
	/* get the minimum difference and its idx */
	minimum = temp_diff[0];

	for (i = 0; i < cost_values.max_connected_nodes; i++) {
		if (temp_diff[i] <= minimum) {
			minimum = temp_diff[i];
			the_opt_idx = i;
		}
	}

	/* there may be several equally minimum difference candidates, save their idxes
	 * mosai_index is the number of the candidates */
	for (int i = 0; i < cost_values.max_connected_nodes; ++i) {
		if (temp_diff[i] == minimum) {
			minimum_one_s_array_indexes[mosai_index] = i;
			mosai_index++;
		}
	}
//	printf("mosai_index %d. ", mosai_index);
//	for (int i = 0; i < mosai_index; ++i) {
//		printf("mosai_index[%d] %d. ", i, minimum_one_s_array_indexes[i]);
//	}
//	puts("");
	/**
	 *  if the distances are the same, get the first of matched direction from these candidates
	 */
	for (int i = 0; i < mosai_index; ++i) {
		int temp = minimum_one_s_array_indexes[mosai_index];
		if (temp_order_dircetion == cost_values.direction[temp]) {
			the_opt_idx = temp;
			/* the first matched direction wins */
			break;
		}

	}
	if (the_opt_idx == 0) {
		/**
		 *  means the optimal decision is myself
		 */
		return 0;
	}

	printf("From Cost Function, minimum distance is %d and index is %d, direction is %c \n",
			minimum, the_opt_idx, cost_values.direction[the_opt_idx]);
	//printf(" last dir %c \n", cost_values.direction[the_opt_idx]);
	return cost_values.index[the_opt_idx];
}

