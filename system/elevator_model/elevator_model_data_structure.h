/*****************************************************************
 * @file elevator_model_data_structure.h
 * elevator_model
 *
 *  Created on		: Feb 28, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  @brief	elevator model interfaces and related data structures built upon drivers
 *****************************************************************/

#ifndef ELEVATOR_MODEL_DATA_STRUCTURE_H_
#define ELEVATOR_MODEL_DATA_STRUCTURE_H_
#include <stdio.h>
#include <pthread.h>
#include "../drivers/elev.h"
typedef struct event_st {
	pthread_cond_t  *cv;
	pthread_mutex_t *mutex;
} event_t;
extern event_t * const input_event_ptr; /** Wrapper of Synchronization primitives */
extern event_t * const floor_reached_event_ptr;/** Wrapper of floor event */
typedef struct input_status_st{
	/// button table to represent the current button status
	/// each floor has 3 types: UP DOWN CMD
	/// -1 stands for not existing
	int request_button[N_FLOORS][3];
	int floor_sensor;
	int stop_button;
	int obst_button;
} input_status_t;
typedef struct light_status_st{
	/// button table to represent the current button status
	/// each floor has 3 types: UP DOWN CMD
	/// -1 stands for not existing
	int floor_button_lights[N_FLOORS][3];
	int floor_indicator_light;/** 0 - 3 */
	int stop_light;
	int door_open_light;
} light_status_t;

extern input_status_t get_input_status(void);
extern input_status_t get_input_status_unsafe(void);/**< not thread safe */

extern light_status_t get_light_status(void);
extern void set_light_status(const light_status_t status);

extern int get_desired_floor(void);
extern int get_desired_floor_unsafe(void);/**< not thread safe */
/**
 *  @param floor 0 ~ NFLOORS-1: normal desired floor; MOTOR_EM_STOP_CMD: emergency stop CMD */
extern void set_desired_floor(const int floor);
extern void set_desired_floor_unsafe(const int floor);/*!< not thread safe */
/** @brief indicate real-time motor running status and distance left */
extern int get_motor_moving_vector(void);
extern int get_motor_last_none_zero_motor_moving_vector(void);/*!< @brief indicate last none zero direction */
extern float get_current_floor_position(void);/*!< @brief float representation of position, x.5 means somewhere in the middle */
extern int get_last_stable_floor(void); /*!< @brief last stable floor number, not somewhere in between */
/**
 *  @param log for redirecting log */
extern int elevator_model_init(FILE* log);

#endif /* ELEVATOR_MODEL_DATA_STRUCTURE_H_ */
