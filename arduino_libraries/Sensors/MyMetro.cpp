#include "MyMetro.h"

MyMetro::MyMetro(unsigned long period){
  this->setPeriod(period);
}

void MyMetro::add(unsigned long ms){
  this->myMillis += ms;
  this->lastMillis = millis();
}


boolean MyMetro::check(){
  long elapsed = millis() - this->lastMillis;
  if (elapsed < 0){
    Serial.print("MyMetro wrap around:");
    Serial.println(elapsed);
    this->myMillis = 0xFFFFFFFF - this->lastMillis + millis(); // wrap around
  } else {
    this->myMillis += elapsed;
  }
  this->lastMillis = millis();
  
  if (this->myMillis > (this->last + this->period)){
    this->last = this->myMillis;
    return true;
  } else return false;
}

void MyMetro::setPeriod(unsigned long period){
  this->period = period;
  this->myMillis = millis();
  this->lastMillis = millis();
  this->last = millis();
}