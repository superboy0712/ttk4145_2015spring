/*
 * @file taskpool_policies_wrapper.c
 *
 *  Created on: Mar 30, 2015
 *      Author: yulongb
 */

#include "lift_task_queue.h"
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
static unsigned int N_LENGTH_OF_TASK_POOL = 4;
static request_type_t *task_pool_secret = (void *)0;
static pthread_mutex_t task_pool_lock = PTHREAD_MUTEX_INITIALIZER;
void default_task_pool_init( unsigned int length){
	N_LENGTH_OF_TASK_POOL = length;
	get_nearest_length_init(length);
	task_pool_secret = malloc(length*sizeof(request_type_t));
	memset(task_pool_secret, 0, length*sizeof(request_type_t));
	if(!task_pool_secret){
		perror("default_task_pool_init, bad malloc");
		exit(-1);
	}
}
void default_task_pool_destroy(void){
	if(task_pool_secret){
		free(task_pool_secret);
	}
	N_LENGTH_OF_TASK_POOL = 4;
}
void default_task_pool_print(void){
	print_request_pool(task_pool_secret, N_LENGTH_OF_TASK_POOL);
}
void push_request(int floor, request_type_t type){
	pthread_mutex_lock(&task_pool_lock);
	set_request_type(task_pool_secret+floor, type);
	pthread_mutex_unlock(&task_pool_lock);
}

void pop_request(int floor, request_type_t type){
	pthread_mutex_lock(&task_pool_lock);
	clr_request_type(task_pool_secret+floor, type);
	pthread_mutex_unlock(&task_pool_lock);
}

request_type_t get_request(int floor, request_type_t type){
	return get_request_type(task_pool_secret+floor, type);
}
static int get_optimal_request_from_specified_on_search_direction(
		const request_type_t * const pool, int specified, int *direction,
		request_type_t * ret_type) {
	int failed_count = 0;
	int ret = specified;
	if (*direction == 0){
		*ret_type = request_up_dn_cmd;
		return ret;
	}
	while (1) {
		/* failed even */
		if (failed_count % 2 == 0){
			if (*direction > 0) {
				ret = get_nearest_request_of_specified_upward(pool, specified,
						request_up_n_cmd);
				*ret_type = request_up_n_cmd;
			} else {
				ret = get_nearest_request_of_specified_downward(pool, specified,
						request_dn_n_cmd);
				*ret_type = request_dn_n_cmd;
			}

			if (ret == -1) {
				failed_count++;
				*direction = -*direction;
			} else {
				return ret;
			}
		}
		/* failed odd */
		if (failed_count % 2 == 1) {
			if (*direction > 0) {
				ret = get_nearest_request_of_specified_upward(pool, 0,
						request_up_n_cmd);
				*ret_type = request_up_n_cmd;
			}
			else {
				ret = get_nearest_request_of_specified_downward(pool,
				N_LENGTH_OF_TASK_POOL - 1, request_dn_n_cmd);
				*ret_type = request_dn_n_cmd;
			}
			if (ret == -1) {
				failed_count++;
				*direction = -*direction;
			} else {
				return ret;
			}
		}
		/* failed limit */
		if (failed_count >= 4){
			*ret_type = request_empty;
			return -1;
		}
	}
	return ret;
}
static unsigned int get_nr_of_req(const request_type_t * pool, unsigned int length) {
	int ret = 0;
	for (int i = 0; i < length; i++) {
		switch (pool[i]) {
		case request_call_up:
		case request_call_down:
		case request_call_cmd:
			ret++;
			break;
		case request_call_up | request_call_down:
		case request_call_up | request_call_cmd:
		case request_call_cmd | request_call_down:
			ret += 2;
			break;
		case request_call_up | request_call_down | request_call_cmd:
			ret += 3;
			break;
		default:
			break;
		}
	}
	return ret;
}
int get_optimal_req(int from, int *dir, request_type_t *ret_type){
	pthread_mutex_lock(&task_pool_lock);
	int rc = get_optimal_request_from_specified_on_search_direction(
			task_pool_secret, from, dir,
			ret_type);
	pthread_mutex_unlock(&task_pool_lock);
	return rc;
}
unsigned int get_req_count(void){
	return get_nr_of_req(task_pool_secret, N_LENGTH_OF_TASK_POOL);
}
