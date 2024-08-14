//todo: put this note from https://wiki.seeedstudio.com/XIAO_BLE/ Question Q3 into the compilation warning: 
// When P0.14 (D14) turns off the ADC function at a high level of 3.3V, P0.31 will be at the input voltage limit of 3.6V. There is a risk of burning out the P0.31 pin.
// Currently for this issue, we recommend that users do not turn off the ADC function of P0.14 (D14) or set P0.14 (D14) to high during battery charging.

#ifndef SEEDNRF52480BATTERY_H
#define SEEDNRF52480BATTERY_H

#define PIN_BATTERY_CURRENT_PIN P0_13
#define PIN_CHARGING_INV P0_17

#include <Arduino.h>

class SeeedNrf52480Battery {
    public:

        SeeedNrf52480Battery(bool disableVoltageReading = false, bool useP0_31 = false);

        bool isCharging();
        void setChargeCurrent100mA();
        void setChargeCurrent50mA();
        
        void enableVoltageReading();
        void setSampleSize(uint8_t adcSampleSize);

        float getVoltage();
        float getPercentage();
        void setVoltageDividerRatio(float voltageDividerRatio);
        float getVoltageDividerRatio();
        int updateADCReading();

    private:

        void chargingInterrupt();
        void disableVoltageReading();
        //battery performance
        //according to the texas instruments BQ25100 documentation 4.2V is the maximum charge voltage
        const float maxVoltage = 4.2;
        //3.2V is just a reasonable discharged voltage for Lithium-(X) cells
        float minVoltage = 3.2;

        //number of samples of adc already collected
        int collectedADCSamples = 0;
        
        //rolling average of inputs
        float batteryADCRollingAvg = 0;

        //number of samples used to smooth fluctuations
        uint8_t adcSampleSize = 10;

        //disable ADC and voltage pin
        bool voltageReadingDisabled = false;

        //pin used to read input voltage
        uint8_t adcInputPin = PIN_VBAT;

        //ratio of Voltage divider: 1.0 +  R16 (nominally 1MOhm +- 1%)  / R17 (nominally 510kOhm +-1%), default 1+ 1 000 000/500 000 = 2.960784314
        float voltageDividerRatio = 2.960784314;

        //reference Voltage for calculations
        float referenceVoltage = 2.40;

        //refernce voltage setting of ADC
        uint8_t adcReferenceMode = AR_INTERNAL2V4;
};

#endif