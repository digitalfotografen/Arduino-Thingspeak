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
  if (!analogPower & (analogPowerPin >= 0)){
    digitalWrite(analogPowerPin, HIGH);
    t+= 50; // time until distance sensors stable
    analogPower = true;
  }
  
  if (ADCSRA == 0){
    //ADCSRA = keep_ADCSRA;
    t += 5;
  }
  return t;
}

void AnalogSensor::sleep(){
  if (analogPower & (analogPowerPin >= 0)){
    digitalWrite(analogPowerPin, LOW);
    analogPower = false;
  }
  
  //ADCSRA = 0;
}