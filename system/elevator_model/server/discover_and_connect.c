#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

ip_struct_t new_connection_data;			//initialzied in accept function
Node_t connected_nodes_data;			//initialzied in tcp_accept function

char global_ipArray[N_CLIENT][INET6_ADDRSTRLEN];    //shared between functios
int global_ip_index;										//local to function

void* discover_udp_function() {

	int send_bytes_remote = 0;
	int sock_desc_remote = 0;
	struct addrinfo *remote_info;
	remote_info = malloc(sizeof(struct addrinfo));

	//recv part

	int sock_desc_local = 0, recv_bytes = 0, flag_cmp = 0;
	struct addrinfo *local_info;
	local_info = malloc(sizeof(struct addrinfo));
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	addrlen = sizeof(remoteaddr);
	int ip_check_var = 0;

	//TCP and UDP  IPs and ports....
	char *status_port = STATUS_PORT;
	char *data_port = DATA_PORT;
	char *order_port = ORDER_PORT;
	char *ip_udp = "129.241.187.255";	//master here behaves as client NOT server!!!! 
	char *port_num_udp = UDP_PORT;
	char my_IP[INET6_ADDRSTRLEN];
	char recv_ip[INET6_ADDRSTRLEN];

	//temp_buffers....
	char buf_recv[SEND_SIZE];
	char buf_send_udp[SEND_SIZE] = "I am Alive!!!";

	//variables for new_connection_data struct

	int temp_desc_status = 0;
	int temp_desc_data = 0;
	int temp_desc_order = 0;

	sock_desc_remote = init_network_udp(ip_udp, port_num_udp, &remote_info);	//to be done: add logic here to see if lan cable is disconnected or not!!!

	if (sock_desc_remote <= 0) {
		printf("UDP Discover Thread: Remote Socket init problem\n.");
	}

	sock_desc_local = init_network_udp("server", port_num_udp, &local_info);

	if (sock_desc_local <= 0) {
		printf("UDP Discover Thread: Local Socket init problem\n.");
	}

	my_ip_function(my_IP);

	printf("UDP Discover Thread: My IP is %s.\n", my_IP);

	while (1) {

		send_bytes_remote = send_string_udp(buf_send_udp, SEND_SIZE,
				sock_desc_remote, remote_info);

		if (send_bytes_remote < 0) {
			perror("UDP Discover Thread: Send Error:");
		}
		//usleep(50*MS);		//sleep for 50ms

		recv_bytes = recvfrom(sock_desc_local, buf_recv, SEND_SIZE, 0,
				(struct sockaddr *) &remoteaddr, &addrlen);

		if (recv_bytes > 0) {

			inet_ntop(remoteaddr.ss_family,
					get_in_addr((struct sockaddr *) &remoteaddr), recv_ip,
					sizeof(recv_ip));

			flag_cmp = strncmp(recv_ip, my_IP, INET6_ADDRSTRLEN);

			if (flag_cmp > 0) {

				pthread_mutex_lock(&new_connection_data.ip_mutex);

				ip_check_var = ip_check_function(recv_ip);

				pthread_mutex_unlock(&new_connection_data.ip_mutex);	//edited part

				if (ip_check_var == -1) {		//-1 means not in list

					printf("\t\t\tUDP Discover Thread:New Connection\n");
					temp_desc_status = connect_function(recv_ip, status_port);
					temp_desc_data = connect_function(recv_ip, data_port);
					temp_desc_order = connect_function(recv_ip, order_port);

					if (temp_desc_status
							!= -1&& temp_desc_data!=-1 && global_ip_index<=N_CLIENT) {

						pthread_mutex_lock(&new_connection_data.ip_mutex);

						strncpy(new_connection_data.new_ip, recv_ip,
								INET6_ADDRSTRLEN);		//copy ip

						new_connection_data.sock_status = temp_desc_status;
						new_connection_data.sock_data = temp_desc_data;	//copy ports...
						new_connection_data.sock_order = temp_desc_order;

						new_connection_data.ip_flag = TRUE;			//copy flag
						strncpy(global_ipArray[global_ip_index], recv_ip,
								INET6_ADDRSTRLEN);	//add to ip array							
						global_ip_index++;	//just for global_ipArray not actual number of clients connected!!!

						pthread_mutex_unlock(&new_connection_data.ip_mutex);

						usleep(50 * MS);

						while (1) {

							pthread_mutex_lock(&new_connection_data.ip_mutex);

							if (new_connection_data.ip_flag == FALSE) {
								printf(
										"\t\t\tUDP Thread:New Connection Added Successfully\n");
								pthread_mutex_unlock(
										&new_connection_data.ip_mutex);
								break;
							}

							else
								pthread_mutex_unlock(
										&new_connection_data.ip_mutex);

							usleep(50 * MS);

						}    //end of ehile
					}    //if(ip_check_var!=-1 && ip

					else
						printf(
								"UDP Discover Thread:Something wrong with index or connect\n");

				}    //if(!ip_check_var)	
				else
					pthread_mutex_unlock(&new_connection_data.ip_mutex);	//release ip_mutex

			}		//if(flag_cmp

		}		//if(recv_bytes

	}    //while(1)

}

void* tcp_accept_function() {

	/*NODE_t struct initialization*/
	int loop_var = 0;

	pthread_mutex_init(&connected_nodes_data.node_mutex, NULL);

	for (loop_var = 0; loop_var < N_CLIENT; loop_var++) {
		connected_nodes_data.clients[loop_var].sock_status = 0;
		connected_nodes_data.clients[loop_var].sock_data = 0;

		memset(connected_nodes_data.clients[loop_var].my_ip, 0,
				INET6_ADDRSTRLEN);
		memset(connected_nodes_data.clients[loop_var].buf_read, 0, SEND_SIZE);
		memset(connected_nodes_data.clients[loop_var].buf_send, 0, SEND_SIZE);
	}

/////////////////////////////////////////////////////////////

	//////Socket variables////

	int sock_status = 0;
	int sock_data = 0;
	int sock_order = 0;

	struct addrinfo *serv_info;

	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	addrlen = sizeof(remoteaddr);					//Added line as precaution

	char remoteIP[INET6_ADDRSTRLEN];

	//IP and PORT
	char *ip = "server";
	char *status_port_num = STATUS_PORT;
	char *data_port_num = DATA_PORT;
	char *order_port_num = ORDER_PORT;

	//////////////////////////

	//////Timeout Struct////

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100 * MS;		//100ms

	//////////////////////////

	//////GP Variables////

	int client_connected = 0;
	int temp_desc_status = 0;	//just for adding sock_data from udp thread....
	int temp_desc_data = 0;		//just for adding sock_data from udp thread....
	int temp_desc_order = 0;	//just for adding sock_order from udp thread....
	int temp_index = 0;

	char my_IP[INET6_ADDRSTRLEN];

	//////////////////////////

	//////Fill Structs and FD parameters////

	serv_info = malloc(sizeof(struct addrinfo));

	//get THREE local sockets for listening...

	sock_status = init_network_tcp(ip, status_port_num, &serv_info);
	sock_data = init_network_tcp(ip, data_port_num, &serv_info);
	sock_order = init_network_tcp(ip, order_port_num, &serv_info);

	////initialize new_connection_data struct

	pthread_mutex_init(&new_connection_data.ip_mutex, NULL);	//initialize mutex!

	pthread_mutex_lock(&new_connection_data.ip_mutex);

	FD_ZERO(&new_connection_data.master_set);
	FD_ZERO(&new_connection_data.sock_set);             //clear current contents

	FD_SET(sock_status, &new_connection_data.master_set);    //include sockets into set
	FD_SET(sock_data, &new_connection_data.master_set);
	FD_SET(sock_order, &new_connection_data.master_set);

	new_connection_data.fdmax =
			(sock_data > sock_status) ? sock_data : sock_status;	//first assign larger of sock_data or sock_status to fdmax
	new_connection_data.fdmax =
			(new_connection_data.fdmax > sock_order) ?
					new_connection_data.fdmax : sock_order;    //now assign larger of sock_order or fdmax to fdmax!!!

	new_connection_data.sock_status = 0;
	new_connection_data.sock_data = 0;
	new_connection_data.sock_order = 0;
	new_connection_data.ip_flag = 0;

	memset(new_connection_data.new_ip, 0, INET6_ADDRSTRLEN);

	pthread_mutex_unlock(&new_connection_data.ip_mutex);

	////////////////////////////////////////////////////

	/////////////////Now polling part and/////////////// 	

	my_ip_function(my_IP);

	printf("TCP Accept Thread:  My IP is %s.\n", my_IP);

	while (1) {

		//printf("TCP Accept Thread: Clients Connected are %d.\n",client_connected);

		pthread_mutex_lock(&new_connection_data.ip_mutex);		//lock set mutex

		new_connection_data.sock_set = new_connection_data.master_set;

		select(new_connection_data.fdmax + 1, &new_connection_data.sock_set,
				NULL, NULL, &tv);    //pass 0 instead of &tv as first correction to timeout issue

		//////////////Accepting new connection on sock_status  using select/////////////////

		if (FD_ISSET(sock_status, &new_connection_data.sock_set)) {

			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock set mutex

			printf(
					"\t\t\tTCP Accept Thread:New ACCEPT Connection on Status Sock!!!\n");
			addrlen = sizeof(remoteaddr);

			temp_desc_status = accept(sock_status,
					(struct sockaddr *) &remoteaddr, &addrlen);

			//accept new connection

			if (temp_desc_status < 0) {
				perror(
						"TCP Accept Thread:Status Sock,Error in accepting new connection:");
			}

			//now add new connection to set of socks to poll
			else {

				inet_ntop(remoteaddr.ss_family,
						get_in_addr((struct sockaddr *) &remoteaddr), remoteIP,
						sizeof(remoteIP));

				pthread_mutex_lock(&new_connection_data.ip_mutex);

				temp_index = ip_check_function(remoteIP);
				printf(
						"\t\t\tTCP Accept Thread: :NEW IP is %s and Temp_index for this ip is %d\n",
						remoteIP, temp_index);
				pthread_mutex_unlock(&new_connection_data.ip_mutex);

				if (temp_index != -1) {	//this means already in list, so no need to incerment client count, just add sock_desc!!!!

					printf(
							"TCP Accept Thread: Adding status socket for already EXISTING connection from %s socket %d.\n",
							remoteIP, temp_desc_status);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[temp_index].sock_status =
							temp_desc_status;    //only sock_desc, NOT ip since already exisitng!

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					FD_SET(temp_desc_status, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_status) {
						new_connection_data.fdmax = temp_desc_status;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

				}    //if(temp_index!=-1){		//this 

				else {	//this means not in list so increment client count as well!

					printf(
							"TCP Accept Thread:Adding status sock for NEW Connection from %s on  socket %d.\n",
							remoteIP, temp_desc_status);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[client_connected].sock_status =
							temp_desc_status;
					strncpy(
							connected_nodes_data.clients[client_connected].my_ip,
							remoteIP, INET6_ADDRSTRLEN);

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					strncpy(global_ipArray[global_ip_index], remoteIP,
							INET6_ADDRSTRLEN);

					global_ip_index++;

					FD_SET(temp_desc_status, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_status) {
						new_connection_data.fdmax = temp_desc_status;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

					client_connected++;	//actual number of clients connected to system!

				}    //else {	//this means

			}		//else { //now add new connection .....

		}		//ISSET if ends here!!!

		else
			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock_mutex for both of if(ISSET.....

		//////////////////////////////////////////////////////////////////////

		//////////////Accepting new connection on sock_data  using select/////////////////

		pthread_mutex_lock(&new_connection_data.ip_mutex);

		if (FD_ISSET(sock_data, &new_connection_data.sock_set)) {

			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock set mutex

			printf(
					"\t\t\tTCP Accept Thread:New ACCEPT Connection on Data Socket!!!\n");
			addrlen = sizeof(remoteaddr);

			temp_desc_data = accept(sock_data, (struct sockaddr *) &remoteaddr,
					&addrlen);

			//accept new connection

			if (temp_desc_data < 0) {
				perror(
						"TCP Accept Thread:Data Socket,Error in accepting new connection:");
			}

			//now add new connection to set of socks to poll
			else {

				inet_ntop(remoteaddr.ss_family,
						get_in_addr((struct sockaddr *) &remoteaddr), remoteIP,
						sizeof(remoteIP));

				pthread_mutex_lock(&new_connection_data.ip_mutex);

				temp_index = ip_check_function(remoteIP);

				pthread_mutex_unlock(&new_connection_data.ip_mutex);

				if (temp_index != -1) {	//this means already in list, so no need to incerment client count, jus add sock_desc!!!!

					printf(
							"TCP Accept Thread: Adding data socket for EXISTING connection from %s socket %d.\n",
							remoteIP, temp_desc_data);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[temp_index].sock_data =
							temp_desc_data;	//only sock_desc, NOT ip since already exisitng!*/

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					FD_SET(temp_desc_data, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_data) {
						new_connection_data.fdmax = temp_desc_data;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

				}    //if(temp_index!=0){		//this 

				else {	//this means not in list so increment client count as well!

					printf(
							"TCP Accept Thread:Adding data sock for NEW Connection from %s on  socket %d.\n",
							remoteIP, temp_desc_data);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[client_connected].sock_data =
							temp_desc_data;
					strncpy(
							connected_nodes_data.clients[client_connected].my_ip,
							remoteIP, INET6_ADDRSTRLEN);

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					strncpy(global_ipArray[global_ip_index], remoteIP,
							INET6_ADDRSTRLEN);
					global_ip_index++;

					FD_SET(temp_desc_data, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_data) {
						new_connection_data.fdmax = temp_desc_data;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

					client_connected++;	//actual number of clients connected to system!

				}    //else {	//this mea

			}		//else { //now add new connection .....

		}		//ISSET if ends here!!!

		else
			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock_mutex for both of if(ISSET.....
		//////////////////////////////////////////////////////////////////////

		//////////////Accepting new connection on sock_order  using select/////////////////

		pthread_mutex_lock(&new_connection_data.ip_mutex);

		if (FD_ISSET(sock_order, &new_connection_data.sock_set)) {

			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock set mutex

			printf(
					"\t\t\tTCP Accept Thread:New ACCEPT Connection on Order Socket!!!\n");
			addrlen = sizeof(remoteaddr);

			temp_desc_order = accept(sock_order,
					(struct sockaddr *) &remoteaddr, &addrlen);

			//accept new connection

			if (temp_desc_order < 0) {
				perror(
						"TCP Accept Thread:Order Socket,Error in accepting new connection:");
			}

			//now add new connection to set of socks to poll
			else {

				inet_ntop(remoteaddr.ss_family,
						get_in_addr((struct sockaddr *) &remoteaddr), remoteIP,
						sizeof(remoteIP));

				pthread_mutex_lock(&new_connection_data.ip_mutex);

				temp_index = ip_check_function(remoteIP);

				pthread_mutex_unlock(&new_connection_data.ip_mutex);

				if (temp_index != -1) {	//this means already in list, so no need to incerment client count, jus add sock_desc!!!!

					printf(
							"TCP Accept Thread: Adding order socket for EXISTING connection from %s socket %d.\n",
							remoteIP, temp_desc_order);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[temp_index].sock_order =
							temp_desc_order;	//only sock_desc, NOT ip since already exisitng!

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					FD_SET(temp_desc_order, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_order) {
						new_connection_data.fdmax = temp_desc_order;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

				}    //if(temp_index!=0){		//this 

				else {	//this means not in list so increment client count as well!

					printf(
							"TCP Accept Thread:Adding order sock for NEW Connection from %s on  socket %d.\n",
							remoteIP, temp_desc_order);

					pthread_mutex_lock(&connected_nodes_data.node_mutex);	//Edited part for thread, lock node mutex

					connected_nodes_data.clients[client_connected].sock_order =
							temp_desc_order;
					strncpy(
							connected_nodes_data.clients[client_connected].my_ip,
							remoteIP, INET6_ADDRSTRLEN);

					pthread_mutex_unlock(&connected_nodes_data.node_mutex);	//unlock node mutex

					pthread_mutex_lock(&new_connection_data.ip_mutex);	//for set operations, ask YB whether this is correct......

					strncpy(global_ipArray[global_ip_index], remoteIP,
							INET6_ADDRSTRLEN);
					global_ip_index++;

					FD_SET(temp_desc_order, &new_connection_data.master_set);

					if (new_connection_data.fdmax < temp_desc_order) {
						new_connection_data.fdmax = temp_desc_order;
					}

					pthread_mutex_unlock(&new_connection_data.ip_mutex);

					client_connected++;	//actual number of clients connected to system!

				}    //else {	//this mea

			}		//else { //now add new connection .....

		}		//ISSET if ends here!!!

		else
			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock_mutex for both of if(ISSET.....
		//////////////////////////////////////////////////////////////////////

		//////////////Adding new connection from udp thread/////////////////

		pthread_mutex_lock(&new_connection_data.ip_mutex);	//lock new_connection_data mutex

		if (new_connection_data.ip_flag == TRUE) {

			strncpy(remoteIP, new_connection_data.new_ip, INET6_ADDRSTRLEN);
			temp_desc_status = new_connection_data.sock_status;
			temp_desc_data = new_connection_data.sock_data;
			temp_desc_order = new_connection_data.sock_order;

			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock new_connection_data mutex inside if

			printf("TCP Accept Thread:New connection via UDP thread from %s \n",
					remoteIP);

			pthread_mutex_lock(&connected_nodes_data.node_mutex);    //Edited part for thread, lock node mutex

			connected_nodes_data.clients[client_connected].sock_status =
					temp_desc_status;
			connected_nodes_data.clients[client_connected].sock_data =
					temp_desc_data;
			connected_nodes_data.clients[client_connected].sock_order =
					temp_desc_order;
			strncpy(connected_nodes_data.clients[client_connected].my_ip,
					remoteIP, INET6_ADDRSTRLEN);

			pthread_mutex_unlock(&connected_nodes_data.node_mutex);    //unlock node mutex

			//add to set and clear flag for udp thread to proceed

			pthread_mutex_lock(&new_connection_data.ip_mutex);	//lock set mutex

			FD_SET(temp_desc_status, &new_connection_data.master_set);
			FD_SET(temp_desc_data, &new_connection_data.master_set);	//here temp_desc_data is sock_data
			FD_SET(temp_desc_order, &new_connection_data.master_set);

			if (new_connection_data.fdmax < temp_desc_status) {
				new_connection_data.fdmax = temp_desc_status;
			}

			if (new_connection_data.fdmax < temp_desc_data) {
				new_connection_data.fdmax = temp_desc_data;	//check logic......
			}

			if (new_connection_data.fdmax < temp_desc_order) {
				new_connection_data.fdmax = temp_desc_order;	//check logic......
			}

			new_connection_data.ip_flag = FALSE;

			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock ip_mutex

			client_connected++;

		}

		else
			pthread_mutex_unlock(&new_connection_data.ip_mutex);	//unlock new_connection_data mutex if not true...

		//////////////////////////////////////////////////////////////////////

		if (client_connected >= N_CLIENT) {
			printf(
					"TCP Accept Thread: Warning, clients connected are at max limit\n");
		}

		usleep(DELAY * MS);

	}    //while(1)
}

