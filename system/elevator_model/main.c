#include "elev.h"
#include <stdio.h>
#include "io.h"
#include "channels.h"
#include <pthread.h>
#include <unistd.h>
#include <assert.h>


int goto_desired_floor(int floor)
{
	assert((floor >= 0)&&(floor <= 3));
	while(1)
	{
		int sensor = elev_get_floor_sensor_signal();  
		if(sensor != -1){
			elev_set_motor_direction(floor - sensor);
		}
		if(floor == sensor){
			return 1;
		}
	}
}
int main() {

	int sensor=0;
    // Initialize hardware
    if (!elev_init()) {
        printf("Unabl e to initialize elevator hardware!\n");
        return 1;
    }

  //  printf("Press STOP button to stop elevator and exit program.\n");

   // elev_set_motor_direction(DIRN_UP);
	int light = 0;
	int j = 0;
	int floor=0;
    while (1) {
	puts("reading buttons: outside:\tFLOOR \tUP \tDONW ");
	for(int i = 0; i < N_FLOORS; i++ ){
		
		int up = (i==3)? 0xff:elev_get_button_signal(BUTTON_CALL_UP,i);
		int down = (i==0)? 0xff:elev_get_button_signal(BUTTON_CALL_DOWN,i);
		printf("\t\t\t\t%d \t%d \t%d\n", i, 
		up,
		down);
	}
	puts("inside:\t\tfloor \tCMD");
	for(int i = 0; i < N_FLOORS; i++ ) {
		printf("\t\t%d,\t%d\n", i, 
		elev_get_button_signal(BUTTON_COMMAND,i));
	}
	
	printf("\nwaiting for input\n");
	scanf("%d",&floor);
	printf("\nvalue entered is:%d",floor);

	goto_desired_floor(floor);
	

	sensor=elev_get_floor_sensor_signal();    
	printf("Current sensor value is:%d\n",sensor);
	printf("Current Obstru value is:%d\n",elev_get_obstruction_signal());
	
	for(int i = 0; i < N_FLOORS-1; i++ ){
		elev_set_button_lamp(BUTTON_CALL_UP, i, light);
		elev_set_button_lamp(BUTTON_CALL_DOWN, i+1, light);
		elev_set_button_lamp(BUTTON_COMMAND, i, light);
		//elev_set_floor_indicator(i);
	}
	elev_set_button_lamp(BUTTON_COMMAND, 3, light);
	elev_set_floor_indicator(j%4);
	j++;
	elev_set_stop_lamp(light);
	elev_set_door_open_lamp(light);
	light = !light;
	sleep(1);
		
    }
	
    return 0;
}

