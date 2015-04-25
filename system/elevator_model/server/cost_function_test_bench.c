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
#include <math.h>
#include <assert.h>
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
#define N_FLOORS 4
int cost_function_v2(struct cost_param_t cost_values, int order_floor,
		char temp_order_dircetion) {
	int the_opt_idx = 0;
	float temp_order_floor = (float)order_floor;
	double cost_array[N_CLIENT];
	int dir = (temp_order_dircetion == 'U')? 1 : -1;
	/* calculate cost of each connected node */
	for (int i = 0; i < cost_values.max_connected_nodes; i++) {
		/* basis cost */
		cost_array[i] = fabs(temp_order_floor - cost_values.floor_position[i]);

		/* if the cage is moving and moving away from order floor, then add extra cost */
		if(cost_values.moving_vector[i] != 0){
			if(cost_values.moving_vector[i]*(temp_order_floor - cost_values.floor_position[i]) <= 0){
				/* order vector and current moving vector are in opposite direction */

				if(cost_values.moving_vector[i]*dir < 0){
					/* calling in different direction */
					if(dir > 0){
						cost_array[i] = cost_values.floor_position[i] + temp_order_floor;
					}else{
						cost_array[i] = 2*(N_FLOORS - 1) - cost_values.floor_position[i] - temp_order_floor;
					}
				}else{
					cost_array[i] = 2*(N_FLOORS - 1) - fabs(temp_order_floor - cost_values.floor_position[i]);
				}
			}
		}

		/* if exception,  add a large cost */
		if(cost_values.stop[i] == 1 || cost_values.obstrukt[i] == 1) {
			cost_array[i] += 1000;/* give the exceptional ones a  large difference */
		}

	}

    /* find the first minumum one, and return its index in the connected array */
	float minimum = cost_array[0];

	for(int i = 0; i < cost_values.max_connected_nodes; i++) {
		if (cost_array[i] < minimum) {
			minimum = cost_array[i];
			the_opt_idx = i;
		}
	}
	/* print */
	printf("The cost_array: ");
	for(int i = 0; i < cost_values.max_connected_nodes; i++){
		printf(" {[%d]%4.1lf},", cost_values.index[i], cost_array[i]);
	}
	puts("");

	printf("Order is %d_%c, the minimum cost is {%4.1lf from index [%d]}\n",
			order_floor, temp_order_dircetion, cost_array[the_opt_idx], cost_values.index[the_opt_idx]);
	if(the_opt_idx == 0)/*< myself */
		return 0;

	return cost_values.index[the_opt_idx];
}
struct cost_param_t t1 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 0 },
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.obstrukt = {0},
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 }
};
struct cost_param_t t2 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 1, 1, 1, 1, 0, 0, 0, 0 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 }
};

struct cost_param_t t3_mv_up = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 0 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {0, 0, 0, 0, 1, 1, 1, 1}
};

struct cost_param_t t3_mv_dn = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 0 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {0, 0, 0, 0, -1, -1, -1, -1}
};

struct cost_param_t t4_mv_up = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = { 1, 1, 1, 1, 0, 0, 0, 0 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {0, 0, 0, 0, 1, 1, 1, 1}
};

struct cost_param_t t4_mv_dn = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.obstrukt = { 1, 1, 1, 1, 0, 0, 0, 0 },
		.stop = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {0, 0, 0, 0, -1, -1, -1, -1}
};

struct cost_param_t t5_mv_up = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.stop = {0 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {1, 1, 1, 1, 1, 1, 1, 1}
};

struct cost_param_t t5_mv_dn = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D'},
		.obstrukt = { 0 },
		.stop = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 8,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17 },
		.moving_vector = {-1, -1, -1, -1, -1, -1, -1, -1}
};


struct cost_param_t t4 = {
		.floor = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 },
		.direction = { 'U','U','U','U','D','D','D','D','U','U','U','U','D','D','D','D'},
		.stop = { 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 },
		.obstrukt = {0},
		.floor_position = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 },
		.max_connected_nodes = 16,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 }
};
struct cost_param_t t5 = {
		.floor = { 3, 2},
		.direction = { 'U','U'},
		.stop = { 1, 1 },
		.obstrukt = {0},
		.max_connected_nodes = 2,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11 }
};

struct cost_param_t t6 = {
		.floor = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		.direction = { 'U','U','U','U','D','D','D','D','U','U','U','U','D','D','D','D'},
		.stop = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
		.obstrukt = {0},
		.floor_position = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		.max_connected_nodes = 16,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 },
		.moving_vector = {0, 1, 2, 3, 0, 1, 2, 3, 0, -1, -2, -3, 0, -1, -2, -3}
};
/**
 * moving away test
 */
struct cost_param_t t7 = {
		.floor = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		.direction = { 'U','U','U','U','D','D','D','D','U','U','U','U','D','D','D','D'},
		.stop = { 0 },
		.obstrukt = {0},
		.floor_position = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		.max_connected_nodes = 16,/**??? whether it is index or number of nodes???*/
		.index = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 },
		.moving_vector = {0, 1, 2, 3, 0, 1, 2, 3, 0, -1, -2, -3, 0, -1, -2, -3}
};
void main(void){
	char dir = 'U';
	int idx;
	for (int i = 0; i < 4; ++i) {
		idx = cost_function_v2(t5_mv_up, i, dir );
		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
	}
	puts("");
	dir = 'D';
	for (int i = 0; i < 4; ++i) {
		idx = cost_function_v2(t5_mv_dn, i, dir );
		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
	}
//	dir = 'D';
//	for (int i = 0; i < 4; ++i) {
//		idx = cost_function(t4, i, dir );
//		printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//	}
//	int i = 2;
//	dir = 'U';
//	idx = cost_function_v2(t7, i, dir );
//	printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//
//	 i = 2;
//	dir = 'D';
//	idx = cost_function_v2(t7, i, dir );
//	printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//
//	 i = 1;
//	dir = 'D';
//	idx = cost_function_v2(t7, i, dir );
//	printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//
//	 i = 0;
//	dir = 'D';
//	idx = cost_function_v2(t7, i, dir );
//	printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
//	 i = 0;
//	dir = 'U';
//	idx = cost_function_v2(t7, i, dir );
//	printf("floor %d, dir %c, optimal idx %d\n", i, dir, idx);
}
