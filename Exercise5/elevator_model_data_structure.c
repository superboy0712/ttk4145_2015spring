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
#include "elev.h"
static pthread_mutex_t input_status_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct input_status_st{

	// button table to represent the current button status
	//each floor has 3 types: UP DOWN CMD
	// -1 stands for not existing
	int Button_external[N_FLOORS][3];
	int floor_sensor;
	int stop_button;
	int obst_button;
} input_status_t;

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
/**
 *  light status related
 */
static pthread_mutex_t light_status_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct light_status_st{
	// button table to represent the current button status
	//each floor has 3 types: UP DOWN CMD
	// -1 stands for not existing
	int floor_button_lights[N_FLOORS][3];
	int floor_indicator_light;/* 0 - 3 */
	int stop_light;
	int door_open_light;
} light_status_t;

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
	pthread_mutex_unlock(&light_status_lock);
}
/**
 *  floor controlling related
 */
static pthread_mutex_t desired_floor_lock = PTHREAD_MUTEX_INITIALIZER;
static int desired_floor = 0;
int get_desired_floor(void){
	pthread_mutex_lock(&desired_floor_lock);
	int ret = desired_floor;
	pthread_mutex_unlock(&desired_floor_lock);
	return ret;
}
void set_desired_floor(const int floor){
	pthread_mutex_lock(&desired_floor_lock);
	desired_floor = floor;
	pthread_mutex_unlock(&desired_floor_lock);
}

int main(int argc, char **argv) {
	argc = 1;
	argv = NULL;
	return 0;
}

