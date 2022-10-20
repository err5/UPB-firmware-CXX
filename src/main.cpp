#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"

#include "display_controller/display_controller.h"
#include "sensors/thermistor.h"
#include "charging_protocols/quick_charge.h"

#define DISPLAY_SDA_PIN 4
#define DISPLAY_SCL_PIN 5

#define LOOP_DELAY 100 // in ms

#define WATER_SENSOR_AC1 17
#define WATER_SENSOR_AC2 18
#define WATER_SENSOR_DATA 27

#define ONBOARD_TEMP_PIN 26

#define THERMISTOR_A_PIN 26
#define THERMISTOR_B_PIN 27

#define QC_A_DM_LOW 	8
#define QC_A_DM_HIGH 	9
#define QC_A_DP_LOW 	10
#define QC_A_DP_HIGH	11

#define QC_B_DP_LOW 	12 
#define QC_B_DP_HIGH 	13
#define QC_B_DM_LOW 	14
#define QC_B_DM_HIGH 	15



int main() {
	if (watchdog_caused_reboot()) {
		printf("Rebooted by Watchdog!\n");
		return 0;
	}
	else {
		printf("Clean boot\n");
	}

	// Enable the watchdog, requiring the watchdog to be updated every 100ms, else the chip will reboot
	stdio_init_all();
	adc_init();
	adc_set_temp_sensor_enabled(true);

	// Init i2c0 controller
	i2c_init(i2c0, 1000000);
	// Set up i2c pins
	gpio_set_function(DISPLAY_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(DISPLAY_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(DISPLAY_SDA_PIN);
	gpio_pull_up(DISPLAY_SCL_PIN);

	// delay for i2c to keep up 
	sleep_ms(50);

	// Using display with 0x3C address!
	pico_ssd1306::SSD1306 display_driver = pico_ssd1306::SSD1306(i2c0, 0x3C, pico_ssd1306::Size::W128xH64);

	display_controller::Display display(&display_driver, 5);

	display.main_menu();

	// Init thermistor
	gpio_init(16);
	gpio_set_dir(16, true);
	gpio_put(16, true);
	sensors::Thermistor t1(THERMISTOR_A_PIN, 1, 100000, 100000, 3950);

	// charging_protocols::QuickChargePort qc_port(16, 17);

	charging_protocols::DigitalPin dm(QC_A_DM_LOW, QC_A_DM_HIGH);
	charging_protocols::DigitalPin dp(QC_A_DP_LOW, QC_A_DP_HIGH);
	charging_protocols::QuickChargePort_alt qc(dm, dp);

	int i = 0;
	int port_mode = 0;

	uint64_t msg_timer = time_us_64() + 10 * 1000000;
	bool msg_switch = true;

	// turned off while debugging
	// watchdog_enable(25 + LOOP_DELAY, true);
	while (true) {
		// timing loop length
		double start_time = time_us_64();

		gpio_put(21, true);
		// ---- DISPLAY TESTING ----
		if (true) {
			display.update_port_a(port_mode);
			display.update_port_b(port_mode);
			display.update_port_c(port_mode + 6);
			display.update_battery(i);
			i++;
			if (i >= 120) { i = 0; }
			port_mode++;
			if (port_mode >= 5) {
				port_mode = 0;
			}

			if (msg_timer <= time_us_64()) {
				display.error("ligma balls", " aslk dj lk jaeioj apr3 98r p;iha jksdh 7hcj ,zn mxb3hb ");
				msg_timer = time_us_64() + 10 * 1000000;
			}
		}
		// ----QC TESTING----
		qc.begin();
		qc.request(ChargingModes::QC_20v);
		sleep_ms(6000);
		qc.request(ChargingModes::QC_12v);
		sleep_ms(6000);
		// ---- SENSORS TESTING ----
		printf("Temperature: %.2fC\n", t1.get());

		// ---- TECHNICAL ----
		printf("---- MAIN LOOP END ----\n");
		int end_time = time_us_64();
		float executionTime = ((end_time - start_time)) / 1000;
		printf("time since start: %dms\nloop time: %.3fms\n", (int)(time_us_64() / 1000), executionTime);


		watchdog_update();
		sleep_ms(LOOP_DELAY);
	};
}

