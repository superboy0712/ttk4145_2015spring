/*
 * taskpool_policies_wrapper.h
 *
 *  Created on: Mar 30, 2015
 *      Author: yulongb
 */

#ifndef SRC_TASKPOOL_POLICIES_WRAPPER_H_
#define SRC_TASKPOOL_POLICIES_WRAPPER_H_
/**
 *  @para from: input the floor to search from
 *  @para dir : input the search direction, output the request's direction
 *  @para ret_type: output the request's type
 *  @return : the optimal request got, -1 if the pool is empty.
 */
#include "lift_task_queue.h"
extern int get_optimal_req(int from, int *dir, request_type_t *ret_type);
extern void push_request(int floor, request_type_t type);
extern void pop_request(int floor, request_type_t type);
extern request_type_t get_request(int floor);
extern unsigned int get_req_count(void);

#endif /* SRC_TASKPOOL_POLICIES_WRAPPER_H_ */
