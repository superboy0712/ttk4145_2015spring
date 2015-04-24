/*
 * cost_function_test_bench.c
 *
 *  Created on: Apr 24, 2015
 *      Author: yulongb
 */
//#include "new.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N_CLIENT 20

struct cost_param_t {

	int floor[N_CLIENT]; /**< Floors status of clients + myself*/

	char direction[N_CLIENT]; /**< Direction status of clients + myself*/

	int index[N_CLIENT]; /**< No. of currently connected clients*/

	char temp_floor[2]; /**< Temporay variable used for conversion from string to int*/

	int stop[N_CLIENT]; /**< STOP button status*/

	int obstrukt[N_CLIENT]; /**< OBSTRUKT button status*/

	float floor_position[N_CLIENT]; /**< In-between floor position*/

	int moving_vector[N_CLIENT]; /**< Direction Vector of motor*/

	int timeout_status[N_CLIENT]; /**< Timeout status of client*/

	int max_connected_nodes; /**< Maximum value of index*/

};
int cost_function(struct cost_param_t cost_values, int temp_order_floor,
		char temp_order_dircetion) {

	int i = 0;
	int minimum;
	int temp_diff[N_CLIENT];
	int minimum_one_s_array_indexes[N_CLIENT];/* for saving the ones that are equally minimum */
	int mosai_index = 0;
	int the_opt_idx = 0;
	memset(minimum_one_s_array_indexes, 0xfff, N_CLIENT * sizeof(int));
	for (i = 0; i < cost_values.max_connected_nodes; i++) {
		/**
		 *  exclude those exceptional ones from candidates
		 */
		if (cost_values.stop[i] == 1 || cost_values.obstrukt[i] == 1) {
			temp_diff[i] = abs(temp_order_floor - cost_values.floor[i]) + 1000;/* give the exceptional ones a  large difference */
		}
		else {
			temp_diff[i] = abs(temp_order_floor - cost_values.floor[i]);
		}

	}
	/* get the minimum difference and its idx */
	minimum = temp_diff[0];

	for (i = 0; i < cost_values.max_connected_nodes; i++) {
		if (temp_diff[i] <= minimum) {
			minimum = temp_diff[i];
			the_opt_idx = i;
		}
	}

	/* there may be several equally minimum difference candidates, save their idxes
	 * mosai_index is the number of the candidates */
	for (int i = 0; i < cost_values.max_connected_nodes; ++i) {
		if (temp_diff[i] == minimum) {
			minimum_one_s_array_indexes[mosai_index] = i;
			mosai_index++;
		}
	}
	printf("mosai_index %d. ", mosai_index);
	for (int i = 0; i < mosai_index; ++i) {
		printf("mosai_index[%d] %d. ", i, minimum_one_s_array_indexes[i]);
	}
	puts("");
	/**
	 *  if the distances are the same, get the first of matched direction from these candidates
	 */
	for (int i = 0; i < mosai_index; ++i) {
		int temp = minimum_one_s_array_indexes[mosai_index];
		if (temp_order_dircetion == cost_values.direction[temp]) {
			the_opt_idx = temp;
			/* the first matched direction wins */
			break;
		}

	}
	if (the_opt_idx == 0) {
		/**
		 *  means the optimal decision is myself
		 */
		return 0;
	}

	printf("From Cost Function, minimum distance is %d and index is %d, direction is %c \n",
			minimum, the_opt_idx, cost_values.direction[the_opt_idx]);
	//printf(" last dir %c \n", cost_values.direction[the_opt_idx]);
	return cost_values.index[the_opt_idx];
}
struct cost_param_t t1 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 0 },
		.obstrukt = {0},
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 }
};
struct cost_param_t t2 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 1, 1, 1, 1, 0, 0, 0, 0 },
		.obstrukt = {0},
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 }
};

struct cost_param_t t3 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D','U','U','U','U','D','D','D','D'},
		.stop = { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
		.obstrukt = {0},
		.max_connected_nodes = 16,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 }
};

struct cost_param_t t4 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D','U','U','U','U','D','D','D','D'},
		.stop = { 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 },
		.obstrukt = {0},
		.max_connected_nodes = 16,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 }
};
void main(void){
	char dir = 'U';
	int idx;
//	for (int i = 0; i < 4; ++i) {
//		idx = cost_function(t1, i, dir );
//		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//	}
	dir = 'U';
	for (int i = 0; i < 4; ++i) {
		idx = cost_function(t4, i, dir );
		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
	}
//	dir = 'D';
//	for (int i = 0; i < 4; ++i) {
//		idx = cost_function(t4, i, dir );
//		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//	}

}
