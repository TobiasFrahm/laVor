/* receiver.c
 *
 *  Created on: 18.12.2018


 *      Author: malte
 *
 * this code is part of the LaVOR system application. Please see the license.txt
 * file for further information.
 *
 * The code is licensed under the
 *
 * 		Creative Commmons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * You are free to use this code for non-commercial applications as long as you leave this
 * attribution included in all copies or substantial portions of the Software.
 
 * Authors: Tobias Frahm, Philipp Haenyes, Joschka Sondhof, Josefine Palm, Malte Rejzek
 */

#include "receiver.h"


void receiver_pos_task(void){

	static const char* TAG = "Mean Position";


	int pos_counter = 0;
	int meanX = 0;
	int meanY = 0;
	struct pos_t rx_pos;

	while(1){
		xQueueReceive(position_queue, &rx_pos, portMAX_DELAY);

		meanX += rx_pos.posX;
		meanY += rx_pos.posY;
		pos_counter ++;

		if (pos_counter >= 30){
			meanX /= pos_counter;
			meanY /= pos_counter;

			ESP_LOGI(TAG, "Mean Position X: %d \t Y: %d", meanX, meanY);

			meanX = 0;
			meanY = 0;
			pos_counter = 0;
		}

	}


}

void receiver_calc(int cap_1, int cap_2) {

	static const char* TAG = "Receiver calculation";

	struct pos_t calc_pos;

	//Capture sort to order by size
	int cap1 = 0;
	int cap2 = 0;

	if (cap_1 > cap_2) {
		cap1 = cap_2;
		cap2 = cap_1;
	} else {
		cap1 = cap_1;
		cap2 = cap_2;
	}

	printf("Capture: %d \t %d \n", cap1, cap2);

	float xp = -1;
	float yp = -1;

	float m1 = 1.0 / tan((float) cap1 * 5.0265e-5);
	float m2 = 1.0 / tan((float) cap2 * 5.0265e-5);

	//printf("M1: %f M2: %f \n", m1, m2);

	// Beacon 1 and Beacon 2
	if (cap1 >= 0 && cap1 < 31250 && cap2 >= 31250 && cap2 < 62500) {
		xp = (BEACON_2_Y) / (m1 - m2);
		yp = m1 * xp;

		ESP_LOGI(TAG, "1 and 2 --> X: %f \t Y: %f", xp, yp);
		xQueueSend(position_queue, &calc_pos, NULL);

	}
	// Beacon 2 and Beacon 3( vlt nicht sinnvoll?!?
	else if (0 && cap1 >= 31250 && cap1 < 62500 && cap2 >= 93750 && cap2 < 125000) {
		xp = ((-m2 * BEACON_3_X) - BEACON_2_Y) / (m1 - m2);
		yp = (m1 * xp) + BEACON_2_Y;

		ESP_LOGI(TAG, "2 and 3 --> X: %f \t Y: %f", xp, yp);
		xQueueSend(position_queue, &calc_pos, NULL);

	}
	//Beacon 1 and Beacon 3
	else if (cap1 >= 0 && cap1 < 31250 && cap2 >= 93750 && cap2 < 125000) {
		xp = (-m2 * BEACON_3_X) / (m1 - m2);
		yp = m1 * xp;

		ESP_LOGI(TAG, "1 and 3 --> X: %f \t Y: %f", xp, yp);
		xQueueSend(position_queue, &calc_pos, NULL);

	}
	// Values out of Range
	else {
		ESP_LOGW(TAG, "Values out of Range!");
	}

}

//void receiver_run(void) {
//
//	receiver_init();
//	uint64_t timer_value;
//	int capture = 0;
//	int capture_diff = 0;
//	int last_capture = 0;
//
//
//
//	while (1) {
//		xQueueReceive(receiver_queue, &timer_value, portMAX_DELAY);
//		capture = (int) timer_value;
//		//printf("Capture: %d \n", capture);
//
//		capture_diff = capture - last_capture;
//		if (capture_diff < 0)
//			capture_diff += 125000;
//
//		if (capture_diff > LASER_INTR_DELAY) {
//
//			receiver_calc(capture, last_capture);
//
//			last_capture = capture;
//		}
//	}
//
//}

