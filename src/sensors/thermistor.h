#pragma once

#include <vector>
#include <stdio.h>
#include <algorithm>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"


// Stats for used 100kOhm K3950 thermsitors
// R0 100000            // Nominal resistance at 25⁰C
// T0 25       	        // Temperature for nominal resistance
// samplingrate 10      // Number of samples
// beta 3950. 		    // The beta coefficient
// Rref 100000  	    // Value of  resistor used for the voltage divider

namespace sensors {

#define ADC_CONVERSION_FACTOR (3.3f / (1 << 12)); // for 3.3v load in 12 bits
    /// thermistor object, only NTC are supported
    class Thermistor {
    private:
        /// @brief GPIO pin number
        uint8_t pin;
        /// @brief adc number
        uint8_t adc;

        /// @param R0 base resistance of thermistor at T0 (25C)
        /// @param R1 resistance of  
        /// @param beta the beta coefficient
        double R0, R1, beta;
        std::vector<int> samples;

        /// @brief sum for averaging
        double sum;

    public:
        /// @brief main constructor
        /// @param pin pin number, GPIO 26-29 are available
        /// @param adc adc number 0-4
        /// @param R0 base resistance of thermistor at T0 (25C)
        /// @param R1 resistance of  
        /// @param beta the beta coefficient
        Thermistor(uint8_t pin, uint8_t adc,
            double R0, double  R1,
            double  beta);

        /// @brief get averaged temperature based on 10 last readings
        /// @param num_samples number of samples for averaging
        /// @return averaged temperature
        double get_average(int num_samples);

        /// @brief get current temperature at thermistor
        /// @return current temperature in C
        double get();
    };
};