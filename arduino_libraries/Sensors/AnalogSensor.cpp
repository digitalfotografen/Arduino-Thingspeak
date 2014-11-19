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
  int value = analogRead(this->pin);
  Serial.print(value);
  Serial.print(" = ");
  float fValue = this->map(float(value), 0, 1023, this->rangeMin, this->rangeMax);
  Serial.println(fValue);
  this->statistic.add( fValue );
}

unsigned long AnalogSensor::prepare(){
  Serial.print("AnalogSensor prepare ");
  Serial.println(this->label);
  unsigned long t = Sensor::prepare();
  analogRead(this->pin); 
  return t;
}

void AnalogSensor::sleep(){
  Sensor::sleep();
}