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
typedef struct input_status_st input_status_t;
/** below is the structure of light_status_t**/
//{
// /*int floor_button_lights[N_FLOORS][3];*/
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
typedef struct light_status_st light_status_t;
extern input_status_t get_input_status(void);
extern light_status_t get_light_status(void);
extern void set_light_status(const light_status_t status);
extern int get_desired_floor(void);
extern void set_desired_floor(const int floor);
#endif /* ELEVATOR_MODEL_DATA_STRUCTURE_H_ */
