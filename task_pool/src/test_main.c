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
/**
 *  interfaces design
 *  @
 */
#include "lift_task_queue.h"
request_type_t request_pool[N_FLOORS] = { 0 };
request_type_t request_pool2[N_FLOORS] = { 0, 0, 0, 1, 2, 3, 4, 7, 6, 5, 4, 3,
		2, 3, 1, 3, 1, 3, 3 };
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
void test2(void) {
	puts("\ndownwards test!");
	print_request_pool(request_pool2, N_FLOORS);
	puts("udc");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_up | request_call_down | request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("u");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_up);
		printf("%d ", j);
	}
	puts("");
	puts("d");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_down);
		printf("%d ", j);
	}
	puts("");
	puts("c");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("ud");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_up | request_call_down);
		printf("%d ", j);
	}
	puts("");
	puts("uc");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_up | request_call_cmd);
		printf("%d ", j);
	}
	puts("");
	puts("dc");
	for (int i = 0; i < 19; i++) {
		int j = get_nearest_request_of_specified_downward(request_pool2, i,
				request_call_down | request_call_cmd);
		printf("%d ", j);
	}

}
unsigned int get_nr_of_req(const request_type_t * pool, unsigned int length) {
	int ret = 0;
	for (int i = 0; i < length; i++) {
		switch (pool[i]) {
		case 1:
		case 2:
		case 4:
			ret++;
			break;
		case 3:
		case 5:
		case 6:
			ret += 2;
			break;
		case 7:
			ret += 3;
			break;
		default:
			break;
		}
	}
	return ret;
}
void test3() {
	int cur_floor = 7;
	int dir = -1;
	int rc = 0;
	int gnr_up_count = 0;
	int gnr_dn_count = 0;
	puts("\n\n\n push and pop test");
	puts("origin pool: ");
	print_request_pool(request_pool2, N_FLOORS);
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
			get_nr_of_req(request_pool2, N_FLOORS), cur_floor, dir);
	while (get_nr_of_req(request_pool2, N_FLOORS) != 0) {
		switch (dir) {
		case 1:
			rc = get_nearest_request_of_specified_upward(request_pool2,
					cur_floor, request_call_up | request_call_cmd);
			if (rc == -1) {
				gnr_up_count %= 2;
				if (gnr_up_count) {
					/* odd */
				} else {
					/* even */
					rc = get_nearest_request_of_specified_downward(
							request_pool2, N_FLOORS-1,
							request_call_down | request_call_cmd);
					clr_request_type(request_pool2 + rc,
							request_call_down | request_call_cmd);
					cur_floor = rc;
				}
				gnr_up_count++;
				dir = -1;
			} else {
				clr_request_type(request_pool2 + rc,
						request_call_up | request_call_cmd);
				cur_floor = rc;
			}
			break;
		case -1:
			rc = get_nearest_request_of_specified_downward(request_pool2,
					cur_floor, request_call_down | request_call_cmd);
			if (rc == -1) {
				gnr_dn_count %= 2;
				if (gnr_dn_count) {
					/* odd */
				} else {
					/* even */
					rc = get_nearest_request_of_specified_upward(
							request_pool2, 0,
							request_call_up | request_call_cmd);
					clr_request_type(request_pool2 + rc,
							request_call_down | request_call_cmd);
					cur_floor = rc;
				}
				gnr_dn_count++;
				dir = 1;
			} else {
				clr_request_type(request_pool2 + rc,
						request_call_down | request_call_cmd);
				cur_floor = rc;
			}
			break;
		default:
			//dir = 0;
			break;
		}
		print_request_pool(request_pool2, N_FLOORS);
		printf("TASKS LEFT %d, current floor is %d, direction %d\n",
				get_nr_of_req(request_pool2, N_FLOORS), cur_floor, dir);
	}
	print_request_pool(request_pool2, N_FLOORS);
	printf("TASKS LEFT %d, current floor is %d, direction %d\n",
				get_nr_of_req(request_pool2, N_FLOORS), cur_floor, dir);
}
int main(void) {
	test1();
	test2();
	test3();
	return 0;
}
