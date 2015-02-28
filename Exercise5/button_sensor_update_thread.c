#include "elev.h"
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "channels.h"
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
//struct Input_Status_t {
//pthread_mutex_t lock ;
//// button table to represent the current button status
////each floor has 3 types: UP DOWN CMD
//// -1 stands for not existing
//int Button_external[N_FLOORS][3];
//int floor_sensor;
//int stop_button;
//int obst_button;
//} Input_Status
//=
//{
//	.lock = PTHREAD_MUTEX_INITIALIZER,
//		/*up, 	down,	cmd*/
//	{
//	/*0*/	{0,	-1,	0},
//	/*1*/	{0,	0,	0},
//	/*2*/	{0,	0,	0},
//	/*3*/	{-1,	0,	0}
//	},
//	.floor_sensor = 0,
//	.stop_button = 0,
//	.obst_button = 0
//};
//
//struct {
//pthread_mutex_t lock;
//// button table to represent the current button status
////each floor has 3 types: UP DOWN CMD
//// -1 stands for not existing
//int floor_button_lights[N_FLOORS][3];
//int floor_indicator_light;/* 0 - 3 */
//int stop_light;
//int door_open_light;
//;
//} light_status
//=
//{
//	.lock = PTHREAD_MUTEX_INITIALIZER,
//		/*up, 	down,	cmd*/
//	{
//	/*0*/	{0,	-1,	0},
//	/*1*/	{0,	0,	0},
//	/*2*/	{0,	0,	0},
//	/*3*/	{-1,	0,	0}
//	},
//	.floor_indicator_light = 0,
//	.stop_light = 0,
//	.door_open_light = 0
//};
//pthread_mutex_t desired_floor_lock = PTHREAD_MUTEX_INITIALIZER;
//int desired_floor = 0;
void *input_polling_thread(){
	while(1){
		pthread_mutex_lock(&(Input_Status.lock));
		puts("reading buttons: outside:\tFLOOR \tUP \tDONW \tCMD ");
		for(int i = 0; i < N_FLOORS; i++ ){
		
			int up = (i==3)? (-1):elev_get_button_signal(BUTTON_CALL_UP,i);
			int down = (i==0)? (-1):elev_get_button_signal(BUTTON_CALL_DOWN,i);
			int cmd = elev_get_button_signal(BUTTON_COMMAND,i);
			Input_Status.Button_external[i][BUTTON_CALL_UP] = up;
			Input_Status.Button_external[i][BUTTON_CALL_DOWN] = down;
			Input_Status.Button_external[i][BUTTON_COMMAND] = cmd;
			printf("\t\t\t\t%d \t%d \t%d \t%d\n", i, 
			up,
			down,
			cmd);
		}
	

		Input_Status.floor_sensor = elev_get_floor_sensor_signal();    
		printf("Current sensor value is:%d\n",Input_Status.floor_sensor);
		Input_Status.obst_button = elev_get_obstruction_signal();    
		printf("Current obst value is:%d\n",Input_Status.obst_button);
		Input_Status.stop_button = elev_get_stop_signal();    
		printf("Current stop value is:%d\n",Input_Status.stop_button);
		pthread_mutex_unlock(&(Input_Status.lock));
		usleep(10000);
	}
	return NULL;
}
pthread_t input_event_parser_th;
pthread_cond_t input_changed_cv = PTHREAD_COND_INITIALIZER;
/* compare latest two polling of input status, if changed then issue an input event */
void *InputEventParser(void * data){
	data = NULL;
	struct Input_Status_t last_input_status;
	while(1){
		pthread_mutex_lock(&(Input_Status.lock));
		last_input_status = Input_Status;
		pthread_mutex_unlock(&(Input_Status.lock));
		usleep(10000);
		pthread_mutex_lock(&(Input_Status.lock));
		if(memcmp(&Input_Status, &last_input_status, sizeof(struct Input_Status_t))){
			pthread_cond_signal(&input_changed_cv);
		}
		pthread_mutex_unlock(&(Input_Status.lock));
	}
	return NULL;
}
void * light_controller_thread(){
	while(1){
		for( int i = 0; i < N_FLOORS; i++){
			for( int j = 0; j < 3; j++){
				if(light_status.floor_button_lights[i][j] != -1)
				elev_set_button_lamp(j, i, light_status.floor_button_lights[i][j]);
			}
		}
		
		elev_set_floor_indicator(light_status.floor_indicator_light);
		elev_set_stop_lamp(light_status.stop_light);
		elev_set_door_open_lamp(light_status.door_open_light);
	}
	usleep(10000);
	return 0;
}

static int motor_moving_vector = 0;
/** 0 - reached desired floor
 *  signed integer - moving directions and distance left, also means busy
 */
int get_motor_moving_vector(void){
	return motor_moving_vector;
}
#define MOTOR_EM_STOP_CMD 0xffff
/* input: DesiredFloor, desired_floor_lock */
void * motor_controller_thread(void * data_motor_controller_ptr)
{
	int read_desired_floor;
	int last_stable_floor = 1; /* init moving downwards until reach a stable */
	while(1)
	{	
		pthread_mutex_lock(&desired_floor_lock);
		read_desired_floor = desired_floor;
		pthread_mutex_unlock(&desired_floor_lock);
		if(
			(read_desired_floor >= 0)&&(read_desired_floor <= N_FLOORS-1)
		    
		  )
		{
			
			int sensor = elev_get_floor_sensor_signal();  /* last floor */
			if(sensor != -1){
				last_stable_floor = sensor;
				motor_moving_vector = read_desired_floor - sensor;
				elev_set_motor_direction(motor_moving_vector);
//				if(motor_moving_vector == 0){
//					/* reach the desired floor, notify the dispatcher */
//				}
			} else {
				/** get to nearest fixed floor downwards, not somewhere in between  **/
				elev_set_motor_direction(read_desired_floor - last_stable_floor);
			}
		} else if (read_desired_floor == MOTOR_EM_STOP_CMD){
			elev_set_motor_direction(0);
		}
		usleep(10000);
	}
}

void * keyboard_read_thread(){
//	FILE * keyin = fopen("./keyboard_in", "r" );
//	FILE * keyout = fopen("./keyboard_out", "w");
	while(1){
		int read;
		printf(  "\nInput floor: ");
		scanf( "%d", &read);
		printf( "\nDesired floor is: %d",read);
		pthread_mutex_lock(&desired_floor_lock);
		desired_floor = read;
		pthread_mutex_unlock(&desired_floor_lock);
	}
	return NULL;
}
int IsRequestAccepted(elev_button_type_t type, int floor){
	/* when motor is free then accepted, change desired floor */
	if(get_motor_moving_vector())
		return 0;
	pthread_mutex_lock(&desired_floor_lock);
	desired_floor = floor;
	pthread_mutex_unlock(&desired_floor_lock);
	return 1;
}
int main(){

	while (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
   	}
	pthread_t button_get_th, light_drive_th, goto_desire_th, keyboard_read_th;
	int rc = pthread_create(&button_get_th, NULL, input_polling_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&light_drive_th, NULL, light_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&goto_desire_th, NULL, motor_controller_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&keyboard_read_th, NULL, keyboard_read_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&input_event_parser_th, NULL, InputEventParser, NULL);
		if (rc){
		perror("pthread_create");
		exit(-1);
	}
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

	while(1){

		pthread_mutex_lock(&(Input_Status.lock));
			pthread_cond_wait(&input_changed_cv, &(Input_Status.lock));

			//last_floor_sensor = Input_Status.floor_sensor;
			//last_obst_button = Input_Status.obst_button;


		if(Input_Status.floor_sensor != -1)
			light_status.floor_indicator_light = Input_Status.floor_sensor;
		/* making it triggered in rising edge */
		light_status.stop_light = ((Input_Status.stop_button - last_stop_button) == 1)? (!light_status.stop_light) : light_status.stop_light;
		last_stop_button = Input_Status.stop_button;
		/**************************************/
		if(light_status.stop_light||Input_Status.obst_button){
			pthread_mutex_lock(&desired_floor_lock);
			desired_floor = MOTOR_EM_STOP_CMD;
			pthread_mutex_unlock(&desired_floor_lock);
		}
		/******************************************************************************/
		/* button value translated into accepted requests */

		for( int i = 0; i < N_FLOORS; i++){
			for( int j = 0; j < 3; j++){
				if(light_status.floor_button_lights[i][j] != -1){
					light_status.floor_button_lights[i][j]
					= (Input_Status.Button_external[i][j])? IsRequestAccepted(j, i)&&get_motor_moving_vector() : light_status.floor_button_lights[i][j];

				}
			}
		}
		memcpy(last_Button_external, Input_Status.Button_external, N_FLOORS*3*sizeof(int) );
		pthread_mutex_unlock(&(Input_Status.lock));
	}
		
	return 0;
}
