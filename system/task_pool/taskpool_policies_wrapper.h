/*
 * taskpool_policies_wrapper.h
 *
 *  Created on: Mar 30, 2015
 *      Author: yulongb
 */
/**
 * @file taskpool_policies_wrapper.h
 * @brief wrapper functions of apis from lift_task_queue.c
 * 		  for convience
 */
#ifndef SRC_TASKPOOL_POLICIES_WRAPPER_H_
#define SRC_TASKPOOL_POLICIES_WRAPPER_H_
#include "lift_task_queue.h"
/**
 *  @param from  input the floor to search from
 *  @param dir  input the search direction, output the request's direction
 *  @param ret_type  output the request's type
 *  @return  the optimal request got, -1 if the pool is empty.
 */
extern int get_optimal_req(int from, int *dir, request_type_t *ret_type);
extern void push_request(int floor, request_type_t type);
extern void pop_request(int floor, request_type_t type);
extern request_type_t get_request(int floor);
extern unsigned int get_req_count(void);
extern void default_task_pool_init( unsigned int length);
extern void default_task_pool_destroy(void);
#endif /* SRC_TASKPOOL_POLICIES_WRAPPER_H_ */
