#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "../pico-ssd1306/ssd1306.h"
#include "../pico-ssd1306/shapeRenderer/ShapeRenderer.h"
#include "../pico-ssd1306/textRenderer/TextRenderer.h"
#include "../pico-ssd1306/textRenderer/16x32_font.h"

// Use the namespace for convenience



enum display_state {
	MAIN_MENU,
	NIGHT_MODE,
	DISPLAY_WARNING,
	DISPLAY_ERROR,
};

class Display {
public:
	pico_ssd1306::SSD1306* disp;
	display_state current_state;

	Display(pico_ssd1306::SSD1306* display_driver) {
		disp = display_driver;
		// disp->setOrientation(2);
		current_state = MAIN_MENU;
		disp->setOrientation(0);
		disp->clear();
	};

	void main_menu() {
		printf("drawing main menu\n");

		disp->clear();

		// display borders
		// pico_ssd1306::drawRect(disp, 0, 0, 127, 63);


		// draw battery area with 0 percent
		update_battery(-1);

		// draw
		disp->sendBuffer();
		current_state = MAIN_MENU;
	}
	/// @brief set battery percentage on a display
	/// @param percentage current battery charge mapped from 0 to 100
	void update_battery(int percentage) {
		if (current_state != MAIN_MENU) {
			printf("tried to redraw percentage while not in main menu\n");
			return;
		}
		printf("setting battery percentage to %03d%\n", percentage);

		if (percentage >= 100) {
			// fill percentage box
			pico_ssd1306::fillRect(disp, 88, 0, 127, 63, pico_ssd1306::WriteMode::ADD);

			// format & display 100 inverted
			pico_ssd1306::drawText(disp, font_16x32, "100", 94, 8,
				pico_ssd1306::WriteMode::INVERT,
				pico_ssd1306::Rotation::deg90);

			// draw inverted percentage symbol
			pico_ssd1306::drawText(disp, font_8x8, "%", 88, 53,
				pico_ssd1306::WriteMode::INVERT,
				pico_ssd1306::Rotation::deg90);
			disp->sendBuffer();
			return;
		}

		// offset if value is only 1 digit 
		int digits_offset = 8;

		if (percentage >= 10) {
			digits_offset = 0;
		}

		// clear percentage area on a screen
		pico_ssd1306::fillRect(disp, 87, 0, 127, 63, pico_ssd1306::WriteMode::SUBTRACT);

		// percentage bar
		pico_ssd1306::drawRect(disp, 87, 0, 98, 63);
		pico_ssd1306::fillRect(disp, 87, 1, 98, percentage * (63. / 100.), pico_ssd1306::WriteMode::ADD);

		// format & display battery value
		char buffer[4];
		sprintf(buffer, "%d", percentage);
		pico_ssd1306::drawText(disp, font_16x32, buffer, 94, 16 + digits_offset,
			pico_ssd1306::WriteMode::INVERT,
			pico_ssd1306::Rotation::deg90);

		// draw percentage symbol
		pico_ssd1306::drawText(disp, font_8x8, "%", 88, 53,
			pico_ssd1306::WriteMode::INVERT,
			pico_ssd1306::Rotation::deg90);

		disp->sendBuffer();
	}
};


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
	Display display(&display_driver);
	display.main_menu();


	while (true) {
		const float conversion_factor = 3.3f / (1 << 12);
		adc_select_input(4);
		float onboard_temp_voltage = adc_read() * conversion_factor;
		float onboard_temp_celsius = 27 - (onboard_temp_voltage - 0.706) / 0.001721;
		adc_select_input(2);
		float thermistor_a_voltage = adc_read() * conversion_factor;

		printf("termistor temp raw: %f\n", thermistor_a_voltage);
		printf("onboard temp: %f C\n", onboard_temp_celsius);


		sleep_ms(250);
	};
}
