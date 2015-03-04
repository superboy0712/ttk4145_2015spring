#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "elevator_model_data_structure.h"
#include "elev.h"

void * keyboard_read_thread(){
//	FILE * keyin = fopen("./keyboard_in", "r" );
//	FILE * keyout = fopen("./keyboard_out", "w");
	while(1){
		int read;
		printf(  "\nInput floor: ");
		scanf( "%d", &read);
		printf( "\nDesired floor is: %d",read);
		set_desired_floor(read);
		//sleep(1);
	}
	return NULL;
}
#define  EMPTY_TASK 0XFFFF
static int task_queue[N_FLOORS] = {EMPTY_TASK,EMPTY_TASK,EMPTY_TASK,EMPTY_TASK};
volatile static int task_queue_top = 0;
int floor_cmp(a,b){
	return (a-b);
}
pthread_mutex_t task_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_task_cv = PTHREAD_COND_INITIALIZER;
event_t new_task_event = {
		.cv = &new_task_cv,
		.mutex = &task_queue_lock
};
int push_task(const int floor){
	if(floor<0 || floor>=N_FLOORS)
		return 0;
	pthread_mutex_lock(new_task_event.mutex);
	for (int i = 0; i < N_FLOORS; ++i) {
		if(floor == task_queue[i])
				//pthread_cond_signal(new_task_event.cv);
				pthread_mutex_unlock(new_task_event.mutex);
			return 0;/*rejected*/
	}
	if(task_queue_top<0 || task_queue_top>= N_FLOORS){
		puts("error in task_queue_top");
		return 0;
	}

		task_queue[task_queue_top] = floor;
		qsort(task_queue, N_FLOORS, sizeof(int), floor_cmp);
		if(task_queue_top != N_FLOORS-1)
			task_queue_top ++;
		pthread_cond_signal(new_task_event.cv);
	pthread_mutex_unlock(new_task_event.mutex);

	return 1;
}

int IsRequestAccepted(elev_button_type_t type, int floor){
	/* when motor is free then accepted, change desired floor */
	if(get_motor_moving_vector())
		return 0;
	return push_task(floor);
}
pthread_t keyboard_read_th, stop_button_controller_th, elevator_running_th;
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
				set_desired_floor(MOTOR_EM_STOP_CMD);
			}
		}
		pthread_mutex_unlock(input_event_ptr->mutex);
	}

	return NULL;
}
int fetch_task(void){
	pthread_mutex_lock(new_task_event.mutex);
	pthread_cond_wait(new_task_event.cv, new_task_event.mutex);
	if(task_queue_top>=0&&task_queue_top<N_FLOORS){
		if(task_queue_top!=0)
			task_queue_top--;
		int ret = task_queue[task_queue_top];
		task_queue[task_queue_top] = EMPTY_TASK;
		pthread_mutex_unlock(new_task_event.mutex);
		return ret;
	}
	pthread_mutex_unlock(new_task_event.mutex);
	return EMPTY_TASK;
}
void go_to_desired_floor(int floor){
	pthread_mutex_lock(floor_reached_event_ptr->mutex);
	set_desired_floor_unsafe(floor);
	pthread_cond_wait(floor_reached_event_ptr->cv, floor_reached_event_ptr->mutex);
	pthread_mutex_unlock(floor_reached_event_ptr->mutex);
}
void open_wait_close(void){
	light_status_t light = get_light_status();
	light.door_open_light = 1;
	set_light_status(light);
	sleep(2);
	input_status_t input = get_input_status();
	while(input.obst_button){
		input = get_input_status();
		sleep(1);
	}
	light = get_light_status();
	light.door_open_light = 0;
	set_light_status(light);
	sleep(2);
}

void *elevator_running_process(void * data){
	while(1){
		/**
		 *  fetch a new task
		 */
		int floor = EMPTY_TASK;
		while(floor == EMPTY_TASK){
			floor = fetch_task();
		}

		go_to_desired_floor(floor);

		open_wait_close();

	}
	return NULL;
}
int main(){

	elevator_model_init(NULL);
	int rc = pthread_create(&keyboard_read_th, NULL, keyboard_read_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&stop_button_controller_th, NULL, stop_button_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&elevator_running_th, NULL, elevator_running_process, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}

	input_status_t input_read;
	light_status_t light_to_write;
	while(1){

		pthread_mutex_lock(input_event_ptr->mutex);
			pthread_cond_wait(input_event_ptr->cv, input_event_ptr->mutex);

		input_read = get_input_status_unsafe();

 //TODO BELOW IS BUGGY CODE. MAKE FLOOR TO SET TO 3.
		light_to_write = get_light_status();
		for( int floor = 0; floor < N_FLOORS; floor++){
			for( int button_type = 0; button_type < 3; button_type++){
				if(light_to_write.floor_button_lights[floor][button_type] != -1){
					light_to_write.floor_button_lights[floor][button_type]
					= (input_read.Button_external[floor][button_type])? IsRequestAccepted(button_type, floor) : light_to_write.floor_button_lights[floor][button_type];

				}
			}
		}
		//light_status_t light = get_light_status();
		set_light_status(light_to_write);
		pthread_mutex_unlock(input_event_ptr->mutex);
	}
		
	return 0;
}
