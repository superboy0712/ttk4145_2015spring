/*****************************************************************
 * error_handle.c
 * process_pairs
 *
 *  Created on		: Feb 20, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  Description		:
 *****************************************************************/
#include <stdlib.h>
#define error_handle( true , s) \
		do {\
		if(true){ \
		perror(s);\
		exit(EXIT_FAILURE);\
		} } while(0)

int main(int argc, char **argv) {
	error_handle(1, "hello world!");
	return 0;
}


