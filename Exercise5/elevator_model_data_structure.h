/*****************************************************************
 * elevator_model_data_structure.h
 * Exercise5
 *
 *  Created on		: Feb 28, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  Description		:
 *****************************************************************/

#ifndef ELEVATOR_MODEL_DATA_STRUCTURE_H_
#define ELEVATOR_MODEL_DATA_STRUCTURE_H_
#include <stdio.h>
#include <pthread.h>
#include "elev.h"
#define MOTOR_EM_STOP_CMD 0xffff
typedef struct event_st {
	pthread_cond_t  *cv;
	pthread_mutex_t *mutex;
} event_t;
extern event_t * const input_event_ptr; /* Wrapper of Synchronization primitives */
extern event_t * const floor_reached_event_ptr;/* Wrapper of floor event */
typedef struct input_status_st{
	// button table to represent the current button status
	//each floor has 3 types: UP DOWN CMD
	// -1 stands for not existing
	int Button_external[N_FLOORS][3];
	int floor_sensor;
	int stop_button;
	int obst_button;
} input_status_t;
typedef struct light_status_st{
	// button table to represent the current button status
	//each floor has 3 types: UP DOWN CMD
	// -1 stands for not existing
	int floor_button_lights[N_FLOORS][3];
	int floor_indicator_light;/* 0 - 3 */
	int stop_light;
	int door_open_light;
} light_status_t;

extern input_status_t get_input_status(void);
extern input_status_t get_input_status_unsafe(void);/* not thread safe */
extern light_status_t get_light_status(void);
extern void set_light_status(const light_status_t status);
extern int get_desired_floor(void);
extern int get_desired_floor_unsafe(void);
/* \para floor: 0 ~ NFLOORS-1, normal desired floor; MOTOR_EM_STOP_CMD, emergency stop CMD */
extern void set_desired_floor(const int floor);
extern void set_desired_floor_unsafe(const int floor);
extern int get_motor_moving_vector(void);
/* \para log for redirecting log */
extern int elevator_model_init(FILE* log);
#endif /* ELEVATOR_MODEL_DATA_STRUCTURE_H_ */
