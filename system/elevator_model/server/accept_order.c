#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

ip_struct_t new_connection_data;			//initialzied in accept function
Node_t connected_nodes_data;			//initialzied in tcp_accept function

void* accept_order_function(void *shared_interface_data) {

	//shared structure part
	my_interface_t* interface_in_order_thrd =
			(my_interface_t*) shared_interface_data;

	//GP variables
	int loop_var = 0;
	int recv_bytes = 0;
	int temp_received_floor = 0;

	char temp_received_direction[2];
	char my_IP[INET6_ADDRSTRLEN];
	char temp_buf_ack[SEND_SIZE];
	char temp_floor_in_str[2];

	strncpy(temp_buf_ack, "ACK_MSG", 7);

	//local set variables
	fd_set order_set;
	int order_max;
	FD_ZERO(&order_set);

	////////////////////////////////////////

	//////Timeout Struct////

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100 * MS;		//100ms

	//////////////////////////

	//local copy of client struct to avoid multiple mutex locks inside loops

	struct client_t local_nodes_data[N_CLIENT];

	my_ip_function(my_IP);

	printf("Accept Order Thread: My own IP is %s.\n", my_IP);

	while (1) {

		/*	Step 1) copy sokc descriptors... 	*/

		pthread_mutex_lock(&connected_nodes_data.node_mutex);	//lock node mutex and copy only order to close connections !!!!

		for (loop_var = 0; loop_var < N_CLIENT; loop_var++) {

			local_nodes_data[loop_var].sock_order =
					connected_nodes_data.clients[loop_var].sock_order;
			local_nodes_data[loop_var].sock_data =
					connected_nodes_data.clients[loop_var].sock_data;
			strncpy(local_nodes_data[loop_var].my_ip,
					connected_nodes_data.clients[loop_var].my_ip,
					INET6_ADDRSTRLEN);

		}

		pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

		/*	Step 2) copy set variables... 	*/
		pthread_mutex_lock(&new_connection_data.ip_mutex);		//lock set mutex

		order_set = new_connection_data.master_set;
		order_max = new_connection_data.fdmax;

		pthread_mutex_unlock(&new_connection_data.ip_mutex);

		/*Now start polling.....*/

		select(order_max + 1, &order_set, NULL, NULL, &tv);    //pass 0 instead of &tv as first correction to timeout issue

		//////////////Receiving new data/////////////////
		for (loop_var = 0; loop_var < N_CLIENT; loop_var++) {

			if (FD_ISSET(local_nodes_data[loop_var].sock_order, &order_set)) {

				recv_bytes = recv(local_nodes_data[loop_var].sock_order,
						local_nodes_data[loop_var].buf_read, SEND_SIZE, 0);

				if (recv_bytes <= 0) {

					printf(
							"Accept Order Thread: Something wrong with this socket,client  %s should be closed %d bytes received.\n",
							local_nodes_data[loop_var].my_ip, recv_bytes);

				}    //if(recv_bytes<=0)

				else {    //else something received

					/*Debug part only*/
					if (!strncmp(local_nodes_data[loop_var].buf_read, "DOJOB_",
							6)) {
						;
					}
					else {
						printf(
								"\t\t\tAccept Order Thread:WRONG Message for status from %s\n",
								local_nodes_data[loop_var].my_ip);
					}
					///////////////////

					subString(local_nodes_data[loop_var].buf_read, 6, 1,
							temp_floor_in_str);
					printf("%s", temp_floor_in_str);
					temp_floor_in_str[1] = '\0';
					temp_received_floor = atoi(temp_floor_in_str);

					subString(local_nodes_data[loop_var].buf_read, 8, 1,
							temp_received_direction);
					temp_received_direction[1] = '\0';

					/*Debug Part*/
					printf(
							"\t\t\tAccept Order Thread: Order Received; message is %s Floor is %d from %s\n.",
							local_nodes_data[loop_var].buf_read,
							temp_received_floor,
							local_nodes_data[loop_var].my_ip);

					pthread_mutex_lock(
							&interface_in_order_thrd->interface_mutex);

					interface_in_order_thrd->received_floor =
							temp_received_floor;
					interface_in_order_thrd->received_direction =
							temp_received_direction[0];
					interface_in_order_thrd->received_floor_flag = TRUE;

					pthread_mutex_unlock(
							&interface_in_order_thrd->interface_mutex);

				}    //else something received

			}    //if(FD_SET...

			//usleep(50*MS);

		}    //for(i=0;....

		usleep(DELAY * MS);

	}    //while(1)

}

