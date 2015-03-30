/*
 * taskpool_policies_wrapper.c
 *
 *  Created on: Mar 30, 2015
 *      Author: yulongb
 */

#include "lift_task_queue.h"

static request_type_t task_pool_secret[N_FLOORS] = {0};

void push_request(int floor, request_type_t type){
	set_request_type(task_pool_secret+floor, type);
}

void pop_request(int floor, request_type_t type){
	clr_request_type(task_pool_secret+floor, type);
}

request_type_t get_request(int floor){
	return get_request_type(task_pool_secret+floor, request_call_up|request_call_down|request_call_cmd);
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
				N_FLOORS - 1, request_dn_n_cmd);
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
	return get_optimal_request_from_specified_on_search_direction(
			task_pool_secret, from, dir,
			ret_type);
}
unsigned int get_req_count(void){
	return get_nr_of_req(task_pool_secret, N_FLOORS);
}
