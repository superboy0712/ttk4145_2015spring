/*****************************************************************
 * @file elevator_controller_layer.c
 * elevator_model
 *
 *  Created on		: Mar 31, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 * @brief main controlling logic built upon model layer
 *****************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "elevator_model_data_structure.h"
#include "../task_pool/lift_task_queue.h"
#include "../task_pool/taskpool_policies_wrapper.h"

request_type_t request_buffer[N_FLOORS] = { request_empty } ;/*!< to hold the parsed events, ready to send to the server*/
/**
 * transition table from button type to request type
 */
request_type_t to_request_type[3] = {
		[BUTTON_CALL_UP] = request_call_up,
		[BUTTON_CALL_DOWN] = request_call_down,
		[BUTTON_COMMAND] = request_call_cmd
};

/**
 * @brief wait for input events, parse button pushed events into request type
 * @param data
 */
void * request_button_events_parser_thread(void *data){
	data = NULL;
	input_status_t input_read;
	while(1){
		pthread_mutex_lock(input_event_ptr->mutex);
		pthread_cond_wait(input_event_ptr->cv, input_event_ptr->mutex);
		input_read = get_input_status_unsafe();
		for( int floor = 0; floor < N_FLOORS; floor++){
			for( int button_type = 0; button_type < 3; button_type++){
				if(input_read.request_button[floor][button_type] == 1){/* rising edge */
					//set_request_type(request_buffer+floor, to_request_type[button_type]);
					push_request(floor, to_request_type[button_type]);
				}
			}
		}
		pthread_mutex_unlock(input_event_ptr->mutex);
		/**
		 * TODO single elevator first, then include in the inquiry status, cost, send request.
		 */
	}
	return NULL;
}
/**
 * stop_button_controller_thread
 * @param data
 */
void * stop_button_controller_thread(void * data){
	while(1)
	{
		pthread_mutex_lock(input_event_ptr->mutex);
		pthread_cond_wait(input_event_ptr->cv, input_event_ptr->mutex);
		input_status_t read = get_input_status_unsafe();
		light_status_t write = get_light_status();
		if(read.stop_button){
			write.stop_light = !write.stop_light;
			set_light_status(write);
			if(write.stop_light){
				//set_desired_floor(MOTOR_EM_STOP_CMD);
			}
		}
		pthread_mutex_unlock(input_event_ptr->mutex);
	}

	return NULL;
}
void car_moving_handler(void);

typedef enum {
	IDLE,
	WORKING,
	REACHED
} elevator_status_t;
elevator_status_t elevator_status = IDLE;
void *elevator_running_process(void * data){
	while(1){
		switch (elevator_status) {
			case IDLE:
				sleep(1);
				if(get_req_count() > 0)
					elevator_status = WORKING;
			break;

			case REACHED:
				open_wait_close();
				elevator_status = WORKING;
			break;

			case WORKING:
				if(get_req_count() == 0){
					elevator_status = IDLE;
				}else{
					car_moving_handler();
					elevator_status = REACHED;
				}
			break;

			default:
				perror("elevator_running_process");
			break;
		}

	}
	return NULL;
}
#define milisec_block_wait_on_reached 150
void car_moving_handler(void){
	struct timespec time_to_wait;
	struct timeval now;
	int rc;
	int dest_floor;
	int cur_floor;
	int dir = get_motor_last_none_zero_motor_moving_vector();
	request_type_t type = request_empty;
	while(1){
		/* */
		cur_floor = get_light_status().floor_indicator_light;
		dest_floor = get_optimal_req(cur_floor, &dir, &type);
		if(dest_floor == -1){
			puts("fetch empty task in cage_move_handler, that shouln't happen!");
			return;
		}
		/* time_to_wait for reached_event */
		gettimeofday(&now,NULL);
		time_to_wait.tv_sec = now.tv_sec+0;
		time_to_wait.tv_nsec = (now.tv_usec+1000UL*milisec_block_wait_on_reached)*1000UL;
		/**********************************/
		pthread_mutex_lock(floor_reached_event_ptr->mutex);
		printf("go to desired floor %d\n\n",dest_floor);
		set_desired_floor_unsafe(dest_floor);
		rc = pthread_cond_timedwait(floor_reached_event_ptr->cv, floor_reached_event_ptr->mutex, &time_to_wait );
		if(rc == 0){
			pthread_mutex_unlock(floor_reached_event_ptr->mutex);
			printf("reached event caught by cage_move_handler.\n");

			if(dest_floor != get_light_status().floor_indicator_light)
				puts("not at the desired floor. sth wrong!");

			pop_request(dest_floor, type);
			return; /* here's the exit! need to unlock before exit */
		}
		else if (rc == ETIMEDOUT){
			puts("time out, continue, update task if any");
		}
		else{
			printf("cage_move_handler, unknown errono: %s", strerror(rc));
		}
		pthread_mutex_unlock(floor_reached_event_ptr->mutex);
	}
}


