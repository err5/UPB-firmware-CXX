#include "quick_charge.h"

using namespace charging_protocols;

DigitalPin::DigitalPin(uint8_t high, uint8_t low) {
    _high = high;
    _low = low;
    gpio_init(_high);
    gpio_init(_low);
    // setting up pulls, else it won't work
    gpio_pull_down(_high);
    gpio_pull_down(_low);
}

void DigitalPin::set_hiz() {
    gpio_set_dir(_high, false);
    gpio_set_dir(_low, false);
}

void DigitalPin::set_0v() {
    gpio_set_dir(_high, true);
    gpio_set_dir(_low, true);
    gpio_put(_high, false);
    gpio_put(_low, false);
}

void DigitalPin::set_600mv() {
    gpio_set_dir(_high, true);
    gpio_set_dir(_low, true);
    gpio_put(_high, true);
    gpio_put(_low, false);
}

void DigitalPin::set_2700mv() {
    gpio_set_dir(_high, true);
    gpio_set_dir(_low, true);
    gpio_put(_high, false);
    gpio_put(_low, true);
}

void DigitalPin::set_3300mv() {
    gpio_set_dir(_high, true);
    gpio_set_dir(_low, true);
    gpio_put(_high, true);
    gpio_put(_low, true);
}

bool DigitalPin::read_high() {
    return gpio_get(_high);
}

bool DigitalPin::read_low() {
    return gpio_get(_low);
}


QuickChargePort_alt::QuickChargePort_alt(DigitalPin dm, DigitalPin dp) :
    _dm(dm),
    _dp(dp)
{
    _mode = ChargingModes::NotConnected;
}

bool QuickChargePort_alt::output_handshake() {
    return true;
}
void QuickChargePort_alt::begin() {
    _dp.set_hiz();
    _dm.set_hiz();

    _dp.set_3300mv();
    sleep_us(10); // short delay
    if (!_dm.read_high()) {             // are D+ & D- disconnected?
        _dp.set_hiz();
        _qc_input = false;              // adapter is generic 5v
        _mode = ChargingModes::GEN_5v;
        _qc_input = false;
        printf("adapter is not QC2.0+ compliant\n");
        return;
    }

    // QC should be 
    _dp.set_600mv();                    // setting 600mv at D+ for adapter to start handshake
    sleep_ms(QC_T_GLITCH_BC_DONE_MS);   // waiting for adapter to disconnect D+ & D-

    _dp.set_3300mv();                   // setting D+ to 3.3v to check if pins are connected
    sleep_us(10);
    if (!_dm.read_high()) {             // are D+ & D- disconnected?
        this->request(ChargingModes::QC_5v);
        _mode = ChargingModes::QC_5v;
        _qc_input = true;               // adapter has disconnected D+ & D- so QC2.0+ is supported
        printf("adapter is QC2.0+ compliant\n");
        return;
    }

    // after handshake tries D+ & D- are still connected, so it's generic 5V 2A
    _dp.set_hiz();
    _dm.set_hiz();
    _qc_input = false;
    _mode = ChargingModes::GEN_5v;
    printf("adapter is not QC2.0+ compliant\n");
    return;

}
bool QuickChargePort_alt::is_qc() {
    return this->_qc_input;
}

void QuickChargePort_alt::request(ChargingModes mode) {
    if (!_qc_input) {
        printf("tried to set QC2.0+ mode when charger is not QC2.0+ compliant\n");
        return;
    }

    _dp.set_hiz();
    _dm.set_hiz();
    sleep_us(10);
    switch (mode) {
    case ChargingModes::QC_5v: {
        _dp.set_600mv();
        _dm.set_0v();
        _mode = ChargingModes::QC_5v;
        break;
    }
    case ChargingModes::QC_9v: {
        _dp.set_3300mv();
        _dm.set_600mv();
        _mode = ChargingModes::QC_9v;
        break;
    }
    case ChargingModes::QC_12v: {
        _dp.set_600mv();
        _dm.set_600mv();
        _mode = ChargingModes::QC_12v;
        break;
    }
    case ChargingModes::QC_20v: {
        _dp.set_3300mv();
        _dm.set_3300mv();
        _mode = ChargingModes::QC_20v;
        break;
    }
    case ChargingModes::QC_Var: {
        _dp.set_600mv();
        _dm.set_3300mv();
        _mode = ChargingModes::QC_Var;
        break;
    }
    case ChargingModes::NotConnected: {
        printf("tried changing mode while device is disconnected\n");
        break;
    }
    default: printf("unknown charging mode requested, nothing changed\n");
    }

    printf("setting charging mode to %s\n", ChargingModes_string[int(_mode)]);

    // lag before setting pins to hiz
    // apparently voltage should just remain 
    // sleep_ms(QC_T_GLICH_V_CHANGE_MS);
    // _dp.set_0v();
    // _dm.set_0v();
}

/*
       | (D+) | (D-) |   Mode   |
       | 0.6V |  0V  |   5V     |
       | 3.3V | 0.6V |   9V     |
       | 0.6V | 0.6V |   12V    |
       | 3.3V | 3.3V |   20V    |
       | 0.6V | 3.3V | VARIABLE |
*/

/// @brief read input of the USB port to check what QC mode is being requested
/// @param adc_dp ADC that D+ pin is connected to
/// @param adc_dm ADC that D- pin is connected to
/// @return requested charging mode
ChargingModes QuickChargePort_alt::get_charging_mode(uint8_t adc_dp, uint8_t adc_dm) {
    float dp = get_voltage(adc_dp);
    float dm = get_voltage(adc_dm);
    sleep_ms(QC_T_GLITCH_BC_DONE_MS);

    return mode_from_voltage(dp, dm);
}


// TODO: POWER OUTPUT CONTROLLER IS REQUIRED


/// @brief match voltage of pins to voltage
/// @param dp voltage at D+ pin
/// @param dm voltage at D- pin
/// @return mode if it was possible to match
ChargingModes QuickChargePort_alt::mode_from_voltage(float dp, float dm) {
    // TODO: measure voltage on pins when port isn't connected
    if (dp == 0. && dm == 0.) {
        return ChargingModes::NotConnected;
    }

    if (0.2 < dp < 1.) {
        if (dm < .2) {
            return ChargingModes::QC_5v;
        }
        else if (.2 < dm < 1.) {
            return ChargingModes::QC_12v;
        }
        else if (2.9 < dm < 3.6) {
            return ChargingModes::QC_Var;
        }
    }

    else if (2.9 < dp < 3.6) {
        if (.2 < dm < 1.) {
            return ChargingModes::QC_12v;
        }
        else if (2.9 < dm < 3.6) {
            return ChargingModes::QC_20v;
        }
    }
    // couldn't match charging mode 
    return ChargingModes::GEN_5v;
}
