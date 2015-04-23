#include "network_module_udp.h"

#define M_SEC 1000

int init_network_udp(char *ip, char *port_num, struct addrinfo **serv_info_p) {

	struct addrinfo hints;
	int sock_desc = 0;
	struct addrinfo * serv_info;
	//struct for timeout part
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100 * M_SEC;

	//Variables
	int yes = 1, broadcast = 1, rv = 0;

	//Sockets variables
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;          // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;

	///////////////////////////////////////////
	/////////////server socket/////////////////
	//////////////////////////////////////////

	if (!strncmp(ip, "server", 6)) {

		hints.ai_flags = AI_PASSIVE;    // use my IP

		//Fill IP address struct
		if ((rv = getaddrinfo(NULL, port_num, &hints, serv_info_p)) != 0) {
			perror("GetAddrInfo UDP error :");
			return -1;
		}
		serv_info = *serv_info_p;
		//create socket descriptor
		if ((sock_desc = socket(serv_info->ai_family, serv_info->ai_socktype,
				serv_info->ai_protocol)) == -1) {
			perror("Socket Descriptor UDP error:");
			return -1;
		}

		//set socket options, reusable and timeout
		if (setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("SetSockopt UDP error :");
			return -1;
		}

		/*					if (setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		 perror("SetSockopt Timeout UDP error :");
		 return -1;
		 }
		 if (setsockopt(sock_desc, SOL_SOCKET, SO_BROADCAST, &broadcast,
		 sizeof broadcast) == -1) {
		 perror("setsockopt UDP (SO_BROADCAST)");
		 exit(1);
		 }*/

		//now bind socket to PORT..
		if (bind(sock_desc, serv_info->ai_addr, serv_info->ai_addrlen) == -1) {
			close(sock_desc);
			perror("Bind UDP error:");
			return -1;
		}

		printf("Successfully created UDP LOCAL Socket\n");

	}

	///////////////////////////////////////////
	/////////////client socket/////////////////
	//////////////////////////////////////////

	else {
		if ((rv = getaddrinfo(ip, port_num, &hints, serv_info_p)) != 0) {
			perror("GetAddrInfo UDP error :");
			return -1;
		}

		serv_info = *serv_info_p;
		//create socket descriptor
		if ((sock_desc = socket(serv_info->ai_family, serv_info->ai_socktype,
				serv_info->ai_protocol)) == -1) {
			perror("Socket Descriptor UDP error:");
			return -1;
		}

		//set socket options, reusable and timeout
		if (setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("SetSockopt UDP error :");
			return -1;
		}

		if (setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))
				< 0) {
			perror("SetSockopt Timeout UDP error :");
			return -1;
		}
		if (setsockopt(sock_desc, SOL_SOCKET, SO_BROADCAST, &broadcast,
				sizeof broadcast) == -1) {
			perror("setsockopt UDP (SO_BROADCAST)");
			return -1;
		}

		printf("Successfully created UDP REMOTE Socket\n");
	}

	// printf("Successfully created UDP Socket\n");

	return sock_desc;
}

/////////UDP String send//////////

int send_string_udp(char *buf_send, int size, int sock_desc,
		struct addrinfo *serv_info) {
	int send_size = 0;

	send_size = sendto(sock_desc, buf_send, size, 0, serv_info->ai_addr,
			serv_info->ai_addrlen);
	if (send_size <= 0) {
		perror("sendto UDP:");
		return -1;
	}

	return send_size;

}

/////////UDP Integer send//////////

int send_integer_udp(int *buf_send, int size, int sock_desc,
		struct addrinfo *serv_info) {
	int send_size = 0;

	if (serv_info == NULL) {
		printf("null serv UDP info");
		return -2;
	}
	send_size = sendto(sock_desc, buf_send, size, 0, serv_info->ai_addr,
			serv_info->ai_addrlen);
	if (send_size <= 0) {
		perror("sendto UDP:");
		return -1;
	}

	return send_size;

}

/////////UDP String Receive//////////

int receive_string_udp(char *buf_recv, int size, int sock_desc,
		struct addrinfo *serv_info) {
	int receive_size = 0;
	socklen_t sin_size;
	struct sockaddr_storage their_addr;
	sin_size = sizeof(their_addr);

	receive_size = recvfrom(sock_desc, buf_recv, size, 0,
			(struct sockaddr *) &their_addr, &sin_size);
	if (receive_size <= 0) {
		perror("recvfrom UDP:");
		return -1;
	}
	return receive_size;

}

/////////UDP Integer Receive//////////

int receive_integer_udp(int *buf_recv, int size, int sock_desc,
		struct addrinfo *serv_info) {
	int receive_size = 0;
	socklen_t sin_size;
	struct sockaddr_storage their_addr;
	sin_size = sizeof(their_addr);

	receive_size = recvfrom(sock_desc, buf_recv, size, 0,
			(struct sockaddr *) &their_addr, &sin_size);
	if (receive_size <= 0) {
		perror("recvfrom UDP:");
		return -1;
	}

	return receive_size;

}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

