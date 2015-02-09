#include "DistanceSensor.h"
#include "Arduino.h"


DistanceSensor::DistanceSensor(char *_label, 
                              int _inputPinNumber,  
                              int _errorPinNumber,
                              int _laserOnPinNumber) : Sensor(_label){
  inputPin = _inputPinNumber;
  this->errorPin = _errorPinNumber;
  this->laserOnPin = _laserOnPinNumber;
  pinMode(this->inputPin, INPUT);
  pinMode(this->errorPin, INPUT_PULLUP);
  pinMode(this->laserOnPin, OUTPUT);
  digitalWrite(this->laserOnPin, LOW);
  this->setPrepareTime(1000);
  //keep_ADCSRA = ADCSRA;
}

void DistanceSensor::measure(){
  mlog.DEBUG(F("DistanceSensor measure:"));
  mlog.DEBUG(label, false);
  boolean error = !digitalRead(this->errorPin);
  if (!error){
    int value = 0;
    // make 20 samples and average as a low pass filter
    for (int i = 0; i < 20; i++){
      value += analogRead(this->inputPin);
    }
    float fValue = float(value) / 20;
    mlog.DEBUG(F(" in:"), false);
    mlog.DEBUG(fValue);
    fValue = this->map(fValue, 
                              this->rangeInMin,
                              this->rangeInMax, 
                              this->rangeOutMin, 
                              this->rangeOutMax);
    mlog.DEBUG(F(" out:"), false);
    mlog.DEBUG(fValue);
    this->putValue( fValue );
  } else {
    mlog.WARNING(F("SENSOR ERROR: "));
    mlog.WARNING(label, false);
  }
}

unsigned long DistanceSensor::prepare(){
  mlog.DEBUG("DistanceSensor prepare:");
  mlog.DEBUG(this->label, false);
  mlog.INFO("Laser on");
  digitalWrite(this->laserOnPin, HIGH);
  unsigned long t = Sensor::prepare();
  analogRead(this->inputPin); 
  return max(t, this->prepareTime); // Laser sensor needs 5000 ms
}

void DistanceSensor::sleep(){
   mlog.INFO("Laser off");
  digitalWrite(this->laserOnPin, LOW);
  Sensor::sleep();
}