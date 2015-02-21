/*****************************************************************
 * server_client_pairs.c
 * process_pairs
 *
 *  Created on		: Feb 21, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  Description		:
 *****************************************************************/
#include "server_thread.h"
#include "client_thread.h"
#include <stddef.h>
#include <pthread.h>
pthread_t server_tid, client_tid;
typedef struct {
	pthread_mutex_t * const mtx;
	pthread_cond_t * const cv;
} thread_arg_t;
pthread_mutex_t signal_master_dead_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t signal_master_dead_cv = PTHREAD_COND_INITIALIZER;
pthread_cond_t signal_server_bind_failed_cv = PTHREAD_COND_INITIALIZER;
thread_arg_t th_arg =
{
	.mtx = &signal_master_dead_mtx,
	.cv = &signal_master_dead_cv
};
int main(void) {

	pthread_create(&server_tid, NULL, server_main, (void*)&th_arg);
	pthread_create(&client_tid, NULL, client_main, (void*)&th_arg);
	pthread_join(server_tid, NULL);
	pthread_join(client_tid, NULL);
	return 1;
}


