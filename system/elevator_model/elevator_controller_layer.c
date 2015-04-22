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
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "elevator_model_data_structure.h"
#include "../task_pool/lift_task_queue.h"
#include "../task_pool/taskpool_policies_wrapper.h"
#include "server/new.h"
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
my_interface_t in_main_interface_DUMMY;
my_interface_t* in_main_interface = &in_main_interface_DUMMY;
char status_buffer[SEND_SIZE];
void * request_button_events_parser_thread(void *data){
	data = NULL;
	input_status_t input_read;
	unsigned int dest_floor = 0xff;
	request_type_t dest_type = request_empty;
	while(1){
		BEGIN:
		pthread_mutex_lock(input_event_ptr->mutex);
		pthread_cond_wait(input_event_ptr->cv, input_event_ptr->mutex);
		input_read = get_input_status_unsafe();
		for( int floor = 0; floor < N_FLOORS; floor++){
			for( int button_type = 0; button_type < 3; button_type++){
				if(input_read.request_button[floor][button_type] == 1){/* rising edge */

					if(button_type == BUTTON_COMMAND){
						/* cmd pushed to local directly */
						push_request(floor, to_request_type[button_type]);
						pthread_mutex_unlock(input_event_ptr->mutex);
						goto BEGIN;
					}else{
						/* pending for decision */
						//set_request_type(request_buffer+floor, to_request_type[button_type]);
					}
					dest_floor = floor;
					dest_type = to_request_type[button_type];
					break;
				}
			}
		}

		pthread_mutex_unlock(input_event_ptr->mutex);
		if(dest_floor == 0xff) continue;
		/**
		 * TODO single elevator first, then include in the inquiry status, cost, send request.
		 */
		pthread_mutex_lock(&in_main_interface->interface_mutex);
//		request_type_t type = request_empty;
//		volatile int dest_floor = get_last_stable_floor();
//		for( int floor = N_FLOORS-1; floor > 0; floor--){
//			type = get_request_type(request_buffer+floor, request_up_dn_cmd);
//			if(type!=request_empty){
//				dest_floor = floor;
//				break;
//			}
//		}

		in_main_interface->order_floor = dest_floor;
		strncpy(in_main_interface->interface_status_buffer, status_buffer, 13);
		in_main_interface->order_direction = (dest_type & request_call_up)? 'U':'D';
		in_main_interface->order_floor_flag = TRUE;
		pthread_mutex_unlock(&in_main_interface->interface_mutex);


		usleep(10 * MS);

		pthread_mutex_lock(&in_main_interface->interface_mutex);
	    int temp_order_floor = 0;
		while (1) {

			if (in_main_interface->order_floor_flag == FALSE) {
				printf("Main:Order Flag Cleared by comm module....\n");
				temp_order_floor = in_main_interface->order_floor;
				pthread_mutex_unlock(&in_main_interface->interface_mutex);
				break;
			}

			else {
				pthread_mutex_unlock(&in_main_interface->interface_mutex);
				usleep(10 * MS);
			}
		}

		printf("Final order is %d -1 means someone else is doing\n",
				temp_order_floor);
		if(temp_order_floor!=-1){
			/* I serve */
			push_request(dest_floor, dest_type);
		}else{
			/* others serve */

		}

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
/**
 * following are elevator_running_process and related wrapper
 */
void car_moving_handler(void);
void open_wait_close(void);
typedef enum {
	IDLE,
	WORKING,
	REACHED
} elevator_status_t;
elevator_status_t elevator_status = IDLE;
void *elevator_running_controller_thread(void * data){
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
#define milisec_block_wait_on_reached 200
void car_moving_handler(void){
	struct timespec time_to_wait;
	struct timespec now;
	int rc;
	int dest_floor;
	int cur_floor;
	float cur_pos = get_current_floor_position();
	int dir = get_motor_last_none_zero_motor_moving_vector();//get_motor_last_none_zero_motor_moving_vector();
	int failed_count = 0;
	int temp_dir;
	request_type_t type = request_empty;
	while(1){
		/* */
	BEGIN:
		cur_pos = get_current_floor_position();
		cur_floor = get_last_stable_floor();
		if(dir == 0) {
			puts("dir equals zero!");
			dir = 1;
		}

		if(cur_pos - cur_floor > 0.1){
			cur_floor++;
		} else if(cur_pos - cur_floor < -0.1){
			cur_floor--;
		}
		temp_dir = dir;
		dest_floor = get_optimal_req(cur_floor, &dir, &type);
		if(dest_floor == -1){
			if(failed_count > 4){
				puts("fetch empty task 5 times in cage_move_handler, that shouln't happen!");
				return;
			}
			else if(2<= failed_count && failed_count<=4){
					if(temp_dir == dir){
						dir = -dir;
					}
					puts("fetch empty task in cage_move_handler, try inverse direction!");
			}
			if(cur_floor == N_FLOORS-1) dir = -1;
			if(cur_floor == 0) dir = 1;
			failed_count++;

			goto BEGIN;
		}

		pthread_mutex_lock(floor_reached_event_ptr->mutex);
		printf("go to desired floor %d\n\n",dest_floor);
		set_desired_floor_unsafe(dest_floor);
				/* time_to_wait for reached_event */
//				clock_gettime(CLOCK_REALTIME, &now);
//				printf("time sec:%ld, nsec:%ld\n",now.tv_sec, now.tv_nsec);
//
//				//gettimeofday(&now,NULL);
//				time_t delta_sec = (now.tv_nsec+1000000UL*milisec_block_wait_on_reached)/1000000000UL;
//				unsigned long delta_nsec = (now.tv_nsec+1000000UL*milisec_block_wait_on_reached)%1000000000UL;
//				time_to_wait.tv_sec = now.tv_sec+delta_sec;
//				time_to_wait.tv_nsec = now.tv_nsec+delta_nsec;
				clock_gettime(CLOCK_REALTIME, &now);
				time_to_wait.tv_sec = now.tv_sec+1;
				time_to_wait.tv_nsec = now.tv_nsec;
				/**********************************/
		rc = pthread_cond_timedwait(floor_reached_event_ptr->cv, floor_reached_event_ptr->mutex, &time_to_wait );
		if(rc == 0){
			pthread_mutex_unlock(floor_reached_event_ptr->mutex);
			printf("reached event caught by cage_move_handler.\n");
			usleep(100000);
			volatile int temp = elev_get_floor_sensor_signal();
			if(dest_floor != temp){
				puts("not at the desired floor. sth wrong!");
			}
			pop_request(dest_floor, type);

			return; /* here's the exit! need to unlock before exit */
		}
		else if (rc == ETIMEDOUT){
			pthread_mutex_unlock(floor_reached_event_ptr->mutex);
			puts("time out, continue, update task if any");
		}
		else{
			pthread_mutex_unlock(floor_reached_event_ptr->mutex);
			printf("cage_move_handler, unknown errono: %s\n\n", strerror(rc));
			printf("to wait sec:%ld, nsec:%ld\n",time_to_wait.tv_sec, time_to_wait.tv_nsec);
			usleep(milisec_block_wait_on_reached*1000UL);
		}

	}
}
void open_wait_close(void){
	light_status_t light = get_light_status();
	light.door_open_light = 1;
	set_light_status(light);
	sleep(2);
	input_status_t input = get_input_status();
	while(input.obst_button){
		sleep(1);
		input = get_input_status();
	}
	light = get_light_status();
	light.door_open_light = 0;
	set_light_status(light);
	sleep(2);
}
/**
 * request button light controller, controlling light on/off
 * according to the requests registered
 */
void *request_button_light_controller_thread(void *data){
	data = NULL;
	light_status_t light_to_write;
	while(1){
		usleep(75000);
		light_to_write = get_light_status();
		for(int floor = 0; floor < N_FLOORS; floor++){
			for (int button_type = 0; button_type < 3; ++button_type) {
				if(light_to_write.floor_button_lights[floor][button_type]!=-1){
					light_to_write.floor_button_lights[floor][button_type] = (get_request(floor, to_request_type[button_type])!=0);
				}
			}
		}
		set_light_status(light_to_write);
		//default_task_pool_print();
		/**
		 *  UPDATE STATUS TO BUFFER
		 */
		pthread_mutex_lock(&in_main_interface->interface_mutex);
			char DIR = (get_motor_last_none_zero_motor_moving_vector()>0)? 'U':'D';
			sprintf(status_buffer,"MY_STATUS_%d_%c", get_last_stable_floor(), DIR);
			strncpy(in_main_interface->interface_status_buffer, status_buffer, 13);
			if(in_main_interface->received_floor_flag==TRUE){
				request_type_t type = (in_main_interface->received_direction == 'U')? request_call_up : request_call_down;
				push_request(in_main_interface->received_floor, type);
			}
			in_main_interface->received_floor_flag=FALSE;
		pthread_mutex_unlock(&in_main_interface->interface_mutex);
	}
	return NULL;
}
pthread_t request_button_light_controller_th, elevator_running_controller_th,
		stop_button_controller_th, request_button_events_parser_th, init_thread;
int main(int argc, char **argv) {
	elevator_model_init(NULL);
	default_task_pool_init(N_FLOORS);
	int rc = pthread_create(&request_button_light_controller_th, NULL, request_button_light_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&elevator_running_controller_th, NULL, elevator_running_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&stop_button_controller_th, NULL, stop_button_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&request_button_events_parser_th, NULL, request_button_events_parser_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&init_thread, NULL, initialzie_function, in_main_interface);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	while(1){
		sleep(10);
		puts("i am alive. boring");
	}
	return 0;
}

