#include "DistanceSensor.h"
#include "Arduino.h"


DistanceSensor::DistanceSensor(char *_label, 
                              int _inputPinNumber,  
                              int _errorPinNumber,
                              int _laserOnPinNumber,
                              float _rangeMin, 
                              float _rangeMax,
                              float _adcMin,
                              float _adcMax) : Sensor(_label){
  inputPin = _inputPinNumber;
  this->errorPin = _errorPinNumber;
  this->laserOnPin = _laserOnPinNumber;
  this->rangeMin = _rangeMin;
  this->rangeMax = _rangeMax;
  this->adcMin = _adcMin;
  this->adcMax = _adcMax;
  pinMode(this->inputPin, INPUT);
  pinMode(this->errorPin, INPUT_PULLUP);
  pinMode(this->laserOnPin, OUTPUT);
  digitalWrite(this->laserOnPin, LOW);
  this->setPrepareTime(1000);
  //keep_ADCSRA = ADCSRA;
}

void DistanceSensor::measure(){
  Serial.print("DistanceSensor measure ");
  Serial.print(label);
  boolean error = !digitalRead(this->errorPin);
  if (!error){
    int value = analogRead(this->inputPin); // medianvärdet
    Serial.print(value);
    Serial.print(" = ");
    float fValue = this->map(float(value), 
                              this->adcMin,
                              this->adcMax, 
                              this->rangeMin, 
                              this->rangeMax);
    Serial.println(fValue);
    this->statistic.add( fValue );
  } else {
    Serial.println(": SENSOR ERROR!");
  }
}

unsigned long DistanceSensor::prepare(){
  Serial.print("DistanceSensor prepare ");
  Serial.println(this->label);
  Serial.println("Laser on");
  digitalWrite(this->laserOnPin, HIGH);
  unsigned long t = Sensor::prepare();
  analogRead(this->inputPin); 
  return max(t, this->prepareTime); // Laser sensor needs 5000 ms
}

void DistanceSensor::sleep(){
  Serial.println("Laser off");
  digitalWrite(this->laserOnPin, LOW);
  Sensor::sleep();
}