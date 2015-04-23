#include "new.h"
#include <stdio.h>
#include <pthread.h>

pthread_t init_thread;

int network_test(void) {
	//create interface_t struct for sharing with comm. moduel
	my_interface_t* in_main_interface = malloc(sizeof(my_interface_t));

	//variables for status....
	char my_status[SEND_SIZE];
	int current_floor = 0;
	char current_dir = 0;
	int temp_order_floor = 0;

	pthread_create(&init_thread, NULL, initialzie_function, in_main_interface);

	int cmd;
	char status_buffer[SEND_SIZE];
	int rand_floor;
	char rand_dir;
	int rand_flag;
	while (1) {

		//sleep(2);
		rand_floor = rand() % 5;

		if (rand_floor == 1 || rand_floor == 3) {
			rand_dir = 'U';
		}

		else {
			rand_dir = 'D';
		}

		if (rand_floor > 3) {
			cmd = 0;
		}

		else
			cmd = 1;

		sprintf(status_buffer, "MY_STATUS_%d_%c", rand_floor, rand_dir);

		printf("Main:My status is %s\n", status_buffer);

		if (cmd == 1) {

			pthread_mutex_lock(&in_main_interface->interface_mutex);

			in_main_interface->order_floor = rand() % 4;
			strncpy(in_main_interface->interface_status_buffer, status_buffer,
					13);
			in_main_interface->order_direction = rand_dir;
			in_main_interface->order_floor_flag = TRUE;

			pthread_mutex_unlock(&in_main_interface->interface_mutex);

			usleep(100 * MS);

			pthread_mutex_lock(&in_main_interface->interface_mutex);

			while (1) {

				if (in_main_interface->order_floor_flag == FALSE) {
					printf("Main:Order Flag Cleared by comm module....\n");
					temp_order_floor = in_main_interface->order_floor;
					pthread_mutex_unlock(&in_main_interface->interface_mutex);
					break;
				}

				else {
					pthread_mutex_unlock(&in_main_interface->interface_mutex);
					usleep(DELAY * MS);
				}
			}

			printf("Final order is %d -1 means someone else is doing\n",
					temp_order_floor);
			sleep(2);

		}

		else {

			pthread_mutex_lock(&in_main_interface->interface_mutex);
			in_main_interface->order_floor_flag = FALSE;
			pthread_mutex_unlock(&in_main_interface->interface_mutex);

			sleep(2);

		}

	}

	pthread_join(init_thread, NULL);

	return 0;

}

