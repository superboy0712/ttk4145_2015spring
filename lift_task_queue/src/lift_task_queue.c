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
#define N_FLOORS 19

typedef enum {
	request_call_up = 1, request_call_down = 2, request_call_cmd = 4
} request_type_t;
//typedef struct task_st {
//	int floor;
//	request_type_t request_type;
//} task_t;
request_type_t request_pool[N_FLOORS] = { 0 };
request_type_t request_pool2[N_FLOORS] = { 0, 0, 0, 1, 2, 3, 4, 7, 6, 5, 4, 3,
		2, 3, 1, 3, 1, 3, 3 };
void print_request_pool(request_type_t * const pool, unsigned int length) {
	assert(length > 0);
	for (int i = 0; i < length; i++) {
		printf("%d ", pool[i]);
	}
	puts("");
}
int get_request_type(request_type_t *ptr, request_type_t type) {
	return (*ptr) & type;
}
void set_request_type(request_type_t *ptr, request_type_t type) {
	*ptr = (*ptr) | type;
}
void clr_request_type(request_type_t *ptr, request_type_t type) {
	*ptr = (*ptr) & (~type);
}
int get_nearest_request_of_specified_upward(request_type_t * const pool,
		int specified, request_type_t type) {
	for (int i = 0; i < N_FLOORS; i++) {
		if ((get_request_type(pool + i, type) != 0) && (i >= specified)) {
			return i;
		}
	}
	puts("get_nearest_request_of_specified_upward: no valid return");
	return -1;
}
int test1(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	print_request_pool(request_pool, N_FLOORS);
	set_request_type(request_pool + 5, request_call_cmd);
	set_request_type(request_pool + 13, request_call_cmd);
	set_request_type(request_pool + 0, request_call_cmd);
	print_request_pool(request_pool, N_FLOORS);
	clr_request_type(request_pool + 5, request_call_cmd);
	clr_request_type(request_pool + 13, request_call_down);
	set_request_type(request_pool + 0, request_call_up);
	print_request_pool(request_pool, N_FLOORS);
	for (int i = 0; i < 19; i++) {
		set_request_type(request_pool + i,
				request_call_up | request_call_down | request_call_cmd);
		printf("%d ",
				get_request_type(request_pool + i,
						request_call_up | request_call_cmd));
	}
	puts("");
	puts("udc");
	print_request_pool(request_pool, N_FLOORS);
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_up | request_call_down | request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("u");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_up);
		printf("%d ", j);
	}
	puts("");
	puts("d");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_down);
		printf("%d ", j);
	}
	puts("");
	puts("c");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("ud");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_up | request_call_down);
		printf("%d ", j);
	}
	puts("");
	puts("uc");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_up | request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("dc");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_upward(request_pool2, i,
				request_call_down | request_call_cmd);
		printf("%d ", j);
	}
	return EXIT_SUCCESS;
}

int main(void) {
	test1();
	return 0;
}

