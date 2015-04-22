#ifndef NETWORK_MODULE_TCP_H
#define NETWORK_MODULE_TCP_H



#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "network_module_udp.h"


#define BACKLOG_TCP 10




/*Arguments:

1) ip= IP address in string format. (pass "server" as string in case of server program
2) port_num=Network Port to be connected with.
3) serv_info=Structure containing Server/Client details.
	Returns socket descriptor
Calling this function from main:
	1)struct addrinfo *serv_info;
	2) serv_info=malloc(sizeof (struct addrinfo));
     	3) sock_desc=init_network_udp(ip, port_num,&serv_info);
then you may use like;
recv_bytes=receive_string_udp(buf_recv,size,sock_desc, serv_info);
send_bytes=send_string_udp(buf_send,size,sock_desc,serv_info);
*/




int init_network_tcp(char *ip,char *port_num, struct addrinfo **serv_info);




/*Arguments:

1) buf_send=Pointer to buffer data to be sent
2) size= Size of data in bytes to be sent.
3) sock_desc=Socket_descriptor.
4) serv_info=Server/Client Address details.
	Returns no. of bytes sent


int send_string_udp(char *buf_send, int size,int sock_desc,struct addrinfo *serv_info);
int send_integer_udp(int *buf_send, int size,int sock_desc, struct addrinfo *serv_info);




/*Arguments:

1) buf_send=Pointer to buffer data to be sent
2) size= Size of data in bytes to be sent.
3) sock_desc=Socket_descriptor.
4) serv_info=Server/Client Address details.
	Returns no. of bytes received

int receive_string_udp(char *buf_recv,int size,int sock_desc, struct addrinfo *serv_info);
int receive_integer_udp(int *buf_recv,int size,int sock_desc, struct addrinfo *serv_info);


int reconnect_server_tcp();

int reconnect_client_tcp();
*/

//void *get_in_addr(struct sockaddr *sa);


#endif
