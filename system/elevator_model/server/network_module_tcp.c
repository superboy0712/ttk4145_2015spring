
#include "network_module_tcp.h"


#define TIMEOUT_MS 100



int init_network_tcp(char *ip,char *port_num, struct addrinfo **serv_info_p)
{

	struct addrinfo hints;
	int sock_desc=0;
	struct addrinfo * serv_info;
	//struct for timeout part
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT_MS*1000;

	//Variables
	int yes=1,rv=0,listen_desc=0,connect_desc=0;


		//Sockets variables
	    memset(&hints, 0, sizeof hints);
	    hints.ai_family = AF_INET;          // set to AF_INET to force IPv4
	    hints.ai_socktype = SOCK_STREAM;	//TCP Sockets




			///////////////////////////////////////////
			/////////////server socket/////////////////
			//////////////////////////////////////////


			if(!strncmp(ip,"server",6)){

					hints.ai_flags = AI_PASSIVE; // use my IP, since server

					//Fill IP address struct.
				if ((rv = getaddrinfo(NULL,port_num, &hints, serv_info_p)) != 0) {
				    	perror("GetAddrInfo TCP error :");
					return -50;
				    	}
					serv_info = *serv_info_p;
					//create socket descriptor.
				if ((sock_desc = socket(serv_info->ai_family, serv_info->ai_socktype,serv_info->ai_protocol)) == -1) {
					perror("Socket Descriptor TCP error:");
					return -50;
				    }


					//set socket options, reusable and timeout.
				if (setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
				    	perror("SetSockopt TCP error :");
				    	return -50;
				    }

				if (setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
				    	perror("SetSockopt Timeout TCP error :");
				    	return -50;
				    }

				
					//now bind socket to PORT..
				if (bind(sock_desc, serv_info->ai_addr, serv_info->ai_addrlen) == -1) {
					close(sock_desc);
					perror("Bind error TCP:");
					return -50;
				    }


					//Start Listening for connections.
				if((listen_desc=listen(sock_desc, BACKLOG_TCP))==-1) {
					close(sock_desc);
					perror("Listen error TCP:");
					return -50;

				   }
				printf("\nServer has created TCP Socket successfully, now wating for accepting connections...\n");

			}

			///////////////////////////////////////////
			/////////////client socket/////////////////
			//////////////////////////////////////////

			else {
				if ((rv = getaddrinfo(ip, port_num, &hints, serv_info_p)) != 0) {
				    	perror("GetAddrInfo TCP error :");
					return -50;
					}

					serv_info = *serv_info_p;
					//create socket descriptor.
				if ((sock_desc = socket(serv_info->ai_family, serv_info->ai_socktype,serv_info->ai_protocol)) == -1) {
					perror("Socket Descriptor TCP error:");
					return -50;
				    }


					//set socket options, reusable and timeout.
				if (setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
				    	perror("SetSockopt TCP error :");
				    	return -50;
				    }

				if (setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
				    	perror("SetSockopt Timeout TCP error :");
				    	return -50;
				    }


					//Connect to server.
				if((connect_desc=connect(sock_desc,serv_info->ai_addr,serv_info->ai_addrlen))==-1)
				{
					perror("ERROR IN CONNECTING TCP:");
					close(connect_desc);
					close(sock_desc);
				    	return -50;			
				}

				char serv_ip[INET_ADDRSTRLEN];

				inet_ntop(serv_info->ai_family,get_in_addr((struct sockaddr *) serv_info->ai_addr),serv_ip,sizeof (serv_ip));

				printf("\n TCP Client is connected to server having IP address:%s Port number: %s\n",serv_ip,port_num);

			}



			    //printf("Successfully created TCP Socket\n");




	return sock_desc;
}



/*
void *get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}


*/
