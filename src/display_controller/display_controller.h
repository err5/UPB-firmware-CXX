#pragma once
#include <stdio.h>
#include <string>
#include <vector>

#include "../../pico-ssd1306/ssd1306.h"
#include "../../pico-ssd1306/shapeRenderer/ShapeRenderer.h"
#include "../../pico-ssd1306/textRenderer/TextRenderer.h"
#include "../../pico-ssd1306/textRenderer/16x32_font.h"


namespace display_controller {
    enum class DisplayState {
        MAIN_MENU,
        NIGHT_MODE,
        DISPLAY_WARNING,
        DISPLAY_ERROR,
    };

    enum class ChargingModes {
        GEN_5v,
        QC_5v,
        QC_9v,
        QC_15v,
        QC_20v,
        QC_Var,
        PD_5v,
        PD_9v,
        PD_12v,
        PD_20v,
        PD_Var,
        Unknown
    };

    class Display {
    private:
        pico_ssd1306::SSD1306* disp;
        /// @brief state of a display at the moment
        DisplayState current_state;
        uint8_t msg_counter;
        /// @brief timestamp for non-blocking waiting
        uint64_t msg_timestamp;
        /// @brief message wait time in useconds
        uint64_t msg_wait_time = 5000000;

        void port_status(const char* port_name, int pos, ChargingModes charging_mode);
        bool is_msg_displaying();
        void display_msg(const char* heading, const char* msg, const char* details[32]);
        std::vector<std::string> cstr_to_lines(const char* cstr, int line_width);


    public:
        Display(pico_ssd1306::SSD1306* display_driver);
        void main_menu();
        void update_battery(int percentage);
        void update_port_a(int charging_mode);
        void update_port_b(int charging_mode);
        void update_port_c(int charging_mode);
        void warning(const char* msg, const char* details[32]);
        void error(const char* msg, const char* details[32]);
    };

}
