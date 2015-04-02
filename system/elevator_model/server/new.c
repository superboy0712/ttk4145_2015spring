#include "network_module_tcp.h"
#include "network_module_udp.h"
#include "new.h"

static ip_struct_t 	ip_shared;			//initialzied in accept function
static Node_t 		in_thread;			//initialzied in tcp_accept function


static char ipArray[N_CLIENT][INET6_ADDRSTRLEN];	//shared between functios
static int ip_index=0;		//local to function



void* discover_udp_function()
{
	



	int send_bytes_remote=0;
	int sock_desc_remote=0;
	struct addrinfo *remote_info;			
	remote_info=malloc(sizeof (struct addrinfo));	

	//recv part

	int sock_desc_local=0,recv_bytes=0,flag_cmp=0;
	struct addrinfo *local_info;
	local_info=malloc(sizeof (struct addrinfo));
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	addrlen=sizeof(remoteaddr);
	int temp_var=0;

		//TCP and UDP  IPs and ports....
	char	*status_port=STATUS_PORT;
	char	*data_port=DATA_PORT;
	char 	*order_port=ORDER_PORT;
	char 	*ip_udp="129.241.187.255";			//master here behaves as client NOT server!!!! 
	char 	*port_num_udp = UDP_PORT;
	char 	my_IP[INET6_ADDRSTRLEN];
	char 	recv_ip[INET6_ADDRSTRLEN];

		//temp_buffers....
	char buf_recv[SEND_SIZE];
	char buf_send_udp[SEND_SIZE]="I am Alive!!!";

		//variables for ip_shared struct

	int		temp_desc_status	=0;
	int 	temp_desc_data		=0;
	int 	temp_desc_order		=0;
	int 	recv_control		=0;

    sock_desc_remote=init_network_udp(ip_udp, port_num_udp,&remote_info);		//to be done: add logic here to see if lan cable is disconnected or not!!!

	if(sock_desc_remote<=0){
		printf("UDP Discover Thread: Remote Socket init problem\n.");
	}
		


	sock_desc_local=init_network_udp("server",port_num_udp,&local_info);

	if(sock_desc_local<=0){
		printf("UDP Discover Thread: Local Socket init problem\n.");
	}


	my_ip_function(my_IP);

	printf("UDP Discover Thread: My IP is %s.\n",my_IP);


	while(1){


		send_bytes_remote=send_string_udp(buf_send_udp,SEND_SIZE,sock_desc_remote,remote_info);
	
			if(send_bytes_remote<0){
				perror("UDP Discover Thread: Send Error:");		
			}
		//usleep(50*MS);		//sleep for 50ms

				recv_bytes=recvfrom(sock_desc_local,buf_recv,SEND_SIZE, 0,(struct sockaddr *)&remoteaddr, &addrlen);


					if(recv_bytes>0){

						inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),recv_ip, sizeof (recv_ip));
				
						flag_cmp=strncmp(recv_ip,my_IP,INET6_ADDRSTRLEN);

							if(flag_cmp>0) {
						
									pthread_mutex_lock(&ip_shared.ip_mutex);

									temp_var=ip_check_function(recv_ip);

									pthread_mutex_unlock(&ip_shared.ip_mutex);		//edited part

										if(temp_var==-1){		//-1 means not in list

											printf("\t\t\tUDP Discover Thread:New Connection\n");									
											temp_desc_status=connect_function(recv_ip, status_port);
											temp_desc_data=connect_function(recv_ip, data_port);
											temp_desc_order=connect_function(recv_ip, order_port);

												if(temp_desc_status!=-1 && temp_desc_data!=-1 && ip_index<=N_CLIENT){

													pthread_mutex_lock(&ip_shared.ip_mutex);

													strncpy(ip_shared.new_ip,recv_ip,INET6_ADDRSTRLEN);		//copy ip

													ip_shared.sock_status=temp_desc_status;
													ip_shared.sock_data=temp_desc_data;						//copy ports...
													ip_shared.sock_order=temp_desc_order;

													ip_shared.ip_flag=TRUE;									//copy flag
													strncpy(ipArray[ip_index],recv_ip,INET6_ADDRSTRLEN);			//add to ip array							
													ip_index++;		//just for ipArray not actual number of clients connected!!!

													pthread_mutex_unlock(&ip_shared.ip_mutex);


													usleep(50*MS);

														while(1){

															pthread_mutex_lock(&ip_shared.ip_mutex);

															if(ip_shared.ip_flag==FALSE) {printf("\t\t\tUDP Thread:New Connection Added Successfully\n"); pthread_mutex_unlock(&ip_shared.ip_mutex); break;}

															else pthread_mutex_unlock(&ip_shared.ip_mutex);

															usleep(50*MS);

														}	//end of ehile
												}	//if(temp_var!=-1 && ip

												else printf("UDP Discover Thread:Something wrong with index or connect\n");

										}	//if(!temp_var)	
										else pthread_mutex_unlock(&ip_shared.ip_mutex);		//release ip_mutex

								
							}//if(flag_cmp


					}//if(recv_bytes

	}	//while(1)

}





void* tcp_accept_function()
{



    		/*NODE_t struct initialization*/
	int loop_var=0;

	pthread_mutex_init(&in_thread.node_mutex,NULL);

	for(loop_var=0; loop_var<N_CLIENT; loop_var++){
		in_thread.clients[loop_var].sock_status=0;
		in_thread.clients[loop_var].sock_data=0;

		memset(in_thread.clients[loop_var].my_ip, 0, INET6_ADDRSTRLEN);
		memset(in_thread.clients[loop_var].buf_read, 0, SEND_SIZE);
		memset(in_thread.clients[loop_var].buf_send, 0, SEND_SIZE);
	}

/////////////////////////////////////////////////////////////


	//////Socket variables////

    int 		sock_status		=0;
	int 		sock_data		=0;
	int			sock_order		=0;
    struct 		addrinfo 			*serv_info;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

	addrlen=sizeof(remoteaddr);											//Added line as precaution

    char remoteIP[INET6_ADDRSTRLEN];

			//IP and PORT
	char *ip="server";
	char *status_port_num=STATUS_PORT;
	char *data_port_num=DATA_PORT;
	char *order_port_num=ORDER_PORT;

	//////////////////////////

	//////Timeout Struct////

	struct 		timeval 	tv;
	tv.tv_sec 				=0;
	tv.tv_usec 				=100*MS;		//100ms

	//////////////////////////

	//////GP Variables////
		
	int 	i					=0;
	int		recv_desc			=0;
	int 	sock_desc_client	=0;
	int	 	client_connected	=0;
	int 	temp_desc			=0;		//just for adding sock_data from udp thread....
	int		temp_desc_1			=0;		//just for adding sock_order from udp thread....
	int 	temp_index			=0;

	char 	my_IP[INET6_ADDRSTRLEN];

	//////////////////////////


	//////Fill Structs and FD parameters////
	
    serv_info=malloc(sizeof (struct addrinfo));
  
			//get THREE local sockets for listening...

	sock_status=init_network_tcp(ip, status_port_num, &serv_info);
	sock_data=init_network_tcp(ip, data_port_num, &serv_info);
	sock_order=init_network_tcp(ip, order_port_num, &serv_info);




			////initialize ip_shared struct

	pthread_mutex_init(&ip_shared.ip_mutex,NULL);		//initialize mutex!


	pthread_mutex_lock(&ip_shared.ip_mutex);

	FD_ZERO(&ip_shared.master_set);    
	FD_ZERO(&ip_shared.sock_set);                     //clear current contents


	FD_SET(sock_status, &ip_shared.master_set);      //include sockets into set
	FD_SET(sock_data, &ip_shared.master_set);
	FD_SET(sock_order, &ip_shared.master_set);


	ip_shared.fdmax= (sock_data > sock_status) ? sock_data : sock_status;			//first assign larger of sock_data or sock_status to fdmax
	ip_shared.fdmax= (ip_shared.fdmax > sock_order) ? ip_shared.fdmax : sock_order;	//now assign larger of sock_order or fdmax to fdmax!!!

	ip_shared.sock_status	=0;
	ip_shared.sock_data		=0;
	ip_shared.sock_order	=0;
	ip_shared.ip_flag		=0;

	memset(ip_shared.new_ip, 0, INET6_ADDRSTRLEN);

	pthread_mutex_unlock(&ip_shared.ip_mutex);

	////////////////////////////////////////////////////



	/////////////////Now polling part and/////////////// 	

	
	my_ip_function(my_IP);

	printf("TCP Accept Thread:  My IP is %s.\n",my_IP);


	while(1){

		//printf("TCP Accept Thread: Clients Connected are %d.\n",client_connected);


		pthread_mutex_lock(&ip_shared.ip_mutex);		//lock set mutex

		ip_shared.sock_set=ip_shared.master_set;

		select(ip_shared.fdmax+1,&ip_shared.sock_set,NULL,NULL,&tv);    //pass 0 instead of &tv as first correction to timeout issue
		

	//////////////Accepting new connection on sock_status  using select/////////////////

		    if(FD_ISSET(sock_status,&ip_shared.sock_set)){
						
						pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock set mutex

						printf("\t\t\tTCP Accept Thread:New ACCEPT Connection on Status Sock!!!\n");
						addrlen=sizeof(remoteaddr);

						sock_desc_client=accept(sock_status,(struct sockaddr *)&remoteaddr,&addrlen);

				//accept new connection


					if(sock_desc_client<0){
						perror("TCP Accept Thread:Status Sock,Error in accepting new connection:");
					}
				
					//now add new connection to set of socks to poll
					else {


						inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),remoteIP, sizeof (remoteIP));

						pthread_mutex_lock(&ip_shared.ip_mutex);

						temp_index=ip_check_function(remoteIP);
						printf("\t\t\tTCP Accept Thread: :NEW IP is %s and Temp_index for this ip is %d\n",remoteIP,temp_index);
						pthread_mutex_unlock(&ip_shared.ip_mutex);


								if(temp_index!=-1){		//this means already in list, so no need to incerment client count, just add sock_desc!!!!

										printf("TCP Accept Thread: Adding status socket for already EXISTING connection from %s socket %d.\n",remoteIP,sock_desc_client);


										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										
										in_thread.clients[temp_index].sock_status=sock_desc_client;	//only sock_desc, NOT ip since already exisitng!

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex


										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);	

								}	//if(temp_index!=-1){		//this 
						
								else {		//this means not in list so increment client count as well!

										printf("TCP Accept Thread:Adding status sock for NEW Connection from %s on  socket %d.\n",remoteIP,sock_desc_client);

										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										in_thread.clients[client_connected].sock_status=sock_desc_client;
										strncpy(in_thread.clients[client_connected].my_ip, remoteIP, INET6_ADDRSTRLEN);

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex

										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										strncpy(ipArray[ip_index], remoteIP, INET6_ADDRSTRLEN);
										
										ip_index++;

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);		

										client_connected++;			//actual number of clients connected to system!

									}	//else {	//this means


					}		//else { //now add new connection .....

		    }		//ISSET if ends here!!!

			else pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock_mutex for both of if(ISSET.....

	//////////////////////////////////////////////////////////////////////



	//////////////Accepting new connection on sock_data  using select/////////////////

			pthread_mutex_lock(&ip_shared.ip_mutex);

		   if(FD_ISSET(sock_data,&ip_shared.sock_set)){
						
						pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock set mutex

						printf("\t\t\tTCP Accept Thread:New ACCEPT Connection on Data Socket!!!\n");
						addrlen=sizeof(remoteaddr);

						sock_desc_client=accept(sock_data,(struct sockaddr *)&remoteaddr,&addrlen);

				//accept new connection

					if(sock_desc_client<0){
						perror("TCP Accept Thread:Data Socket,Error in accepting new connection:");
					}
				
					//now add new connection to set of socks to poll
					else {


						inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),remoteIP, sizeof (remoteIP));

						pthread_mutex_lock(&ip_shared.ip_mutex);

						temp_index=ip_check_function(remoteIP);

						pthread_mutex_unlock(&ip_shared.ip_mutex);

								if(temp_index!=-1){		//this means already in list, so no need to incerment client count, jus add sock_desc!!!!

										printf("TCP Accept Thread: Adding data socket for EXISTING connection from %s socket %d.\n",remoteIP,sock_desc_client);


										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										in_thread.clients[temp_index].sock_data=sock_desc_client;		//only sock_desc, NOT ip since already exisitng!*/
										

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex


										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);	

								}	//if(temp_index!=0){		//this 
						
								else {		//this means not in list so increment client count as well!

										printf("TCP Accept Thread:Adding data sock for NEW Connection from %s on  socket %d.\n",remoteIP,sock_desc_client);

										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										in_thread.clients[client_connected].sock_data=sock_desc_client;
										strncpy(in_thread.clients[client_connected].my_ip, remoteIP, INET6_ADDRSTRLEN);

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex


										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										strncpy(ipArray[ip_index], remoteIP, INET6_ADDRSTRLEN);
										ip_index++;

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);		

										client_connected++;			//actual number of clients connected to system!

									}	//else {	//this mea


					}		//else { //now add new connection .....

		    }		//ISSET if ends here!!!


		else pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock_mutex for both of if(ISSET.....
	//////////////////////////////////////////////////////////////////////




	//////////////Accepting new connection on sock_order  using select/////////////////

			pthread_mutex_lock(&ip_shared.ip_mutex);

		   if(FD_ISSET(sock_order,&ip_shared.sock_set)){
						
						pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock set mutex

						printf("\t\t\tTCP Accept Thread:New ACCEPT Connection on Order Socket!!!\n");
						addrlen=sizeof(remoteaddr);

						sock_desc_client=accept(sock_order,(struct sockaddr *)&remoteaddr,&addrlen);

				//accept new connection

					if(sock_desc_client<0){
						perror("TCP Accept Thread:Order Socket,Error in accepting new connection:");
					}
				
					//now add new connection to set of socks to poll
					else {


						inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr *)&remoteaddr),remoteIP, sizeof (remoteIP));

						pthread_mutex_lock(&ip_shared.ip_mutex);

						temp_index=ip_check_function(remoteIP);

						pthread_mutex_unlock(&ip_shared.ip_mutex);

								if(temp_index!=-1){		//this means already in list, so no need to incerment client count, jus add sock_desc!!!!

										printf("TCP Accept Thread: Adding order socket for EXISTING connection from %s socket %d.\n",remoteIP,sock_desc_client);


										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										in_thread.clients[temp_index].sock_order=sock_desc_client;		//only sock_desc, NOT ip since already exisitng!

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex


										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);	

								}	//if(temp_index!=0){		//this 
						
								else {		//this means not in list so increment client count as well!

										printf("TCP Accept Thread:Adding order sock for NEW Connection from %s on  socket %d.\n",remoteIP,sock_desc_client);

										pthread_mutex_lock(&in_thread.node_mutex);		//Edited part for thread, lock node mutex

										in_thread.clients[client_connected].sock_order=sock_desc_client;
										strncpy(in_thread.clients[client_connected].my_ip, remoteIP, INET6_ADDRSTRLEN);

										pthread_mutex_unlock(&in_thread.node_mutex);		 //unlock node mutex

										pthread_mutex_lock(&ip_shared.ip_mutex);		//for set operations, ask YB whether this is correct......

										strncpy(ipArray[ip_index], remoteIP, INET6_ADDRSTRLEN);
										ip_index++;

										FD_SET(sock_desc_client,&ip_shared.master_set);

											if(ip_shared.fdmax<sock_desc_client){				
												ip_shared.fdmax=sock_desc_client;
											}

										pthread_mutex_unlock(&ip_shared.ip_mutex);		

										client_connected++;			//actual number of clients connected to system!

									}	//else {	//this mea


					}		//else { //now add new connection .....

		    }		//ISSET if ends here!!!


		else pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock_mutex for both of if(ISSET.....
	//////////////////////////////////////////////////////////////////////





	//////////////Adding new connection from udp thread/////////////////


			pthread_mutex_lock(&ip_shared.ip_mutex);		//lock ip_shared mutex

				if(ip_shared.ip_flag==TRUE){

					strncpy(remoteIP,ip_shared.new_ip,INET6_ADDRSTRLEN);
					sock_desc_client=ip_shared.sock_status;
					temp_desc=ip_shared.sock_data;
					temp_desc_1=ip_shared.sock_order;

					pthread_mutex_unlock(&ip_shared.ip_mutex);	//unlock ip_shared mutex inside if


								printf("TCP Accept Thread: Adding new connection via UDP thread from %s on status socket %d data socket %d and order socket %d \n",remoteIP,sock_desc_client,temp_desc,temp_desc_1);

								pthread_mutex_lock(&in_thread.node_mutex);	//Edited part for thread, lock node mutex

								in_thread.clients[client_connected].sock_status=sock_desc_client;
								in_thread.clients[client_connected].sock_data=temp_desc;
								in_thread.clients[client_connected].sock_order=temp_desc_1;	
								strncpy(in_thread.clients[client_connected].my_ip,remoteIP,INET6_ADDRSTRLEN);

								pthread_mutex_unlock(&in_thread.node_mutex);	//unlock node mutex

									//add to set and clear flag for udp thread to proceed


								pthread_mutex_lock(&ip_shared.ip_mutex);		//lock set mutex


								FD_SET(sock_desc_client,&ip_shared.master_set);
								FD_SET(temp_desc,&ip_shared.master_set);		//here temp_desc is sock_data
								FD_SET(temp_desc_1,&ip_shared.master_set);

									if(ip_shared.fdmax<sock_desc_client){				
										ip_shared.fdmax=sock_desc_client;
									}

									if(ip_shared.fdmax<temp_desc){
										ip_shared.fdmax=temp_desc;		//check logic......
									}

									if(ip_shared.fdmax<temp_desc_1){
										ip_shared.fdmax=temp_desc_1;		//check logic......
									}



								ip_shared.ip_flag=FALSE;

								pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock ip_mutex

								client_connected++;
				

				}

				else pthread_mutex_unlock(&ip_shared.ip_mutex);		//unlock ip_shared mutex if not true...

	//////////////////////////////////////////////////////////////////////

		if(client_connected>=N_CLIENT){
			printf("TCP Accept Thread: Warning, clients connected are at max limit\n");
		}

	usleep(DELAY*MS);

	}	//while(1)
}






void* resolve_order_function(void* temp_struct){

		//shared structure part
	my_interface_t* status_thread=(my_interface_t* ) temp_struct;

	//GP variables
	int 	loop_var=0;
	int		temp_send_bytes=0;		//used for DOJOB_ order send message
	int 	recv_bytes[N_CLIENT];
	int		send_desc[N_CLIENT];

		//cost function part
	int		temp_desc[N_CLIENT+1];
	int		floor_int_array[N_CLIENT+1];
	char 	floor_char_array[N_CLIENT+1];
	char 	dir_array[N_CLIENT+1];
	int 	desired_index; 
	int 	temp_order_floor;
	char	temp_order_dir;
	
	int my_floor;
	char my_floor_char;
	char my_dir;

	char 	buf_read[SEND_SIZE];
	char 	buf_send_status[SEND_SIZE];
	char 	buf_send_order[SEND_SIZE];
	char 	buf_recv_ack[SEND_SIZE];
	char 	my_IP[INET6_ADDRSTRLEN];
	char 	my_status[SEND_SIZE];

	strncpy(buf_send_status,"SEND_STATUS",11);


	struct client_t 	local_client[N_CLIENT];

	struct cost_param	my_cost_struct;


	my_ip_function(my_IP);

	printf("Resolve Order Thread: My own IP is %s. \n",my_IP);


	while(1){

		pthread_mutex_lock(&status_thread->interface_mutex);

			if(status_thread->order_floor_flag==TRUE) {

				temp_order_floor=status_thread->order_floor;
				temp_order_dir=status_thread->order_direction;

				strncpy(my_status,status_thread->interface_status_buffer,SEND_SIZE);
			
				pthread_mutex_unlock(&status_thread->interface_mutex);
				


						//get sock descriptors.....

				pthread_mutex_lock(&in_thread.node_mutex);		//lock node mutex

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){
						local_client[loop_var].sock_status=in_thread.clients[loop_var].sock_status;
						local_client[loop_var].sock_data=in_thread.clients[loop_var].sock_data;
						local_client[loop_var].sock_order=in_thread.clients[loop_var].sock_order;
						strncpy(local_client[loop_var].my_ip,in_thread.clients[loop_var].my_ip,INET6_ADDRSTRLEN);
			
					}

				pthread_mutex_unlock(&in_thread.node_mutex);		//unlock node mutex	

				

							//send SEND_STATUS  to everyone non zero sock_status  and expect receive on sock_data from same client 

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){
							if(local_client[loop_var].sock_status!=0){
								
									send_desc[loop_var]=send(local_client[loop_var].sock_status, buf_send_status, SEND_SIZE, 0);
									usleep(50*MS);

										if(send_desc[loop_var]>0){
											
											
											recv_bytes[loop_var]=recv(local_client[loop_var].sock_data, local_client[loop_var].buf_read, SEND_SIZE, 0);
												if(recv_bytes[loop_var]<=0){
													printf("Resolve Order Thread: Status Request sent to client %s descriptor %d but nothing received\n.",local_client[loop_var].my_ip,local_client[loop_var].sock_order);}										
												else {
													/*Debug part*/
													if(!strncmp(local_client[loop_var].buf_read,"MY_STATUS_",10)){;}
													else {printf("Resolve Order Thread: Status Reply Recieved in wrong format from client %s and is %s\n",local_client[loop_var].my_ip,local_client[loop_var].buf_read);}
												}
													/*////////////*/

										}	//add check here to exclude wrong format replies

							}	//if(local_client[loop_var].sock_status!=	
							
							else	
								usleep(50*MS);			

					}	//for(i=0;i<N_

							
											//extract  MY OWN status at location zero of all arrays.....
								subString(my_status, 10, 1, &my_cost_struct.temp_floor);
								my_cost_struct.temp_floor[1] = '\0';
								my_cost_struct.floor[0]=atoi(&my_cost_struct.temp_floor);									
								subString(my_status, 12, 1, &my_cost_struct.direction[0]);
								my_cost_struct.index[0]=1;

								
					int index =1;

					for(loop_var=0; loop_var<N_CLIENT; loop_var++){

							if(recv_bytes[loop_var]>0){

							


											//extract floors...
								subString(local_client[loop_var].buf_read, 10, 1, &my_cost_struct.temp_floor);
								my_cost_struct.temp_floor[1] = '\0';
								my_cost_struct.floor[index]=atoi(&my_cost_struct.temp_floor);
												//extract directions...
								subString(local_client[loop_var].buf_read, 12, 1, &my_cost_struct.direction[index]);
												//copy sock_desc....
								my_cost_struct.index[index]=loop_var+1;

								printf("Resolve Order Thread: Received Client's Floor is %d Direction is %c and index is %d\n",my_cost_struct.floor[index],my_cost_struct.direction[index],my_cost_struct.index[index]);	//debug

								index++;
								//usleep(50*MS);
				
							
							}		//if(recv_bytes[loop_
								
					}	//for(loop_var=0; loop_var<N_CLIE

						my_cost_struct.max_index=index;		//maximum number of clients connected including myself.....

										//call cost function...
						desired_index=cost_function( my_cost_struct, temp_order_floor, temp_order_dir);					

						if(desired_index!=0){
							
							sprintf(buf_send_order, "DOJOB_%d_%c", temp_order_floor,temp_order_dir);
							temp_send_bytes=send(local_client[desired_index-1].sock_order, buf_send_order, SEND_SIZE, 0);		//SOCK_ORDER!!!
							usleep(50*MS);		//maybe some delay but should not be, ideally
							printf("\t\t\tResolve Order Thread:Dispatching job %s to client %s order is %d\n",buf_send_order,local_client[desired_index-1].my_ip,temp_order_floor);
						
								if(temp_send_bytes<=0){

									printf("Resolve Order Thread: Failed on sending order to %s client, I shall do this job\n",local_client[desired_index-1].my_ip);

										pthread_mutex_lock(&status_thread->interface_mutex);

										status_thread->order_floor=temp_order_floor;
										status_thread->order_floor_flag=FALSE;

										pthread_mutex_unlock(&status_thread->interface_mutex);

								}	//if(temp_send_bytes<=


								else {	//else send_bytes were good and go for ACK message

												pthread_mutex_lock(&status_thread->interface_mutex);

												status_thread->order_floor=-1;			//clear for main to not add this job in its own queue
												status_thread->order_floor_flag=FALSE;

												pthread_mutex_unlock(&status_thread->interface_mutex);


								}	//else send_bytes were good and go for ACK message

								usleep(50*MS);

						}	//if(desired_index!


						else {	//myself has lowest cost...


									printf("Resolve Order Thread:Minimum cost is myself!\n.");
						
									pthread_mutex_lock(&status_thread->interface_mutex);

									status_thread->order_floor=temp_order_floor;
									status_thread->order_floor_flag=FALSE;

									pthread_mutex_unlock(&status_thread->interface_mutex);
									
						}	//else myself has....

						/*clear struct....*/

						for(loop_var=0; loop_var<N_CLIENT; loop_var++){

						my_cost_struct.floor[loop_var]=0;
						my_cost_struct.direction[loop_var]=0;
						my_cost_struct.index[loop_var]=0;
						recv_bytes[loop_var]=0;
						
						}
			
					
			
			}	//if(status_thread->order_f

			else pthread_mutex_unlock(&status_thread->interface_mutex);		//else rrlease mutex outside thread.....




		usleep((DELAY)*MS);
	}	//while(1)




}



void* send_status_function(void *temp_struct){


		//shared structure part
	my_interface_t* recv_thread=(my_interface_t* ) temp_struct;




	//GP variables
	int 	loop_var=0;
	int 	recv_bytes=0;
	int 	send_bytes=0;
	int 	temp_flag_read=0;
	int 	temp_flag_array[N_CLIENT];

	char 	temp_buf_recv_status[SEND_SIZE];	//temp_buffer
	char 	temp_buf_interface[SEND_SIZE];		//temp_buffer
	char 	my_status[SEND_SIZE];				//put data from interface buffer
	char 	my_IP[INET6_ADDRSTRLEN];

	char 	temp_floor;

	fd_set recv_set;
	int 	recv_max;
	FD_ZERO(&recv_set);





	////////////////////////////////////////


	//////Timeout Struct////

	struct 		timeval 	tv;
	tv.tv_sec 				= 0;
	tv.tv_usec 				= 100*MS;		//100ms

	//////////////////////////


		//local copy of client struct to avoid multiple mutex locks inside loops

	struct 	client_t 	local_client[N_CLIENT];

	my_ip_function(my_IP);

	printf("Send Status TCP Thread: My own IP is %s.\n",my_IP);



	while(1){

				//not sure about this approach, ask yulong whether this works or we will have a deadlock!

					/*	Step 1) copy sokc descriptors... 	*/

		pthread_mutex_lock(&in_thread.node_mutex);		//lock node mutex and copy all descriptors to close connections !!!!

			for(loop_var=0;loop_var<N_CLIENT;loop_var++){

				local_client[loop_var].sock_status=in_thread.clients[loop_var].sock_status;
				local_client[loop_var].sock_data=in_thread.clients[loop_var].sock_data;
				local_client[loop_var].sock_order=in_thread.clients[loop_var].sock_order;
				strncpy(local_client[loop_var].my_ip,in_thread.clients[loop_var].my_ip,INET6_ADDRSTRLEN);			

			}
	
		pthread_mutex_unlock(&in_thread.node_mutex);		//unlock node mutex

					/*	Step 2) copy interface buffer, here DONOT change send status flag (set true in main)....... 	*/



		pthread_mutex_lock(&recv_thread->interface_mutex);

		strncpy(temp_buf_interface,recv_thread->interface_status_buffer,SEND_SIZE);

		pthread_mutex_unlock(&recv_thread->interface_mutex);



		pthread_mutex_lock(&ip_shared.ip_mutex);		//lock set mutex

		recv_set=ip_shared.master_set;
		recv_max=ip_shared.fdmax;

		pthread_mutex_unlock(&ip_shared.ip_mutex);


		
		int i=select(recv_max+1,&recv_set,NULL,NULL,&tv);    //pass 0 instead of &tv as first correction to timeout issue
		if(i==-1){printf("Send Status TCP Thread:Select Failed\n");}

		//////////////Receiving new data/////////////////
				for(loop_var=0;loop_var<N_CLIENT;loop_var++){
				
					if(FD_ISSET(local_client[loop_var].sock_status, &recv_set)){

						recv_bytes=recv(local_client[loop_var].sock_status, local_client[loop_var].buf_read, SEND_SIZE, 0);


						if(recv_bytes<=0){
							if(recv_bytes==0){

								printf("Send Status TCP Thread:Client %s disconnected on recv error closing all sockets %d bytes recvived.\n",local_client[loop_var].my_ip,recv_bytes);
								close(local_client[loop_var].sock_status);        
								close(local_client[loop_var].sock_data);        	
								close(local_client[loop_var].sock_order);			//client's connection closed all sockets...


								pthread_mutex_lock(&ip_shared.ip_mutex);
							
								int delete_flag=0;
								while(!(delete_flag=ip_delete_function(local_client[loop_var].my_ip)));

								printf("Send Status TCP Thread:Deleting %s ip SUCCESS.\n",local_client[loop_var].my_ip);//}
								
								memset(local_client[loop_var].my_ip,0,INET6_ADDRSTRLEN);

								FD_CLR(local_client[loop_var].sock_status, &ip_shared.master_set);
								FD_CLR(local_client[loop_var].sock_data, &ip_shared.master_set);
								FD_CLR(local_client[loop_var].sock_order, &ip_shared.master_set);

								pthread_mutex_unlock(&ip_shared.ip_mutex);

								local_client[loop_var].sock_status=0;				//clear sock status
								local_client[loop_var].sock_data=0;				//clear sock data both sockets...!!!!
								local_client[loop_var].sock_order=0;
							}
							else{
								perror("RX_ERROR:");
							}


						}	//if(recv_bytes<=0)

						else{


								/*Debug part only*/
								if(!strncmp(local_client[loop_var].buf_read, "SEND_STATUS", 11)){;}
								else {printf("\t\t\tSend Status TCP Thread:WRONG Message for status\n");}
								///////////////////
					
							send_bytes=send(local_client[loop_var].sock_data, temp_buf_interface, SEND_SIZE, 0);

								if(send_bytes<=0){

											printf("Send Status TCP Thread:Sending status failed on data channel for %s \n",local_client[loop_var].my_ip);

								}
							
								else {	//else (paired with if(send_bytes....)

									printf("Send Status TCP Thread: Status successfully sent to client %s.\n",local_client[loop_var].my_ip);

								} 	//else (paired with if(send_bytes....)
								//usleep(50*MS);//										!!!!!!!!!!!!!!!!!!!
				
						}  //else something received*/
              
					}	//if(FD_SET...

					//usleep(50*MS);

				}	//for(i=0;....


								//update sock descriptors....

				pthread_mutex_lock(&in_thread.node_mutex);		//lock node mutex

					for(loop_var=0;loop_var<N_CLIENT;loop_var++){

						in_thread.clients[loop_var].sock_status=local_client[loop_var].sock_status;
						in_thread.clients[loop_var].sock_data=local_client[loop_var].sock_data;			 
						in_thread.clients[loop_var].sock_order=local_client[loop_var].sock_order;		//update all descriptors, 
						strncpy(in_thread.clients[loop_var].my_ip,local_client[loop_var].my_ip,INET6_ADDRSTRLEN);	

					}

				pthread_mutex_unlock(&in_thread.node_mutex);		//unlock node mutex	 
            



	

	usleep(DELAY*MS);
	
	}	//while(1)

}






void* accept_order_function(void *temp_struct){


		//shared structure part
	my_interface_t* order_thread=(my_interface_t* ) temp_struct;




	//GP variables
	int 	loop_var=0;
	int 	recv_bytes=0;
	int 	send_bytes=0;
	int 	temp_received_floor=0;
	char 	temp_received_direction[2];
	int 	temp_flag_read=0;
	int 	temp_flag_array[N_CLIENT];

	char 	my_IP[INET6_ADDRSTRLEN];
	char 	temp_buf_ack[SEND_SIZE];
	char	temp_floor_in_str[2];

	strncpy(temp_buf_ack, "ACK_MSG", 7);


		//local set variables
	fd_set 	order_set;
	int 	order_max;
	FD_ZERO(&order_set);





	////////////////////////////////////////


	//////Timeout Struct////

	struct 		timeval 	tv;
	tv.tv_sec 				= 0;
	tv.tv_usec 				= 100*MS;		//100ms

	//////////////////////////


		//local copy of client struct to avoid multiple mutex locks inside loops

	struct 	client_t 	local_client[N_CLIENT];

	my_ip_function(my_IP);

	printf("Accept Order Thread: My own IP is %s.\n",my_IP);



	while(1){


		//printf("Accept order Thread is Alive\n");
					/*	Step 1) copy sokc descriptors... 	*/

		pthread_mutex_lock(&in_thread.node_mutex);		//lock node mutex and copy only order to close connections !!!!

			for(loop_var=0;loop_var<N_CLIENT;loop_var++){

				local_client[loop_var].sock_order=in_thread.clients[loop_var].sock_order;	
				local_client[loop_var].sock_data=in_thread.clients[loop_var].sock_data;
				strncpy(local_client[loop_var].my_ip,in_thread.clients[loop_var].my_ip,INET6_ADDRSTRLEN);					

			}
	
		pthread_mutex_unlock(&in_thread.node_mutex);		//unlock node mutex


			/*	Step 2) copy set variables... 	*/
		pthread_mutex_lock(&ip_shared.ip_mutex);		//lock set mutex

		order_set=ip_shared.master_set;
		order_max=ip_shared.fdmax;

		pthread_mutex_unlock(&ip_shared.ip_mutex);
		
			/*Now start polling.....*/
		
		select(order_max+1,&order_set,NULL,NULL,&tv);    //pass 0 instead of &tv as first correction to timeout issue

		//////////////Receiving new data/////////////////
				for(loop_var=0;loop_var<N_CLIENT;loop_var++){
					
					if(FD_ISSET(local_client[loop_var].sock_order, &order_set)){

						recv_bytes=recv(local_client[loop_var].sock_order, local_client[loop_var].buf_read, SEND_SIZE, 0);


						if(recv_bytes<=0){

							printf("Accept Order Thread: Something wrong with this socket,client  %s should be closed %d bytes received.\n",local_client[loop_var].my_ip,recv_bytes);

						}	//if(recv_bytes<=0)

						else{	//else something received
								
								
								/*Debug part only*/
								if(!strncmp(local_client[loop_var].buf_read, "DOJOB_", 6)){;}
								else {printf("\t\t\tAccept Order Thread:WRONG Message for status from %s\n",local_client[loop_var].my_ip);}
								///////////////////
			
							subString(local_client[loop_var].buf_read, 6, 1, temp_floor_in_str);
							printf("%s", temp_floor_in_str);
							temp_floor_in_str[1] = '\0';
							temp_received_floor=atoi(temp_floor_in_str);

							subString(local_client[loop_var].buf_read, 8, 1, temp_received_direction);
							temp_received_direction[1] = '\0';

							/*Debug Part*/
						printf("\t\t\tAccept Order Thread: Order Received; message is %s Floor is %d from %s\n.",local_client[loop_var].buf_read,temp_received_floor,local_client[loop_var].my_ip);

							pthread_mutex_lock(&order_thread->interface_mutex);

							order_thread->received_floor=temp_received_floor;
							order_thread->received_direction=temp_received_direction[0];
							order_thread->received_floor_flag=TRUE;

							pthread_mutex_unlock(&order_thread->interface_mutex);

				
						}  //else something received
              
					}	//if(FD_SET...

					//usleep(50*MS);

				}	//for(i=0;....
	

	usleep(DELAY*MS);
	
	}	//while(1)

}




void* initialzie_function(void * temp_struct){

pthread_t udp_discover_thread;
pthread_t tcp_accept_thread;
pthread_t send_status_thread;
pthread_t accept_order_thread;
pthread_t resolve_order_thread;


					//create interface_t struct for sharing with comm. moduel
	my_interface_t* in_init_interface=(my_interface_t* ) temp_struct;

				//initialize struct to desired state
	pthread_mutex_init(&in_init_interface->interface_mutex,NULL);

 	in_init_interface->received_floor_flag=FALSE;		//add order in queue and clear in main
	in_init_interface->order_floor_flag=FALSE;
	in_init_interface->received_floor=0;
	in_init_interface->order_floor=0;
	in_init_interface->order_direction='U';
	memset(in_init_interface->interface_status_buffer, 0, SEND_SIZE);


			//spawn comm. module threads....
pthread_create(&tcp_accept_thread, NULL, tcp_accept_function, NULL);		//maybe some delay to let it settle down....
usleep(100*MS);
pthread_create(&udp_discover_thread, NULL, discover_udp_function, NULL);
pthread_create(&send_status_thread, NULL, send_status_function, in_init_interface);
pthread_create(&accept_order_thread, NULL, accept_order_function, in_init_interface);
pthread_create(&resolve_order_thread, NULL, resolve_order_function, in_init_interface);

while(1){sleep(10);}

pthread_join(udp_discover_thread, NULL);
pthread_join(tcp_accept_thread, NULL);
pthread_join(send_status_thread, NULL);
pthread_join(accept_order_thread, NULL);
pthread_join(resolve_order_thread, NULL);

}











	int ip_check_function(char *ip){			//if new connectio, it will return -1 otherwise it will return 0 to N_CLIENT i.e. ip_index of that client 
		int i;
	
			for(i=0;i<N_CLIENT;i++){

				if(!strncmp(ip,ipArray[i],INET6_ADDRSTRLEN)) return i;		//return index; to be used in accept function!!!! 
							
								
			}

		return -1;	//-1 means not in list
	}





	int connect_function(char *ip, char *port_num){

	//socket structs and variables
 	struct addrinfo *client_info_tcp;
	client_info_tcp=malloc(sizeof (struct addrinfo));

	//GP variables
	int sock_desc=0;

	sock_desc=init_network_tcp(ip,port_num,&client_info_tcp);


		if(sock_desc<=0){
			return -1;
		}
		else{
			return sock_desc;
		}
	}






	void my_ip_function(char *ip){
				int fd;
				struct ifreq ifr;

				fd = socket(AF_INET, SOCK_DGRAM, 0);


				ifr.ifr_addr.sa_family = AF_INET;

	 
				strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

				ioctl(fd, SIOCGIFADDR, &ifr);

				close(fd);

			
				strncpy(ip,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),INET_ADDRSTRLEN);
		
	}





	int ip_delete_function(char *ip){

		int i;
	
			for(i=0;i<N_CLIENT;i++){
				if(!strncmp(ip,ipArray[i],INET6_ADDRSTRLEN)) {memset(ipArray[i],0,INET6_ADDRSTRLEN); return 1;}	//if ip matches, delete ip from array
							
								
			}

		return 0;	

	}






	char* subString (const char* input, int offset, int len, char* dest)
	{
	  int input_len = strlen (input);

	  if (offset + len > input_len)
	  {
		 return NULL;
	  }

	  strncpy (dest, input + offset, len);
	  return dest;
	}


	void makeStatus(char *status, int floor, char dir){

 
	sprintf(status, "MY_STATUS_%d_%c",floor,dir);

	}


	int cost_function(struct cost_param my_cost_struct, int temp_order_floor, char temp_order_dircetion){

	int i=0;
	int minimum, location = 0;
	int temp_diff[N_CLIENT];
	int minimum_one_s_array_indexes[N_CLIENT];/* several ones are minimum */
	int mosai_index = 0;
	int the_opt_idx = 0;
	memset(minimum_one_s_array_indexes, 0xfff, N_CLIENT*sizeof(int));
		for(i=0; i<my_cost_struct.max_index; i++)	{
			temp_diff[i]=temp_order_floor-my_cost_struct.floor[i];
				if(temp_diff[i]<0){temp_diff[i]=(-1*temp_diff[i]);}		
		}

			minimum = temp_diff[0];
	 
		for ( i = 1 ; i <my_cost_struct.max_index ; i++ ) 
		{
		    if ( temp_diff[i] < minimum )
		    {
		       minimum = temp_diff[i];
		       //location = i+1;
		       the_opt_idx = i;
		    }
		} 

		for (int i = 1; i < my_cost_struct.max_index; ++i) {
			if (temp_diff[i] == minimum){
				minimum_one_s_array_indexes[mosai_index] = i;
				mosai_index++;
			}
		}

		for (int i = 0; i < mosai_index; ++i) {
			int temp = minimum_one_s_array_indexes[mosai_index];
			if(temp_order_dircetion == my_cost_struct.direction[temp]){
				the_opt_idx = temp;
				/* the first matched direction wins */
				break;
			}

		}
		//location = the_opt_idx + 1;
		if(the_opt_idx==0){return 0;}

		 printf("From Cost Fucntion, min is %d and index is %d, direction is %c \n",minimum, the_opt_idx, my_cost_struct.direction[the_opt_idx]);
		return my_cost_struct.index[the_opt_idx];
	}





