/*
 * lift_task_queue.h
 *
 *  Created on: Mar 28, 2015
 *      Author: yulongb
 */

#ifndef LIFT_TASK_QUEUE_H_
#define LIFT_TASK_QUEUE_H_

typedef enum {
	request_empty = 0,
	request_call_up = 1, request_call_down = 2, request_call_cmd = 4,
	request_up_n_dn = 3, request_up_n_cmd = 5, request_dn_n_cmd = 6,
	request_up_dn_cmd = 7
} request_type_t;

extern void print_request_pool(const request_type_t * const pool, unsigned int length);
extern int get_request_type(const request_type_t *ptr, request_type_t type);
extern void set_request_type(request_type_t *ptr, request_type_t type);
extern void clr_request_type(request_type_t *ptr, request_type_t type);
extern int get_nearest_request_of_specified_upward(const request_type_t * const pool,
													int specified, request_type_t type);
extern int get_nearest_request_of_specified_downward(const request_type_t * const pool,
													int specified, request_type_t type);
extern void task_pool_init(unsigned int length);/* for test bench only*/
#endif /* LIFT_TASK_QUEUE_H_ */
