#include "SeeedNrf52480Battery.h"
#include "SeeedNrf52480Battery.h"

/// @brief initialize the battery class
/// @param disableVoltageReading disable the reading of the voltage, defaults to false
/// @param useP0_31 use P0_31 instead of PIN_VBAT for reading the voltage, with certain firmware combinations this can lead to different, possibly more accurate results
SeeedNrf52480Battery::SeeedNrf52480Battery(bool disableVoltageReading, bool useP0_31){
    this->voltageReadingDisabled = disableVoltageReading;
    if(useP0_31){
        this->adcInputPin = P0_31;
    }

    if(!disableVoltageReading){
        enableVoltageReading();
        //read first sample, also because Arduino's analog read might need a first run to initialize the ADC
        this->updateADCReading();
    }

    pinMode(PIN_BATTERY_CURRENT_PIN, OUTPUT);
    pinMode(PIN_CHARGING_INV, INPUT);
    this->setChargeCurrent50mA();
}


/// @brief tell if the battery is charging
/// @return true if charging, false if not charging
bool SeeedNrf52480Battery::isCharging()
{
    return !digitalRead(PIN_CHARGING_INV);
}

/// @brief enable the voltage reading feature
void SeeedNrf52480Battery::enableVoltageReading(){
    // Set the analog reference to 2.4V
    analogReference(this->adcReferenceMode);
    // default 10uS
    analogAcquisitionTime(AT_40_US); 
    //set to 12-bit input mode
    analogReadResolution(12);
    //enable the adc
    pinMode(PIN_VBAT_ENABLE, OUTPUT);
    digitalWrite(PIN_VBAT_ENABLE, LOW); // VBAT read enable
}

void SeeedNrf52480Battery::disableVoltageReading()
{
    //currently not implented, because I'm unsure; my idea is the proper way
    //When P0.14 (D14) turns off the ADC function at a high level of 3.3V, 
    //P0.31 will be at the input voltage limit of 3.6V. There is a risk of burning out the P0.31 pin.
    //Currently for this issue, we recommend that users do not turn off the 
    //ADC function of P0.14 (D14) or set P0.14 (D14) to high during battery charging.
    
    /*pinMode(adcInputPin, OUTPUT);
    digitalWrite(adcInputPin, HIGH);*/
}

/// @brief set the maximum battery charge current to 100mA
void SeeedNrf52480Battery::setChargeCurrent100mA(){
    digitalWrite(PIN_BATTERY_CURRENT_PIN, LOW);
}

/// @brief set the maximum battery charge current to 50mA
void SeeedNrf52480Battery::setChargeCurrent50mA(){
    digitalWrite(PIN_BATTERY_CURRENT_PIN, HIGH);
}


/// @brief retrieve filtered battery voltage
/// @return smoothed battery voltage value
float SeeedNrf52480Battery::getVoltage(){
    this->updateADCReading();
    float adcValue = this->batteryADCRollingAvg;
    //turn raw adc values into voltage (12 bits ADC, hence/4096)
    return (this->voltageDividerRatio * this->referenceVoltage * adcValue) / 4096;
}

/// @brief retrieve the battery percentage
/// @return linearly estimated battery level in percent
float SeeedNrf52480Battery::getPercentage(){
    return (this->getVoltage()-minVoltage)/(maxVoltage - minVoltage) * 100;
}

/// @brief setting for internal voltage divider
/// @param voltageDividerRatio ratio, formula is: 1.0 +  R16 (nominally 1MOhm +- 1%)  / R17 (nominally 510kOhm +-1%)
void SeeedNrf52480Battery::setVoltageDividerRatio(float voltageDividerRatio){
    this->voltageDividerRatio = voltageDividerRatio;
}

/// @brief setting for the internal voltage divider, can be used if you measured the voltage divider bridge or have a good reference from e.g. a multimeter
/// @return voltage divider ratio: 1.0 +  R16 (nominally 1MOhm +- 1%)  / R17 (nominally 510kOhm +-1%), default 1+ 1 000 000/500 000 = 2.960784314
float SeeedNrf52480Battery::getVoltageDividerRatio(){
    return this->voltageDividerRatio;
}

/// @brief read new analogue value from the ADC
/// @return  number of total collected samples, capped at adcSampleSize
int SeeedNrf52480Battery::updateADCReading(){
    float newVal = analogRead(adcInputPin);
    if(this->collectedADCSamples >= this->adcSampleSize){ 
        this->batteryADCRollingAvg -= this->batteryADCRollingAvg / this->adcSampleSize;
    }else{
        this->collectedADCSamples++;
    }
    this->batteryADCRollingAvg += newVal / this->adcSampleSize;
    return this->collectedADCSamples;
}

/// @brief adjust the fully charged voltage of your battery; used for percentage calculation ONLY, this does not set charge limits
/// @param maxVolts voltage in volts
void SeeedNrf52480Battery::setMaxVoltage(float maxVolts)
{
    this->maxVoltage = maxVolts;
}

/// @brief adjust the discharged voltage of your battery; used for percentage calculation ONLY, this does not set discharge limits
/// @param minVolts voltage in volts
void SeeedNrf52480Battery::setMinVoltage(float minVolts)
{
    this->minVoltage = minVolts;
}
