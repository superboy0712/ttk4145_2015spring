#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

ip_struct_t 	new_connection_data;			//initialzied in accept function
Node_t 				connected_nodes_data;			//initialzied in tcp_accept function


void* resolve_order_function(void* shared_interface_data){

		//shared structure part
	my_interface_t* interface_in_resolve_thrd=(my_interface_t* ) shared_interface_data;

	//GP variables
	int 	loop_var					=0;
	int		temp_send_bytes		=0;		//used for DOJOB_ order send message
	int 	recv_bytes[N_CLIENT];
	int		send_desc[N_CLIENT];

		//cost function part
	int 	desired_index; 
	int 	temp_order_floor;
	char	temp_order_dir;


	char 	buf_send_status[SEND_SIZE];
	char 	buf_send_order[SEND_SIZE];
	char 	buf_recv_ack[SEND_SIZE];
	char 	my_IP[INET6_ADDRSTRLEN];
	char 	my_status[SEND_SIZE];

	strncpy(buf_send_status,"SEND_STATUS",11);


	struct client_t 	local_nodes_data[N_CLIENT];

	struct cost_param_t	cost_values;


	my_ip_function(my_IP);

	printf("Resolve Order Thread: My own IP is %s. \n",my_IP);


	while(1){

		pthread_mutex_lock(&interface_in_resolve_thrd->interface_mutex);

			if(interface_in_resolve_thrd->order_floor_flag==TRUE) {

				temp_order_floor=interface_in_resolve_thrd->order_floor;
				temp_order_dir=interface_in_resolve_thrd->order_direction;

				strncpy(my_status,interface_in_resolve_thrd->interface_status_buffer,SEND_SIZE);
			
				pthread_mutex_unlock(&interface_in_resolve_thrd->interface_mutex);
				


						//get sock descriptors.....

				pthread_mutex_lock(&connected_nodes_data.node_mutex);		//lock node mutex

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){
						local_nodes_data[loop_var].sock_status=connected_nodes_data.clients[loop_var].sock_status;
						local_nodes_data[loop_var].sock_data=connected_nodes_data.clients[loop_var].sock_data;
						local_nodes_data[loop_var].sock_order=connected_nodes_data.clients[loop_var].sock_order;
						strncpy(local_nodes_data[loop_var].my_ip,connected_nodes_data.clients[loop_var].my_ip,INET6_ADDRSTRLEN);
			
					}

				pthread_mutex_unlock(&connected_nodes_data.node_mutex);		//unlock node mutex	

				

							//send SEND_STATUS  to everyone non zero sock_status  and expect receive on sock_data from same client 

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){
							if(local_nodes_data[loop_var].sock_status!=0){
									send_desc[loop_var]=send(local_nodes_data[loop_var].sock_status, buf_send_status, SEND_SIZE, 0);
									usleep(100*MS);		//changed from 50MS

										if(send_desc[loop_var]>0){
											recv_bytes[loop_var]=recv(local_nodes_data[loop_var].sock_data, local_nodes_data[loop_var].buf_read, SEND_SIZE, 0);

												if(recv_bytes[loop_var]<=0){
														printf("Resolve Order Thread: Status Request sent to client %s but nothing received CLOSING\n.",local_nodes_data[loop_var].my_ip);

																	//EDITED PART
														close(local_nodes_data[loop_var].sock_status);        
														close(local_nodes_data[loop_var].sock_data);        	
														close(local_nodes_data[loop_var].sock_order);			//client's connection closed all sockets...


														pthread_mutex_lock(&new_connection_data.ip_mutex);
					
														int delete_flag=0;
														while(!(delete_flag=ip_delete_function(local_nodes_data[loop_var].my_ip)));

														printf("Resolve Order Thread: Deleting %s ip SUCCESS.\n",local_nodes_data[loop_var].my_ip);//}
						
														memset(local_nodes_data[loop_var].my_ip,0,INET6_ADDRSTRLEN);

														FD_CLR(local_nodes_data[loop_var].sock_status, &new_connection_data.master_set);
														FD_CLR(local_nodes_data[loop_var].sock_data, &new_connection_data.master_set);
														FD_CLR(local_nodes_data[loop_var].sock_order, &new_connection_data.master_set);

														pthread_mutex_unlock(&new_connection_data.ip_mutex);

														local_nodes_data[loop_var].sock_status=0;				//clear sock status
														local_nodes_data[loop_var].sock_data=0;				//clear sock data both sockets...!!!!
														local_nodes_data[loop_var].sock_order=0;
																//EDITED PART ENDS HERE

												}										
												else {
														/*Debug part*/
														if(!strncmp(local_nodes_data[loop_var].buf_read,"MY_STATUS_",10)){
																printf("Resolve Order Thread: Status Recieved from client %s is %s\n",local_nodes_data[loop_var].my_ip,local_nodes_data[loop_var].buf_read);
														}

														else {
															printf("Resolve Order Thread: Status Reply Recieved in wrong format from client %s and is %s\n",local_nodes_data[loop_var].my_ip,local_nodes_data[loop_var].buf_read);
														}
												}
													/*////////////*/

										}	//add check here to exclude wrong format replies

							}	//if(local_client[loop_var].sock_status!=	
							
							else	
								usleep(50*MS);			

					}	//for(i=0;i<N_

							
											//extract  MY OWN status at location zero of all arrays.....
								subString(my_status, 10, 1, &cost_values.temp_floor);
								cost_values.temp_floor[1] = '\0';
								cost_values.floor[0]=atoi(&cost_values.temp_floor);									
								subString(my_status, 12, 1, &cost_values.direction[0]);
								cost_values.index[0]=1;

								
					int index =1;

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){

							if(recv_bytes[loop_var]>0){

							


											//extract floors...
								subString(local_nodes_data[loop_var].buf_read, 10, 1, &cost_values.temp_floor);
								cost_values.temp_floor[1] = '\0';
								cost_values.floor[index]=atoi(&cost_values.temp_floor);
												//extract directions...
								subString(local_nodes_data[loop_var].buf_read, 12, 1, &cost_values.direction[index]);
												//copy sock_desc....
								cost_values.index[index]=loop_var+1;

								printf("Resolve Order Thread: Received Client's Floor is %d Direction is %c \n",cost_values.floor[index],cost_values.direction[index]);	//debug

								index++;

								//usleep(50*MS);
				
							
							}		//if(recv_bytes[loop_
								
					}	//for(loop_var=0; loop_var<N_CLIE

						cost_values.max_index=index;		//maximum number of clients connected including myself.....

										//call cost function...
						desired_index=cost_function( cost_values, temp_order_floor, temp_order_dir);					

						if(desired_index!=0){
							
							sprintf(buf_send_order, "DOJOB_%d_%c", temp_order_floor,temp_order_dir);
							temp_send_bytes=send(local_nodes_data[desired_index-1].sock_order, buf_send_order, SEND_SIZE, 0);		//SOCK_ORDER!!!
							usleep(50*MS);		//maybe some delay but should not be, ideally
							printf("\t\t\tResolve Order Thread:Dispatching job %s to client %s order is %d\n",buf_send_order,local_nodes_data[desired_index-1].my_ip,temp_order_floor);
						
								if(temp_send_bytes<=0){

									printf("Resolve Order Thread: Failed on sending order to %s client, I shall do this job\n",local_nodes_data[desired_index-1].my_ip);

										pthread_mutex_lock(&interface_in_resolve_thrd->interface_mutex);

										interface_in_resolve_thrd->order_floor=temp_order_floor;
										interface_in_resolve_thrd->order_floor_flag=FALSE;

										pthread_mutex_unlock(&interface_in_resolve_thrd->interface_mutex);

								}	//if(temp_send_bytes<=


								else {	//else send_bytes were good and go for ACK message

												pthread_mutex_lock(&interface_in_resolve_thrd->interface_mutex);

												interface_in_resolve_thrd->order_floor=-1;			//clear for main to not add this job in its own queue
												interface_in_resolve_thrd->order_floor_flag=FALSE;

												pthread_mutex_unlock(&interface_in_resolve_thrd->interface_mutex);


								}	//else send_bytes were good and go for ACK message

								usleep(50*MS);

						}	//if(desired_index!


						else {	//myself has lowest cost...


									printf("Resolve Order Thread:Minimum cost is myself!\n.");
						
									pthread_mutex_lock(&interface_in_resolve_thrd->interface_mutex);

									interface_in_resolve_thrd->order_floor=temp_order_floor;
									interface_in_resolve_thrd->order_floor_flag=FALSE;

									pthread_mutex_unlock(&interface_in_resolve_thrd->interface_mutex);
									
						}	//else myself has....

						/*clear struct....*/

						for(loop_var=0; loop_var<N_CLIENT; loop_var++){

							cost_values.floor[loop_var]=0;
							cost_values.direction[loop_var]=0;
							cost_values.index[loop_var]=0;
							recv_bytes[loop_var]=0;
								//client's parameter stored locally
							local_nodes_data[loop_var].sock_status=0;
							local_nodes_data[loop_var].sock_data=0;
							local_nodes_data[loop_var].sock_order=0;
							memset(local_nodes_data[loop_var].my_ip, 0, INET6_ADDRSTRLEN);
							memset(local_nodes_data[loop_var].buf_read, 0, SEND_SIZE);
							memset(local_nodes_data[loop_var].buf_send, 0, SEND_SIZE);
						
						}
			
					
			
			}	//if(interface_in_resolve_thrd->order_f

			else pthread_mutex_unlock(&interface_in_resolve_thrd->interface_mutex);		//else release mutex outside thread.....

						/*clear struct....*/
						for(loop_var=0; loop_var<N_CLIENT; loop_var++){

							cost_values.floor[loop_var]=0;
							cost_values.direction[loop_var]=0;
							cost_values.index[loop_var]=0;
							recv_bytes[loop_var]=0;
								//client's parameter stored locally
							local_nodes_data[loop_var].sock_status=0;
							local_nodes_data[loop_var].sock_data=0;
							local_nodes_data[loop_var].sock_order=0;
							memset(local_nodes_data[loop_var].my_ip, 0, INET6_ADDRSTRLEN);
							memset(local_nodes_data[loop_var].buf_read, 0, SEND_SIZE);
							memset(local_nodes_data[loop_var].buf_send, 0, SEND_SIZE);
						
						}
			




		usleep(DELAY*MS);
	}	//while(1)




}


