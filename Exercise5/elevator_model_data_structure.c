/*****************************************************************
 * elevator_model_data_structure.c
 * Exercise5
 *
 *  Created on		: Feb 28, 2015 
 *  Author			: yulongb
 *	Email			: yulongb@stud.ntnu.no
 *  Description		:
 *****************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "elevator_model_data_structure.h"

#define MOTOR_EM_STOP_CMD 0xffff
pthread_mutex_t input_status_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t input_changed_cv = PTHREAD_COND_INITIALIZER;
static event_t input_events
={
		.cv = &input_changed_cv,
		.mutex = &input_status_lock
};

event_t * const input_event_ptr = &input_events;

static input_status_t input_status
=
{
		/*up, 	down,	cmd*/
	{
	/*0*/	{0,	-1,	0},
	/*1*/	{0,	0,	0},
	/*2*/	{0,	0,	0},
	/*3*/	{-1,	0,	0}
	},
	.floor_sensor = 0,
	.stop_button = 0,
	.obst_button = 0
};

input_status_t get_input_status(void){
	pthread_mutex_lock(&input_status_lock);
	input_status_t ret = input_status;
	pthread_mutex_unlock(&input_status_lock);
	return ret;
}
input_status_t get_input_status_unsafe(void){
	return input_status;
}
/**
 *  light status related
 */
static pthread_mutex_t light_status_lock = PTHREAD_MUTEX_INITIALIZER;

static light_status_t light_status
=
{
		/*up, 	down,	cmd*/
	{
	/*0*/	{0,	-1,	0},
	/*1*/	{0,	0,	0},
	/*2*/	{0,	0,	0},
	/*3*/	{-1,	0,	0}
	},
	.floor_indicator_light = 0,
	.stop_light = 0,
	.door_open_light = 0
};
light_status_t get_light_status(void){
	pthread_mutex_lock(&light_status_lock);
	light_status_t ret = light_status;
	pthread_mutex_unlock(&light_status_lock);
	return ret;
}
void set_light_status(const light_status_t status){
	pthread_mutex_lock(&light_status_lock);
	light_status = status;
	light_status.floor_button_lights[0][1]=-1;
	light_status.floor_button_lights[N_FLOORS-1][0]=-1;
	pthread_mutex_unlock(&light_status_lock);
}
/**
 *  floor controlling related
 */
static pthread_mutex_t desired_floor_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t desired_floor_reached_cv = PTHREAD_COND_INITIALIZER;
static int desired_floor = 0;
static event_t floor_reached_event
={
		.cv = &desired_floor_reached_cv,
		.mutex = &desired_floor_lock
};

event_t * const floor_reached_event_ptr = &input_events;
int get_desired_floor(void){
	pthread_mutex_lock(&desired_floor_lock);
	int ret = desired_floor;
	pthread_mutex_unlock(&desired_floor_lock);
	return ret;
}
void set_desired_floor(const int floor){
	if((floor>=0 && floor<=N_FLOORS-1) || floor==MOTOR_EM_STOP_CMD)
	{
		pthread_mutex_lock(&desired_floor_lock);
		desired_floor = floor;
		pthread_mutex_unlock(&desired_floor_lock);
	}
}
void set_desired_floor_unsafe(const int floor){
	desired_floor = floor;
}
void *input_polling_thread(void * data){
	data = NULL;
	while(1){
		pthread_mutex_lock(&input_status_lock);
		puts("reading buttons: outside:\tFLOOR \tUP \tDONW \tCMD ");
		for(int i = 0; i < N_FLOORS; i++ ){

			int up = (i==3)? (-1):elev_get_button_signal(BUTTON_CALL_UP,i);
			int down = (i==0)? (-1):elev_get_button_signal(BUTTON_CALL_DOWN,i);
			int cmd = elev_get_button_signal(BUTTON_COMMAND,i);
			input_status.Button_external[i][BUTTON_CALL_UP] = up;
			input_status.Button_external[i][BUTTON_CALL_DOWN] = down;
			input_status.Button_external[i][BUTTON_COMMAND] = cmd;
			printf("\t\t\t\t%d \t%d \t%d \t%d\n", i,
			up,
			down,
			cmd);
		}


		input_status.floor_sensor = elev_get_floor_sensor_signal();
		printf("Current sensor value is:%d\n",input_status.floor_sensor);
		input_status.obst_button = elev_get_obstruction_signal();
		printf("Current obst value is:%d\n",input_status.obst_button);
		input_status.stop_button = elev_get_stop_signal();
		printf("Current stop value is:%d\n",input_status.stop_button);

		printf("Current floor value is:%d\n",desired_floor);

		pthread_mutex_unlock(&input_status_lock);
		usleep(50000);
	}
	return NULL;
}

/** 0 - reached desired floor
 *  signed integer - moving directions and distance left, also means busy
 */
static int motor_moving_vector = 0;
int get_motor_moving_vector(void){
	return motor_moving_vector;
}

/* input: read_desired_floor */
void * motor_driver_thread(void * data_motor_controller_ptr)
{
	data_motor_controller_ptr = NULL;
	int read_desired_floor = 0;
	int last_stable_floor = 1; /* init moving downwards until reach a stable */
	while(1)
	{
		read_desired_floor = desired_floor;
		//read_desired_floor = desired_floor;
		if(
			(read_desired_floor >= 0)&&(read_desired_floor <= N_FLOORS-1)

		  )
		{

			int sensor = elev_get_floor_sensor_signal();  /* last floor */
			if(sensor != -1){
				last_stable_floor = sensor;
				motor_moving_vector = read_desired_floor - sensor;
				elev_set_motor_direction(motor_moving_vector);
				if(motor_moving_vector == 0){
					/* reach the desired floor, notify the dispatcher */
					pthread_mutex_lock((floor_reached_event.mutex));
						pthread_cond_broadcast((floor_reached_event.cv));
					pthread_mutex_unlock((floor_reached_event.mutex));
				}
			} else {
				/** get to nearest fixed floor downwards, not somewhere in between  **/
				elev_set_motor_direction(read_desired_floor - last_stable_floor);
				/**
				 * TODO add timeout detection here. if the sensor
				 * keep -1 for longer than 10 seconds, change direction;
				 * if still -1, then means power was down, init necessarily
				 */
			}
		}

		if (light_status.stop_light||input_status.obst_button||light_status.door_open_light){
			elev_set_motor_direction(0);
			motor_moving_vector = 0;
		}
		usleep(100000);
	}
}

void * light_driver_thread(){
	//light_status_t light_status;
	while(1){
		//light_status = get_light_status();
		for( int i = 0; i < N_FLOORS; i++){
			for( int j = 0; j < 3; j++){
				if(light_status.floor_button_lights[i][j] != -1)
				{
					if((i==0 && j==BUTTON_CALL_DOWN)||(i==(N_FLOORS-1)&& j== BUTTON_CALL_UP))
						continue;
					elev_set_button_lamp(j, i, light_status.floor_button_lights[i][j]);
				}
			}
		}
		int floor_indicator_light = elev_get_floor_sensor_signal();
		if(floor_indicator_light>=0 && floor_indicator_light< N_FLOORS)
			elev_set_floor_indicator(floor_indicator_light);

		elev_set_stop_lamp(light_status.stop_light);
		elev_set_door_open_lamp(light_status.door_open_light);
		usleep(10000);
	}

	return 0;
}

/* compare latest two polling of input status, if changed then issue an input event */
/* the waiting one should wait for event_t to capture and parse relative event */
void *input_event_dispatcher_thread(void * data){
	data = NULL;
	input_status_t last_input_status;
	while(1){
		last_input_status = get_input_status();
		usleep(10000);
			pthread_mutex_lock((input_events.mutex));
			last_input_status.floor_sensor = input_status.floor_sensor;/* exclude sensor light */
			/* issue only rising edge triggered event */
			if(memcmp(&input_status, &last_input_status, sizeof(input_status_t)) > 0){
				pthread_cond_broadcast((input_events.cv));
			}
			pthread_mutex_unlock((input_events.mutex));
	}
	return NULL;
}

static pthread_t input_polling_th, motor_driver_th, input_event_dispatcher_th, light_driver_th;

int elevator_model_init( FILE * log) {
	log = NULL;
	while (!elev_init()) {
	        printf("Unable to initialize elevator hardware!\n");
	}
	int rc = pthread_create(&input_polling_th, NULL, input_polling_thread, NULL);
	if (rc){
		perror("pthread_create in elevator_motor_init");
		exit(-1);
	}
	rc = pthread_create(&motor_driver_th, NULL, motor_driver_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&input_event_dispatcher_th, NULL, input_event_dispatcher_thread, NULL);
		if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&light_driver_th, NULL, light_driver_thread, NULL);
			if (rc){
			perror("pthread_create");
			exit(-1);
	}
	return rc;
}
//int main(void) {
//	return 0;
//}


