/*
 ============================================================================
 Name        : lift_task_queue.c
 Author      : Yulong Bai
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
/**
 *  interfaces design
 *  @
 */
#include "lift_task_queue.h"
const char *request_parse_string_table[8] = {
		[0] = "req_empty",
		"req_up",
		"req_down",
		"req_up & down",
		"req_cmd",
		"req_up & cmd",
		"req_down & cmd",
		"req_down & up & cmd"
};
static unsigned int N_LENGTH_OF_TASK_POOL = 4; /*default*/
void print_request_pool(const request_type_t * const pool, unsigned int length) {
	assert(length > 0);
	for (int i = 0; i < length; i++) {
		printf("%d: %s\n", i, request_parse_string_table[pool[i]]);
	}
	puts("");
}
inline int get_request_type(const request_type_t *ptr, request_type_t type) {
	return (*ptr) & type;
}
inline void set_request_type(request_type_t *ptr, request_type_t type) {
	*ptr = (*ptr) | type;
}
inline void clr_request_type(request_type_t *ptr, request_type_t type) {
	*ptr = (*ptr) & (~type);
}
int get_nearest_request_of_specified_upward(const request_type_t * const pool,
		int specified, request_type_t type) {
	for (int i = 0; i < N_LENGTH_OF_TASK_POOL; i++) {
		if ((get_request_type(pool + i, type) != 0) && (i >= specified)) {
			return i;
		}
	}
	puts("get_nearest_request_of_specified_upward: no valid return");
	return -1;
}
int get_nearest_request_of_specified_downward(const request_type_t * const pool,
		int specified, request_type_t type) {
	for (int i = N_LENGTH_OF_TASK_POOL-1; i >=0; i--) {
		if ((get_request_type(pool + i, type) != 0) && (i <= specified)) {
			return i;
		}
	}
	puts("get_nearest_request_of_specified_downward: no valid return");
	return -1;
}
void get_nearest_length_init(unsigned int length){
	N_LENGTH_OF_TASK_POOL = length;
}
