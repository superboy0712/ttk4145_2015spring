#include "elev.h"
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "channels.h"
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
struct {
pthread_mutex_t lock ;
// button table to represent the current button status
//each floor has 3 types: UP DOWN CMD
// -1 stands for not existing
int Button_external[N_FLOORS][3];
int floor_sensor;
int stop_button;
int obst_button;
} Input_Status
=
{
	.lock = PTHREAD_MUTEX_INITIALIZER,
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

struct {
pthread_mutex_t lock ;
// button table to represent the current button status
//each floor has 3 types: UP DOWN CMD
// -1 stands for not existing
int Button_lights[N_FLOORS][3];
int floor_light;/* 0 - 3 */
int stop_light;
int door_open_light;
;
} Light_Status
=
{
	.lock = PTHREAD_MUTEX_INITIALIZER,
		/*up, 	down,	cmd*/
	{ 	
	/*0*/	{0,	-1,	0},
	/*1*/	{0,	0,	0},
	/*2*/	{0,	0,	0},
	/*3*/	{-1,	0,	0}
	},
	.floor_light = 0,
	.stop_light = 0,
	.door_open_light = 0
};
pthread_mutex_t desired_floor_lock = PTHREAD_MUTEX_INITIALIZER;
int DesiredFloor = 0;
void *ButtonGetThread(){
	while(1){
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
	
		usleep(10000);
	}
	return NULL;
}

void * LightDriverThread(){
	while(1){
		for( int i = 0; i < N_FLOORS; i++){
			for( int j = 0; j < 3; j++){
				if(Light_Status.Button_lights[i][j] != -1) 
				elev_set_button_lamp(j, i, Light_Status.Button_lights[i][j]);	
			}
		}
		
		elev_set_floor_indicator(Light_Status.floor_light);
		elev_set_stop_lamp(Light_Status.stop_light);
		elev_set_door_open_lamp(Light_Status.door_open_light);
	}
	usleep(10000);
	return 0;
}


static int motor_moving_vector = 0;
int get_motor_moving_vector(void){
	return motor_moving_vector;
}

void * goto_desired_floor_thread()
{
	int read_desired_floor;
	int last_stable_floor = 1; /* init moving downwards until reach a stable */
	while(1)
	{	
		pthread_mutex_lock(&desired_floor_lock);
		read_desired_floor = DesiredFloor;
		pthread_mutex_unlock(&desired_floor_lock);
		if(
			(read_desired_floor >= 0)&&(read_desired_floor <= 3)
		    
		  )
		{
			
			int sensor = elev_get_floor_sensor_signal();  /* last floor */
			if(sensor != -1){
				last_stable_floor = sensor;
				motor_moving_vector = read_desired_floor - sensor;
				elev_set_motor_direction(motor_moving_vector);
			} else {
				/** get to nearest fixed floor downwards, not somewhere in between  **/
				elev_set_motor_direction(read_desired_floor - last_stable_floor);
			}
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
		DesiredFloor = read;
		pthread_mutex_unlock(&desired_floor_lock);
	}
	return NULL;
}
int main(){

	while (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
   	}
	pthread_t button_get_th, light_drive_th, goto_desire_th, keyboard_read_th;
	int rc = pthread_create(&button_get_th, NULL, ButtonGetThread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&light_drive_th, NULL, LightDriverThread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&goto_desire_th, NULL, goto_desired_floor_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	rc = pthread_create(&keyboard_read_th, NULL, keyboard_read_thread, NULL);
	if (rc){
		perror("pthread_create");
		exit(-1);
	}
	while(1){
		for( int i = 0; i < N_FLOORS; i++){
			for( int j = 0; j < 3; j++){
				Light_Status.Button_lights[i][j] 
				= Input_Status.Button_external[i][j];	
			}
		}
		
		if(Input_Status.floor_sensor != -1)
			Light_Status.floor_light = Input_Status.floor_sensor;

		Light_Status.stop_light = Input_Status.stop_button;
		
		usleep(10000);


	}
		
	pthread_join(button_get_th, NULL);
	pthread_join(light_drive_th, NULL);
	return 0;
}
