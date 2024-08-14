#include "SeeedNrf52480Battery.h"

SeeedNrf52480Battery::SeeedNrf52480Battery(bool disableVoltageReading, bool useP0_31){
    this->disableVoltageReading = disableVoltageReading;
    if(useP0_31){
        this.ADCInputPin = P0_31;
    }

    if(!disableVoltageReading){
        enableVoltageReading();
        //read first sample, also because Arduino's analog read might need a first run to initialize the ADC
    }

    pinMode(PIN_BATTERY_CURRENT_PIN, OUTPUT);
    pinMode(PIN_CHARGING_INV, INPUT);
    this->setChargeCurrent50mA();
    attachInterrupt(digitalPinToInterrupt(PIN_CHARGING_INV), ISR, CHANGE);
}

bool SeeedNrf52480Battery::isCharging(){
    return !digitalRead(PIN_CHARGING_INV);
}

int SeeedNrf52480Battery::currentChargeStateSinceMs(){
    return millis()-lastChargingPeak;
}

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

void SeeedNrf52480Battery::chargingInterrupt()
{
    this->lastChargingPeak = millis();
}

void SeeedNrf52480Battery::disableVoltageReading()
{
    //currently not implented, because I'm unsure my idea is the proper way
    //When P0.14 (D14) turns off the ADC function at a high level of 3.3V, 
    //P0.31 will be at the input voltage limit of 3.6V. There is a risk of burning out the P0.31 pin.
    //Currently for this issue, we recommend that users do not turn off the 
    //ADC function of P0.14 (D14) or set P0.14 (D14) to high during battery charging.
    
    /*pinMode(adcInputPin, OUTPUT);
    digitalWrite(adcInputPin, HIGH);*/
}

void SeeedNrf52480Battery::setChargeCurrent100mA(){
    digitalWrite(PIN_BATTERY_CURRENT_PIN, LOW);
}

void SeeedNrf52480Battery::setChargeCurrent50mA(){
    digitalWrite(PIN_BATTERY_CURRENT_PIN, HIGH);
}


float SeeedNrf52480Battery::getVoltage(){
    float adcValue = this->batteryADCRollingAvg;
    //turn raw adc values into voltage (12 bits ADC, hence/4096)
    return (this->voltageDividerRatio * this->referenceVoltage * adcValue) / 4096;
}

float SeeedNrf52480Battery::getPercentage(){
    return (this->getBatteryVoltage()-minVoltage)/(maxVoltage - minVoltage) * 100;
}

void SeeedNrf52480Battery::setVoltageDividerRatio(float voltageDividerRatio){
    this->voltageDividerRatio = voltageDividerRatio;
}

float SeeedNrf52480Battery::getVoltageDividerRatio(){
    return this->voltageDividerRatio;
}

int SeeedNrf52480Battery::updateVoltageReading(){
    float newVal = analogRead(adcInputPin);
    if(this.collectedADCSamples >= this->adcSampleSize){ 
        this->batteryADCRollingAvg -= this->batteryADCRollingAvg / this->adcSampleSize;
    }else{
        this.collectedADCSamples++;
    }
    this->batteryADCRollingAvg += newVal / this->adcSampleSize;
    return this->collectedADCSamples;
}
