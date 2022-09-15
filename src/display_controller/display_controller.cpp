#include "display_controller.h"

namespace display_controller {

    Display::Display(pico_ssd1306::SSD1306* display_driver) {
        disp = display_driver;
        current_state = DisplayState::MAIN_MENU;
        disp->setOrientation(1);
        disp->clear();
        msg_timestamp = 0;
    };

    bool Display::is_msg_displaying() {
        if (msg_timestamp >= time_us_64()) {
            return true;
            printf("tried to redraw percentage while message is shown\n");
        }
        else {
            return false;
            current_state = DisplayState::MAIN_MENU;
        }
    }

    /// @brief clear up and initialize main menu
    void Display::main_menu() {
        if (is_msg_displaying()) { return; }

        printf("drawing main menu\n");

        disp->clear();

        // battery percentage & port status divider
        pico_ssd1306::drawRect(disp, 32, 8, 33, 38);

        // draw battery area with 0 percent
        update_battery(-1);

        // draw
        disp->sendBuffer();
        current_state = DisplayState::MAIN_MENU;
    }

    /// @brief set battery percentage on a display
    /// @param percentage current battery charge mapped from 0 to 100
    void Display::update_battery(int percentage) {
        if (is_msg_displaying()) { return; }

        printf("updating battery percentage to %02d%\n", percentage);

        // offset if value is only 1 digit 
        int digits_offset = 0;
        if (percentage >= 10) {
            digits_offset = -8;
        }
        if (percentage >= 100) {
            digits_offset = -16;
            percentage = 100;
        }

        // clear up percentage VALUE
        pico_ssd1306::fillRect(disp, 0, 0, 30, 48, pico_ssd1306::WriteMode::SUBTRACT);
        // clear up percentage BAR 
        pico_ssd1306::fillRect(disp, 1, 50, 126, 62, pico_ssd1306::WriteMode::SUBTRACT);

        // vertical percentage bar
        pico_ssd1306::drawRect(disp, 0, 49, 127, 63);
        pico_ssd1306::fillRect(disp, 3, 52, percentage * (124. / 100.), 60, pico_ssd1306::WriteMode::ADD);

        // format & display battery value
        char buffer[4];
        sprintf(buffer, "%d", percentage);
        pico_ssd1306::drawText(disp, font_16x32, buffer, 0, 16 + digits_offset,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);

        // draw percentage symbol
        pico_ssd1306::drawText(disp, font_8x8, "%", 3, 53,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);

        disp->sendBuffer();
    }

    /// @brief USB-A port status update/redraw, height of an element is 24 pixels
    /// @param port_name short port name (6 char max)
    /// @param pos vertical position of UI element
    /// @param charging_mode integer value of PD_Charging  
    // TODO: Replace with enum
    void Display::port_status(const char* port_name, int pos, ChargingModes charging_mode) {
        if (is_msg_displaying()) { return; }

        // clear up
        pico_ssd1306::fillRect(disp, pos, 0, pos + 27, 48,
            pico_ssd1306::WriteMode::SUBTRACT);

        pico_ssd1306::drawText(disp, font_8x8, port_name, pos + 10, 8,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);

        const char* mode_text;

        switch (charging_mode) {
        case ChargingModes::Unknown: mode_text = "ERROR "; break;
        case ChargingModes::GEN_5v: mode_text = "GEN 5v"; break;
        case ChargingModes::QC_5v: mode_text = "QC  5v"; break;
        case ChargingModes::QC_9v: mode_text = "QC  9v"; break;
        case ChargingModes::QC_15v: mode_text = "QC 12v"; break;
        case ChargingModes::QC_20v: mode_text = "QC 20v"; break;
        case ChargingModes::QC_Var: mode_text = "QC Var"; break;
        case ChargingModes::PD_5v: mode_text = "PD  5v"; break;
        case ChargingModes::PD_9v: mode_text = "PD 9v"; break;
        case ChargingModes::PD_12v: mode_text = "PD 12v"; break;
        case ChargingModes::PD_20v: mode_text = "PD 20v"; break;
        case ChargingModes::PD_Var: mode_text = "PD Var"; break;
        default: mode_text = "??????"; break;
        }

        printf("updating charging mode to: %s\n", mode_text);

        drawText(disp, font_5x8, mode_text, pos, 9,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);

        disp->sendBuffer();
    }

    /// @brief redraw charging mode of 1st (higher) USB-A Port on the display
    /// @param charging_mode PD_Mode or QC_Mode enum!
    void Display::update_port_a(int charging_mode) {
        if (is_msg_displaying()) { return; }

        ChargingModes c;
        c = ChargingModes(charging_mode);

        port_status("USB1", 105, c);
    }

    /// @brief redraw charging mode of 2nd (lower) USB-A Port on the display
    /// @param charging_mode PD_Mode or QC_Mode enum!
    void Display::update_port_b(int charging_mode) {
        if (is_msg_displaying()) { return; }

        ChargingModes c;
        c = ChargingModes(charging_mode);

        port_status("USB2", 105 - 32, c);
    }

    /// @brief redraw charging mode of Type-C Port on the display
    /// @param charging_mode PD_Mode or QC_Mode enum!
    void Display::update_port_c(int charging_mode) {
        if (is_msg_displaying()) { return; }

        ChargingModes c;
        c = ChargingModes(charging_mode);

        port_status("USBC", 105 - 64, c);
    }

    /// @brief displaying warning on a screen non-blocking style
    /// @param msg message to display
    /// @param details longer char[] of details bellow

    /// @brief clear up & display msg filling screen
    /// @param heading big characters on top, max is 4
    /// @param msg the message
    /// @param details details of message
    void Display::display_msg(const char* heading, const char* msg, const char* details[32]) {
        printf("displaying a warning: %s\n", msg);

        disp->clear();

        pico_ssd1306::drawText(disp, font_12x16, heading, 108, 0,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);


        // draw message
        pico_ssd1306::drawText(disp, font_5x8, msg, 108 - 16, 0,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);


        // draw details
        int idx = 0;
        while (details[idx] != 0x0) {
            pico_ssd1306::drawText(disp, font_5x8, details[idx], 108 - 32 - (9 * idx), 0,
                pico_ssd1306::WriteMode::ADD,
                pico_ssd1306::Rotation::deg90);


            idx++;
        }
        disp->sendBuffer();

        msg_timestamp = time_us_64() + msg_wait_time;
    }

    /// @brief Chopping const char* into string array with max length of each entry <= line_width, word endings are preserved.
    /// Exception: If single word is longer than line_width then it's not chopped. 
    /// Used to map words to fixed width screen.
    /// @param cstr const char* to map
    /// @param line_width max length of one entry in array
    /// @return array of strings, each string length will not exceed line_width 
    std::vector<std::string> Display::cstr_to_lines(const char* cstr, int line_width) {
        // i don't understand anything now
        std::vector<std::string> words{};
        std::string str = cstr;

        str.append(" END");

        // remove all spaces from a string and build string array of words
        size_t pos = 0;
        while ((pos = str.find(" ")) != std::string::npos) {
            words.push_back(str.substr(0, pos));
            str.erase(0, pos + 1);
        }
        // cause i'm stupid
        words.push_back(" ");
        std::vector<std::string> result{};

        size_t ptr = 0;
        std::string buf;
        while (ptr < words.size() - 1) {
            // build element in array if it's len is <= line_width
            size_t temp_ptr = ptr;
            while (buf.length() + words[ptr].length() <= line_width) {
                // add spaces between words
                if (buf.length() != 0) {
                    buf.append(" ");
                }
                buf.append(words[ptr]);
                ptr++;
            }
            if (temp_ptr == ptr && ptr != words.size()) {
                buf = words[ptr];
                ptr++;
            }

            result.push_back(buf);
            buf.clear();

        }
        return result;
    }
    /// @brief display warning on screen
    /// @param msg warning 
    /// @param details array of lines
    void Display::warning(const char* msg, const char* details[32]) {
        current_state = DisplayState::DISPLAY_WARNING;
        display_msg("WARN", msg, details);
    }

    /// @brief display error on screen
    /// @param msg warning 
    /// @param details array of lines
    void Display::error(const char* msg, const char* details[32]) {
        current_state = DisplayState::DISPLAY_ERROR;
        display_msg("ERR", msg, details);
    }
};

