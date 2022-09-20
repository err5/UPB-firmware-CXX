#include "display_controller.h"

using namespace  display_controller;

Display::Display(pico_ssd1306::SSD1306* display_driver, int msg_length) {
    disp = display_driver;
    current_state = DisplayState::MAIN_MENU;
    disp->setOrientation(1);
    disp->clear();
    last_msg_timestamp = 0;
    msg_wait_time = msg_length * 1000000;
};

bool Display::is_msg_displaying() {
    if (last_msg_timestamp >= time_us_64()) {
        printf("tried to redraw display while message is shown\n");
        return true;
    }
    else {
        return false;
    }
}

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
        pico_ssd1306::WriteMode::INVERT,
        pico_ssd1306::Rotation::deg90);

    disp->sendBuffer();
}

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

void Display::update_port_a(int charging_mode) {
    if (is_msg_displaying()) { return; }

    ChargingModes c;
    c = ChargingModes(charging_mode);

    port_status("USB1", 105, c);
}

void Display::update_port_b(int charging_mode) {
    if (is_msg_displaying()) { return; }

    ChargingModes c;
    c = ChargingModes(charging_mode);

    port_status("USB2", 105 - 32, c);
}

void Display::update_port_c(int charging_mode) {
    if (is_msg_displaying()) { return; }

    ChargingModes c;
    c = ChargingModes(charging_mode);

    port_status("USBC", 105 - 64, c);
}

void Display::display_msg(const char* heading, const char* msg, const char* details) {
    printf("displaying a message: %s\n", msg);

    disp->clear();

    pico_ssd1306::drawText(disp, font_12x16, heading, 108, 0,
        pico_ssd1306::WriteMode::ADD,
        pico_ssd1306::Rotation::deg90);


    // draw message
    pico_ssd1306::drawText(disp, font_5x8, msg, 108 - 16, 0,
        pico_ssd1306::WriteMode::ADD,
        pico_ssd1306::Rotation::deg90);


    std::vector<std::string> details_lines = cstr_to_lines(details, 12);


    // draw details
    int idx = 0;

    while (idx < details_lines.size() - 1) {
        pico_ssd1306::drawText(disp, font_5x8, details_lines[idx].c_str(), 108 - 32 - (9 * idx), 0,
            pico_ssd1306::WriteMode::ADD,
            pico_ssd1306::Rotation::deg90);

        idx++;
    }
    disp->sendBuffer();

    last_msg_timestamp = time_us_64() + msg_wait_time;
}


std::vector<std::string> Display::cstr_to_lines(const char* cstr, int line_width) {
    // i don't understand anything now
    std::vector<std::string> words{};
    std::string str = cstr;

    // remove all spaces from a string and build string array of words
    size_t pos = 0;
    while ((pos = str.find(" ")) != std::string::npos) {
        words.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
    }

    std::vector<std::string> result{};

    size_t ptr = 0;
    std::string buf;
    while (ptr < words.size()) {
        // build element in array if it's len is <= line_width
        size_t temp_ptr = ptr;
        while (buf.length() + words[ptr].length() <= line_width && ptr < words.size()) {
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

void Display::warning(const char* msg, const char* details) {
    current_state = DisplayState::DISPLAY_WARNING;
    display_msg("WARN", msg, details);
}

void Display::error(const char* msg, const char* details) {
    current_state = DisplayState::DISPLAY_ERROR;
    display_msg("ERR", msg, details);
}

