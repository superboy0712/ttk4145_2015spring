#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

ip_struct_t 	new_connection_data;			//initialzied in accept function
Node_t 				connected_nodes_data;			//initialzied in tcp_accept function



void* send_status_function(void *shared_interface_data){


		//shared structure part
	my_interface_t* interface_in_status_thrd=(my_interface_t* ) shared_interface_data;




	//GP variables
	int 	loop_var			=0;
	int 	recv_bytes		=0;
	int 	send_bytes		=0;

	char 	temp_buf_interface[SEND_SIZE];		//temp_buffer
	char 	my_IP[INET6_ADDRSTRLEN];


	fd_set recv_set;
	int 	recv_max;
	FD_ZERO(&recv_set);





	////////////////////////////////////////


	//////Timeout Struct////

	struct 		timeval 	tv;
	tv.tv_sec 					= 0;
	tv.tv_usec 					= 100*MS;		//100ms

	//////////////////////////


		//local copy of client struct to avoid multiple mutex locks inside loops

	struct 	client_t 	local_nodes_data[N_CLIENT];

	my_ip_function(my_IP);

	printf("Send Status TCP Thread: My own IP is %s.\n",my_IP);



	while(1){

				//not sure about this approach, ask yulong whether this works or we will have a deadlock!

					/*	Step 1) copy sokc descriptors... 	*/

		pthread_mutex_lock(&connected_nodes_data.node_mutex);		//lock node mutex and copy all descriptors to close connections !!!!

			for(loop_var=0;loop_var<N_CLIENT;loop_var++){

				local_nodes_data[loop_var].sock_status=connected_nodes_data.clients[loop_var].sock_status;
				local_nodes_data[loop_var].sock_data=connected_nodes_data.clients[loop_var].sock_data;
				local_nodes_data[loop_var].sock_order=connected_nodes_data.clients[loop_var].sock_order;
				strncpy(local_nodes_data[loop_var].my_ip,connected_nodes_data.clients[loop_var].my_ip,INET6_ADDRSTRLEN);			

			}
	
		pthread_mutex_unlock(&connected_nodes_data.node_mutex);		//unlock node mutex

					/*	Step 2) copy interface buffer, here DONOT change send status flag (set true in main)....... 	*/



/*		pthread_mutex_lock(&interface_in_status_thrd->interface_mutex);

		strncpy(temp_buf_interface,interface_in_status_thrd->interface_status_buffer,SEND_SIZE);

		pthread_mutex_unlock(&interface_in_status_thrd->interface_mutex);


*/
		pthread_mutex_lock(&new_connection_data.ip_mutex);		//lock set mutex

		recv_set=new_connection_data.master_set;
		recv_max=new_connection_data.fdmax;

		pthread_mutex_unlock(&new_connection_data.ip_mutex);


		
		int i=select(recv_max+1,&recv_set,NULL,NULL,&tv);    //pass 0 instead of &tv as first correction to timeout issue
		if(i==-1){printf("Send Status TCP Thread:Select Failed\n");}

		//////////////Receiving new data/////////////////
				for(loop_var=0;loop_var<N_CLIENT;loop_var++){
				
					if(FD_ISSET(local_nodes_data[loop_var].sock_status, &recv_set)){

						recv_bytes=recv(local_nodes_data[loop_var].sock_status, local_nodes_data[loop_var].buf_read, SEND_SIZE, 0);


						if(recv_bytes<=0){
							if(recv_bytes==0){

								printf("Send Status TCP Thread:Client %s disconnected on recv error closing all sockets %d bytes recvived.\n",local_nodes_data[loop_var].my_ip,recv_bytes);
								close(local_nodes_data[loop_var].sock_status);        
								close(local_nodes_data[loop_var].sock_data);        	
								close(local_nodes_data[loop_var].sock_order);			//client's connection closed all sockets...


								pthread_mutex_lock(&new_connection_data.ip_mutex);
							
								int delete_flag=0;
								while(!(delete_flag=ip_delete_function(local_nodes_data[loop_var].my_ip)));

								printf("Send Status TCP Thread:Deleting %s ip SUCCESS.\n",local_nodes_data[loop_var].my_ip);//}
								
								memset(local_nodes_data[loop_var].my_ip,0,INET6_ADDRSTRLEN);

								FD_CLR(local_nodes_data[loop_var].sock_status, &new_connection_data.master_set);
								FD_CLR(local_nodes_data[loop_var].sock_data, &new_connection_data.master_set);
								FD_CLR(local_nodes_data[loop_var].sock_order, &new_connection_data.master_set);

								pthread_mutex_unlock(&new_connection_data.ip_mutex);

								local_nodes_data[loop_var].sock_status=0;				//clear sock status
								local_nodes_data[loop_var].sock_data=0;				//clear sock data both sockets...!!!!
								local_nodes_data[loop_var].sock_order=0;
							}
							else{
								perror("RX_ERROR:");
							}


						}	//if(recv_bytes<=0)

						else{


								/*Debug part only*/
								if(!strncmp(local_nodes_data[loop_var].buf_read, "SEND_STATUS", 11)){;}
								else {printf("\t\t\tSend Status TCP Thread:WRONG Message for status\n");}
								///////////////////
							pthread_mutex_lock(&interface_in_status_thrd->interface_mutex);

							strncpy(temp_buf_interface,interface_in_status_thrd->interface_status_buffer,SEND_SIZE);

							pthread_mutex_unlock(&interface_in_status_thrd->interface_mutex);
			
							send_bytes=send(local_nodes_data[loop_var].sock_data, temp_buf_interface, SEND_SIZE, 0);

								if(send_bytes<=0){

											printf("Send Status TCP Thread:Sending status failed on data channel for %s \n",local_nodes_data[loop_var].my_ip);

								}
							
								else {	//else (paired with if(send_bytes....)

									printf("Send Status TCP Thread: Status %s successfully sent to client %s.\n",temp_buf_interface,local_nodes_data[loop_var].my_ip);

								} 	//else (paired with if(send_bytes....)
								//usleep(50*MS);//										!!!!!!!!!!!!!!!!!!!
				
						}  //else something received*/
              
					}	//if(FD_SET...

					//usleep(50*MS);

				}	//for(i=0;....


								//update sock descriptors....

				pthread_mutex_lock(&connected_nodes_data.node_mutex);		//lock node mutex

					for(loop_var=0;loop_var<N_CLIENT;loop_var++){

						connected_nodes_data.clients[loop_var].sock_status=local_nodes_data[loop_var].sock_status;
						connected_nodes_data.clients[loop_var].sock_data=local_nodes_data[loop_var].sock_data;			 
						connected_nodes_data.clients[loop_var].sock_order=local_nodes_data[loop_var].sock_order;		//update all descriptors, 
						strncpy(connected_nodes_data.clients[loop_var].my_ip,local_nodes_data[loop_var].my_ip,INET6_ADDRSTRLEN);	

					}

				pthread_mutex_unlock(&connected_nodes_data.node_mutex);		//unlock node mutex	 
            



	

	usleep(DELAY*MS);
	
	}	//while(1)

}


