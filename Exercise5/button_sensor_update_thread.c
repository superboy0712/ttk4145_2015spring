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
int IsRequestAccepted(elev_button_type_t type, int floor){
	/* when motor is free then accepted, change desired floor */
	if(get_motor_moving_vector())
		return 0;
	set_desired_floor(floor);
	return 1;
}
pthread_t keyboard_read_th;

int main(){
	elevator_model_init(NULL);
	int rc = pthread_create(&keyboard_read_th, NULL, keyboard_read_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	input_status_t input_read = get_input_status();
    /* temp for detecting rising edge events */
	int last_Button_external[N_FLOORS][3] =
		{
			/*0*/	{0,	-1,	0},
			/*1*/	{0,	0,	0},
			/*2*/	{0,	0,	0},
			/*3*/	{-1,	0,	0}
		};
	//int last_floor_sensor = 0;
	//int last_obst_button = 0;
	int last_stop_button = 0;
	light_status_t light_to_write;
	while(1){

		pthread_mutex_lock(input_event_ptr->mutex);
			pthread_cond_wait(input_event_ptr->cv, input_event_ptr->mutex);

			//last_floor_sensor = Input_Status.floor_sensor;
			//last_obst_button = Input_Status.obst_button;
		input_read = get_input_status_unsafe();

		/* making it triggered in rising edge */
		light_to_write.stop_light = ((input_read.stop_button - last_stop_button) == 1)? (!light_to_write.stop_light) : light_to_write.stop_light;
		last_stop_button = input_read.stop_button;
		/**************************************/
		if(light_to_write.stop_light||input_read.obst_button){
			set_desired_floor(MOTOR_EM_STOP_CMD);
		}
		/******************************************************************************/
		/* button value translated into accepted requests */
// TODO BELOW IS BUGGY CODE. MAKE FLOOR TO SET TO 3.
//		for( int i = 0; i < N_FLOORS; i++){
//			for( int j = 0; j < 3; j++){
//				if(light_to_write.floor_button_lights[i][j] != -1){
//					light_to_write.floor_button_lights[i][j]
//					= (input_read.Button_external[i][j])? IsRequestAccepted(j, i) : light_to_write.floor_button_lights[i][j];
//
//				}
//			}
//		}
		memcpy(light_to_write.floor_button_lights, input_read.Button_external, N_FLOORS*3*sizeof(int) );
		light_to_write.door_open_light = 0;
		set_light_status(light_to_write);
		pthread_mutex_unlock(input_event_ptr->mutex);

	}
		
	return 0;
}
