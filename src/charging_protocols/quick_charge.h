#pragma once

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#define QC3_MIN_VOLTAGE_MV              3600
#define QC3_CLASS_A_MAX_VOLTAGE_MV      12000
#define QC3_CLASS_B_MAX_VOLTAGE_MV      20000

// timing values for Portable Device are not available, indicative values for a HVDCP charger were taken from the uP7104 datasheet
#define QC_T_GLITCH_BC_DONE_MS          1500
#define QC_T_GLICH_V_CHANGE_MS          60
#define QC_T_ACTIVE_MS                  1
#define QC_T_INACTIVE_MS                1

#define ADC_CONVERSION_FACTOR (3.3f / (1 << 12)); // for 3.3v load in 12 bits

/// @brief Possible configurations for QC both input & output
enum class ChargingModes {
    GEN_5v,
    QC_5v,
    QC_9v,
    QC_12v,
    QC_20v,
    QC_Var,
    NotConnected
};

static const char* ChargingModes_string[] = {
    "GEN_5v",
    "QC_5v",
    "QC_9v",
    "QC_12v",
    "QC_20v",
    "QC_Var",
    "NotConnected"
};

namespace charging_protocols {


    class DigitalPin {
    private:
        uint8_t _high, _low;
    public:
        DigitalPin(uint8_t high, uint8_t low);
        void set_hiz();
        void set_0v();
        void set_600mv();
        void set_2700mv();
        void set_3300mv();

        bool read_high();
        bool read_low();
    };

    class QuickChargePort_alt {
    private:
        DigitalPin _dp, _dm;
        ChargingModes _mode;
        bool _handshake_done, _qc_input, _is_input;
        double _millivolt_estimated;
    public:
        /// @brief main contructor
        /// @param dp DigitalPin that is connected to D+
        /// @param dm DigitalPin that is connected to D-
        QuickChargePort_alt(DigitalPin dp, DigitalPin dm);

        /// @brief commit handshake to input voltage
        void begin();
        /// @brief commit handshake to output voltage
        /// @return is device QC compliant
        bool output_handshake();

        /// @brief request given mode from power adapter
        /// @param mode mode to request
        void request(ChargingModes mode);
        float get_voltage(uint adc);

        /// @brief check if adapter is QC2.0+ compliant
        /// @return type of an adapter, true for QC, false for Generic 
        bool is_qc();

        /// @brief Return mode that matches given voltages
        /// @param dp voltage on D+ pin 
        /// @param dm voltage on D- pin
        /// @return matching QC2.0 mode
        ChargingModes mode_from_voltage(float dp, float dm);

        /// @brief get voltage on the port and return mode that is currently being requested
        /// @param adc_dp ADC connected to D+ pin 
        /// @param adc_dm ADC connected to D- pin
        /// @return 
        ChargingModes get_charging_mode(uint8_t adc_dp, uint8_t adc_dm);


        // TODO: disable output and try setting input to 5v if possible
        void panic();
    };
};