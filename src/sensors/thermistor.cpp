#include "thermistor.h"


using namespace sensors;

Thermistor::Thermistor(uint8_t pin, uint8_t adc,
    double R0, double  R1,
    double  beta) {
    this->pin = pin;
    this->adc = adc;
    this->R0 = R0;
    this->R1 = R1;
    this->beta = beta;
    this->samples = { 1,2,3,4,5,6,7,8,9,0 };
    this->sum = 0;

}


double Thermistor::get_average(int num_samples) {
    samples.resize(num_samples);
    samples.push_back(this->get());
    std::rotate(samples.rbegin(), samples.rbegin() + 1, samples.rend());

    printf("size: %d\n", samples.size());
    for (double d : samples) {
        sum += d;
        printf("d: %.2f\n", d);
    }
    sum /= num_samples;
    printf("sum: %.2f\n", sum);
    return sum;
}

double Thermistor::get() {
    adc_select_input(0);
    int raw = adc_read();
    float voltage = raw * ADC_CONVERSION_FACTOR;
    float resistance = R1 * voltage;
    // beta coefficient equation
    double T = 1. / ((1. / (25 + 273.15)) + (1. / beta) * log(resistance / R0));
    // conversion to celsius + correction
    T -= 273.15;
    printf("resistance: %.2fOhm\n", resistance);
    printf("voltage: %.2fV\n", voltage);
    return T;
}