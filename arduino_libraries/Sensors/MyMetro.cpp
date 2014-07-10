#include "MyMetro.h"

MyMetro::MyMetro(unsigned long period){
  this->period = period;
  this->myMillis = millis();
  this->lastMillis = this->myMillis;
  this->last = this->myMillis;
}

void MyMetro::add(unsigned long ms){
  this->myMillis += ms;
}


boolean MyMetro::check(){
  this->myMillis = millis() > this->lastMillis ?
    this->myMillis + millis() - this->lastMillis:
    this->myMillis + 0xFFFFFFFF - this->lastMillis + millis(); // wrap around
  this->lastMillis = millis();
  
  if (this->myMillis >= this->last + this->period){
    this->last += this->period;
    return true;
  } else return false;
}

void MyMetro::setPeriod(unsigned long period){
  this->period = period;
}