#pragma once
#include <stdio.h>
#include <string>
#include <vector>

#include "../../pico-ssd1306/ssd1306.h"
#include "../../pico-ssd1306/shapeRenderer/ShapeRenderer.h"
#include "../../pico-ssd1306/textRenderer/TextRenderer.h"
#include "../../pico-ssd1306/textRenderer/16x32_font.h"

namespace display_controller {
    /// @brief possible states of the display
    enum class DisplayState {
        MAIN_MENU,
        NIGHT_MODE,
        DISPLAY_WARNING,
        DISPLAY_ERROR,
    };
    /// @brief possible modes of all ports
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
        /// @brief display driver, used to draw text & shapes
        pico_ssd1306::SSD1306* disp;

        /// @brief state of a display at the moment
        DisplayState current_state;

        /// @brief timestamp for non-blocking waiting
        uint64_t last_msg_timestamp;

        /// @brief message wait time in microseconds
        uint64_t msg_wait_time;

        /// @brief USB port status update/redraw, height of an element is 24 pixels
        /// @param port_name short port name (6 char max)
        /// @param pos vertical position of UI element
        /// @param charging_mode integer value of PD_Charging  
        void port_status(const char* port_name, int pos, ChargingModes charging_mode);

        /// checks timer to answer if message should be displaying now
        bool is_msg_displaying();

        /// @brief clear up & display msg filling screen
        /// @param heading big characters on top, max is 4
        /// @param msg the message
        /// @param details details of message
        void display_msg(const char* heading, const char* msg, const char* details);

        /// @brief Chopping const char* into string array with max length of each entry <= line_width, word endings are preserved.
        /// Exception: If single word is longer than line_width then it's not chopped. 
        /// Used to map words to fixed width screen.
        /// @param cstr const char* to map
        /// @param line_width max length of one entry in array
        /// @return array of strings, each string length will not exceed line_width 
        std::vector<std::string> cstr_to_lines(const char* cstr, int line_width);


    public:

        /// @brief Display constructor
        /// @param display_driver initialized display driver that's used to draw text & shapes 
        /// @param msg_length time to wait when showing message in seconds
        Display(pico_ssd1306::SSD1306* display_driver, int msg_length);

        /// @brief clear up and initialize main menu
        void main_menu();

        /// @brief set battery percentage on a display
        /// @param percentage current battery charge mapped from 0 to 100
        void update_battery(int percentage);

        /// @brief redraw charging mode of 1st (higher) USB-A Port on the display
        /// @param charging_mode PD_Mode or QC_Mode enum!
        void update_port_a(int charging_mode);

        /// @brief redraw charging mode of 2nd (lower) USB-A Port on the display
        /// @param charging_mode PD_Mode or QC_Mode enum!
        void update_port_b(int charging_mode);

        /// @brief redraw charging mode of Type-C Port on the display
        /// @param charging_mode PD_Mode or QC_Mode enum!
        void update_port_c(int charging_mode);

        /// @brief display a warning on the screen
        /// @param msg warning 
        /// @param details array of lines
        void warning(const char* msg, const char* details);

        /// @brief display an error on the screen
        /// @param msg warning 
        /// @param details array of lines
        void error(const char* msg, const char* details);
    };

}
