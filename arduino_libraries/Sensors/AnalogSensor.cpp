#include "AnalogSensor.h"
#include "Arduino.h"


AnalogSensor::AnalogSensor(char *_label, int _adcPinNumber, float _rangeMin, float _rangeMax) : Sensor(_label){
  this->pin = _adcPinNumber;
  this->rangeMin = _rangeMin;
  this->rangeMax = _rangeMax;
  pinMode(pin, INPUT);
  //keep_ADCSRA = ADCSRA;
}

void AnalogSensor::measure(){
  Serial.print("AnalogSensor measure ");
  Serial.print(label);
  Serial.print(": ");
  int value = analogRead(pin);
  Serial.println(value);
  this->statistic.add( this->map(float(value), 0, 1023, this->rangeMin, this->rangeMax) );
}

unsigned long AnalogSensor::prepare(){
  unsigned long t = 0;
  return t +   Sensor::prepare();
}

void AnalogSensor::sleep(){
  Sensor::sleep();
}