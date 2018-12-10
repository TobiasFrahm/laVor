/*
 * beacon.c
 *MOST CODE IS BASED ON EXAMPLES FROM https://github.com/espressif/esp-idf
 *  Created on: Nov 2, 2018
 *      Author: Tobias Frahm
 *
 * This code is part of the LaVOR system application. Please see the license.txt
 * file for further information.
 *
 * The code is licensed under the
 *
 * 		Creative Commmons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * You are free to use this code for non-commercial applications as long as you leave this
 * attribution included in all copies or substantial portions of the Software.
 *
 * Authors: Tobias Frahm, Philipp Haenyes, Joschka Sondhof, Josefine Palm, Malte Rejzek
 */
#include "beacon.h"

static const char* TAG = "Beacon Control";

void beacon_controller(void *pvParameters){
	//control
	beacon_controller_init();




			// Berechnet die Schrittweite für Lageregler
			float a_step = CON_SPEED_SETPOINT;
			// Werte für Regler, update später über MQTT!
			float s_setpoint = CON_SPEED_SETPOINT;
			float p = CON_P;
			float i = CON_I;
			float d = CON_D;
			float a = CON_A;

			float a_setpoint = 0;

			float s_e_sum = 0;
			float s_e_old = 0;
			float y = 20.0;
			float speed = 0.0;
			// Ziele für Queues
			// uint32_t capture = 0;
			int count = 0;
			float old_count = 0.0;
	        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM0A, MCPWM_OPR_A, y); // "Anschubsen"
	    	vTaskDelay(100/portTICK_PERIOD_MS);


		while (1) {



			        xQueueReceive(timer_queue, &count, portMAX_DELAY);

					//xQueueReceive(cap_queue, &capture, 0);
					//if(capture > 0 ) speed = (4000000000 / capture) * 2; //Speed in mHz

			        speed = (float)count - old_count;
			        if (speed < 0) speed += ENCODER_CPR;
			        if (speed > 200) speed = s_setpoint;

			        float a_e = a_setpoint - (float)count;
			        if (a_e > 800.0) a_e -= 1600;
			        if (a_e <= -800.0) a_e += 1600;


			        float s_e = s_setpoint - speed + (a_e * a) ; // Regelfehler berechnen

			        if (y < 100.0) s_e_sum += s_e; //Integrierer Begrenzt in Grenze der Stellgröße(ANTI-WINDUP)
			        y = (p*s_e) + (i/CON_FREQUENCY*s_e_sum) + d*CON_FREQUENCY*(s_e - s_e_old) ;

			        // Begrenzung für Duty-Cycle
			        if(y >= 100.0) y = 100.0;
			        if(y < 0.0) y = 0.0;

			        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM0A, MCPWM_OPR_A, y);



			        printf("Speed: %f \t A_set %f  \t  A_e: %f \n", speed, a_setpoint, a_e);
			        //printf("Speed: ");
			        //printf("%d \n", speed);

			        // Neuer Sollwert für Winkelcontrol
			      		        	a_setpoint += a_step;
			      		        	if (a_setpoint >= ENCODER_CPR) a_setpoint = 0;

			      		        	s_e_old = s_e;
			      		        	old_count = count;

		}

}



void beacon_slave_run(void *pvParameters) {
	//control
	ESP_LOGI(TAG, "Startup");
	beacon_salve_init();
	while (1) {

		vTaskDelay(500);
	}
}

void beacon_slave_test_run(void *pvParameters) {
	//control
	struct upd_event_t *udp_event;
	esp_mqtt_event_handle_t event;

	double time;
	ESP_LOGI(TAG, "Startup");
	beacon_salve_init();

	while (1) {

		if (udpQueue != 0) {
			if (xQueueReceive(udpQueue, &(udp_event), (TickType_t ) 10)) {
				ESP_LOGI(TAG, "udp_received");
				timer_pause(TIMER_GROUP_0, TIMER_0);
				timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &time);
				timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
				timer_start(TIMER_GROUP_0, TIMER_0);
				ESP_LOGI(TAG, "Time[s] since last reset: %f", time);
			}
		}
		if (mqttQueue != 0) {
			if (xQueueReceive(mqttQueue, &(event), (TickType_t ) 10)) {
				ESP_LOGI(TAG, "mqtt_received");
				//printf("DATA=%.*s\r\n", event->data_len, event->data);
				cjson_mc(event->data);

			}
		}
		//vTaskDelay(2000);
		//esp_mqtt_client_publish(mqttClient, "/esp/test0", "test-esp", sizeof("test-esp"), 0, 0);
	}
}


