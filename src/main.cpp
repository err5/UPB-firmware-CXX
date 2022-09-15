#include <stdio.h>
#include <string>
#include <vector>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "display_controller/display_controller.h"


#define SDA_PIN 4
#define SCL_PIN 5

#define THERMISTOR_A_PIN 28
#define ONBOARD_TEMP_PIN 26

int main() {
	stdio_init_all();
	adc_init();
	adc_set_temp_sensor_enabled(true);

	adc_gpio_init(26);
	adc_gpio_init(28);

	// Init i2c0 controller
	i2c_init(i2c0, 1000000);
	// Set up pins 12 and 13
	gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(SDA_PIN);
	gpio_pull_up(SCL_PIN);

	// delay for i2c to keep up 
	sleep_ms(250);


	// Create a new display object at address 0x3C and size of 128x64
	pico_ssd1306::SSD1306 display_driver = pico_ssd1306::SSD1306(i2c0, 0x3C, pico_ssd1306::Size::W128xH64);

	display_controller::Display display(&display_driver);

	display.main_menu();

	int i = 0;
	int port_mode = 0;

	const char* test_err[32] = { "Fuck", "you", "third line" };
	const char* test_warn[32] = { "Bitch", "you'll", "ligma balls", "fourth line" };

	uint64_t msg_timer = 0;
	bool msg_switch = true;

	while (true) {
		// check if msg is being displayed
			// display testing
		display.update_port_a(port_mode);
		display.update_port_b(port_mode);
		display.update_port_c(port_mode + 6);
		display.update_battery(i);
		i++;
		if (i >= 120) {
			i = 0;
		}
		port_mode++;
		if (port_mode >= 5) {
			port_mode = 0;
		}
		if (msg_timer < time_us_64()) {
			if (msg_switch) {
				display.warning("ligma balls", test_warn);
				msg_switch = false;
			}
			else {
				display.error("ligma balls", test_err);
				msg_switch = true;
			}
			msg_timer = time_us_64() + 10000000;
			sleep_ms(500);
		}

		printf("---- MAIN LOOP END ----\n");
		sleep_ms(10);
	};
}

