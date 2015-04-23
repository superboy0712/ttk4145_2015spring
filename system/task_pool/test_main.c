/*
 * test_main.c
 *
 *  Created on: Mar 28, 2015
 *      Author: yulongb
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
/**
 *  interfaces design
 *  @
 */
#include "lift_task_queue.h"
#define N_LENGTH_OF_TASK_POOL 19
request_type_t request_pool[N_LENGTH_OF_TASK_POOL] = { 0 };
request_type_t request_pool2[N_LENGTH_OF_TASK_POOL] = { 0, 0, 0, 1, 2, 3, 4, 7, 6, 5, 4, 3,
		2, 3, 1, 3, 1, request_call_up, 0 };
request_type_t request_pool3[N_LENGTH_OF_TASK_POOL] = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, request_call_down, request_call_down };
request_type_t request_pool4[N_LENGTH_OF_TASK_POOL] = { request_call_down, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, request_call_up };

unsigned int get_nr_of_req(const request_type_t * pool, unsigned int length) {
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
/**
 * @return -1: empty, otherwise the optimal request
 * @ direction: input the searching direction, return the request's direction
 */
int get_optimal_request_from_specified_on_search_direction(
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
		assert(failed_count > 0);
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

extern const char *request_parse_string_table[8];

void get_optimal_test(int dir, const request_type_t * const pool,
		const char * testname) {
	printf("---------------------------%s, dir %d!!---------------------------",testname, dir);
	print_request_pool(pool, N_LENGTH_OF_TASK_POOL);
	request_type_t ret_type = request_empty;
	for (int i = 0; i < N_LENGTH_OF_TASK_POOL; i++) {
		int temp_dir = dir;
		int j = get_optimal_request_from_specified_on_search_direction(
				request_pool2, i, &temp_dir, &ret_type);
		printf("@%d, ret:%d %s dir:%d\n", i, j, request_parse_string_table[ret_type], temp_dir);
	}
}

typedef enum states {
	first_try = 1,
	second_try,
	third_try
} states_t;
int get_optimal_request_from_specified_on_search_direction_ver2(
		const request_type_t * const pool, int specified, int *direction,
		request_type_t * ret_type) {
	int ret = specified;
	states_t state = first_try;
	if (*direction == 0){
		*ret_type = request_up_dn_cmd;
		return ret;
	}
	while (1) {

		switch (state) {
			case first_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, specified,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, specified,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					state = second_try;
					*direction = -*direction;
				}else{
					return ret;
				}
			break;
			case second_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, 0,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, N_LENGTH_OF_TASK_POOL-1,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					state = third_try;
					*direction = -*direction;
				}else{
					return ret;
				}
			break;
			case third_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, specified,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, specified,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					*ret_type = request_empty;
				}
				return ret;
			break;
			default:
			break;
		}
	}
	return ret;
}

int get_optimal_request_from_specified_on_search_direction_ver3(
		const request_type_t * const pool, int specified, int *direction,
		request_type_t * ret_type) {
	int ret = specified;
	states_t state = first_try;
	if (*direction == 0){
		*ret_type = request_up_dn_cmd;
		return ret;
	}
	while (1) {

		switch (state) {
			case first_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, specified,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, specified,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					state = second_try;
					*direction = -*direction;
				}else{
					return ret;
				}
			break;
			case second_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, 0,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, N_LENGTH_OF_TASK_POOL-1,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					state = third_try;
					*direction = -*direction;
				}else{
					return ret;
				}
			break;
			case third_try:
				if (*direction > 0) {
					ret = get_nearest_request_of_specified_upward(pool, 0,
							request_up_n_cmd);
					*ret_type = request_up_n_cmd;
				} else {
					ret = get_nearest_request_of_specified_downward(pool, N_LENGTH_OF_TASK_POOL-1,
							request_dn_n_cmd);
					*ret_type = request_dn_n_cmd;
				}

				if(ret == -1){
					*ret_type = request_empty;
				}
				return ret;
			break;
			default:
			break;
		}
	}
	return ret;
}

void scheduling_test_general(int cur_floor, int dir,
		const request_type_t * const pool, const char * testname,
		int (*get_optimal_request)(
				const request_type_t * const pool, int specified, int *direction,
				request_type_t * ret_type)) {

	request_type_t request_pool_local[N_LENGTH_OF_TASK_POOL];
	memcpy(request_pool_local, pool, N_LENGTH_OF_TASK_POOL * sizeof(request_type_t));
	printf("\n\n\n%s\n", testname);
	puts("origin pool: ");
	print_request_pool(request_pool_local, N_LENGTH_OF_TASK_POOL);
	int temp_dir = dir;
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
			get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir);
	puts("---------------------------Tests Start!!---------------------------");
	int rc = cur_floor;
	request_type_t ret_type;
	int last_none_zero_actual_moving_dir = temp_dir;
	int mov_dir = temp_dir;
	while (get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL) != 0) {
		rc = get_optimal_request(request_pool_local,
				cur_floor, &temp_dir, &ret_type);
		if(rc == -1){
			break;
		}
		if(rc != cur_floor){
			last_none_zero_actual_moving_dir = (rc-cur_floor)/abs(rc-cur_floor);
			cur_floor += last_none_zero_actual_moving_dir;
			mov_dir = last_none_zero_actual_moving_dir;
		}else{
			mov_dir = 0;
		}

		if(cur_floor == rc){
			clr_request_type(request_pool_local + rc, ret_type);
		}
		/* when 7 up, push 0 up */
		if(cur_floor == 7 && mov_dir>0){
			set_request_type(request_pool_local+0, request_call_up);
		}
		/** printing **/
		//print_request_pool(request_pool_local, N_FLOORS);
		printf("TASKS LEFT %d, current floor is %d, output direction %d, moving dir %d\n",
				get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir, mov_dir);
		temp_dir = last_none_zero_actual_moving_dir;
	}
	puts("---------------------------Tests End!!---------------------------");
	puts("---------------------------Final Result!!---------------------------");
	print_request_pool(request_pool_local, N_LENGTH_OF_TASK_POOL);
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
			get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir);
}
void scheduling_test_general_dynamic(int cur_floor, int dir,
		const request_type_t * const pool, const char * testname,
		int (*get_optimal_request)(
				const request_type_t * const pool, int specified, int *direction,
				request_type_t * ret_type)) {

	request_type_t request_pool_local[N_LENGTH_OF_TASK_POOL];
	memcpy(request_pool_local, pool, N_LENGTH_OF_TASK_POOL * sizeof(request_type_t));
	printf("\n\n\n%s\n", testname);
	puts("origin pool: ");
	print_request_pool(request_pool_local, N_LENGTH_OF_TASK_POOL);
	int temp_dir = dir;
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
			get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir);
	puts("---------------------------Tests Start!!---------------------------");
	int rc = cur_floor;
	request_type_t ret_type;
	int last_none_zero_actual_moving_dir = temp_dir;
	int mov_dir = temp_dir;
	while (get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL) != 0) {
		rc = get_optimal_request(request_pool_local,
				cur_floor, &temp_dir, &ret_type);
		if(rc == -1){
			break;
		}
		if(rc != cur_floor){
			last_none_zero_actual_moving_dir = (rc-cur_floor)/abs(rc-cur_floor);
			cur_floor += last_none_zero_actual_moving_dir;
			mov_dir = last_none_zero_actual_moving_dir;
		}else{
			mov_dir = 0;
		}

		if(cur_floor == rc){
			clr_request_type(request_pool_local + rc, ret_type);
		}
		/* ask if continue or insert some more new req */
		char new_type;
		int new_floor;
		printf("'x' :continue; u/d/c number: new req. \n");
		scanf("%c %d\n", &new_type, &new_floor);
		switch (new_type) {
			case 'x':
			break;
			case 'u':
				set_request_type(request_pool_local+new_floor, request_call_up);
			break;
			case 'd':
				set_request_type(request_pool_local+new_floor, request_call_down);
			break;
			case 'c':
				set_request_type(request_pool_local+new_floor, request_call_cmd);
			break;
			default:
			break;
		}
		/** printing **/
		//print_request_pool(request_pool_local, N_FLOORS);
		printf("TASKS LEFT %d, current floor is %d, output direction %d, moving dir %d\n",
				get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir, mov_dir);
		temp_dir = last_none_zero_actual_moving_dir;
	}
	puts("---------------------------Tests End!!---------------------------");
	puts("---------------------------Final Result!!---------------------------");
	print_request_pool(request_pool_local, N_LENGTH_OF_TASK_POOL);
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
			get_nr_of_req(request_pool_local, N_LENGTH_OF_TASK_POOL), cur_floor, temp_dir);
}
int main(void) {
	get_nearest_length_init(N_LENGTH_OF_TASK_POOL);
//	scheduling_test(N_LENGTH_OF_TASK_POOL - 1, -1, request_pool3, "top down");
//	scheduling_test(0, 1, request_pool3, "bottom up");
//	scheduling_test(N_LENGTH_OF_TASK_POOL - 1, 1, request_pool3, "top up");
//	scheduling_test_general(8, 1, request_pool3, "8 up full from v1", get_optimal_request_from_specified_on_search_direction);
//	scheduling_test_general(8, 1, request_pool3, "8 up full from v2", get_optimal_request_from_specified_on_search_direction_ver2);
	scheduling_test_general_dynamic(8, 1, request_pool3, "8 up full from v3", get_optimal_request_from_specified_on_search_direction_ver3);
//	scheduling_test_general(N_LENGTH_OF_TASK_POOL - 1, -1, request_pool3, "top down",get_optimal_request_from_specified_on_search_direction_ver3);
//	scheduling_test_general(0, 1, request_pool3, "bottom up", get_optimal_request_from_specified_on_search_direction_ver3);
	return 0;
}
