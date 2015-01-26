#include "AnalogSensor.h"
#include "Arduino.h"


AnalogSensor::AnalogSensor(char *_label, int _adcPinNumber) : Sensor(_label){
  this->pin = _adcPinNumber;
  pinMode(pin, INPUT);
  //keep_ADCSRA = ADCSRA;
}

void AnalogSensor::measure(){
  mlog.DEBUG(F("AnalogSensor measure:"));
  mlog.DEBUG(label, false);
  int value = 0; 
  // make 20 samples and average as a low pass filter
  for (int i = 0; i < 20; i++){
    value += analogRead(this->pin);
  }
  float fValue = float(value) / 20;
  mlog.DEBUG(" in:", false);
  mlog.DEBUG(fValue);
  fValue = this->map(fValue, 
                            this->rangeInMin,
                            this->rangeInMax, 
                            this->rangeOutMin, 
                            this->rangeOutMax);
  mlog.DEBUG(" out:", false);
  mlog.DEBUG(fValue);
  this->statistic.add( fValue );
}

unsigned long AnalogSensor::prepare(){
  mlog.DEBUG(F("AnalogSensor prepare:"));
  mlog.DEBUG(this->label, false);
  unsigned long t = Sensor::prepare();
  analogRead(this->pin); 
  return t;
}

void AnalogSensor::sleep(){
  Sensor::sleep();
}