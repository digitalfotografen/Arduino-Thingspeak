#include "SensorGroup.h"

SensorGroup::SensorGroup(){
  //this->lastUpdate = 0;
  //this->period = 1000;
  //this->failCounter = 0;
  this->numberOfSensors = 0;
}

void SensorGroup::addSensor(Sensor *sensor){
  this->sensors[numberOfSensors++] = sensor;
}

void SensorGroup::measureAll(){
  if (numberOfSensors > 0){
    for (int i = 0; i < numberOfSensors; i++){
      sensors[i]->measure();
    }
  } else {
    Serial.println("ERROR: SensorGroup:measureAll There are no sensors");
  }
}

unsigned long SensorGroup::prepare(){
  unsigned long maxTime = 0;
  if (numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      maxTime = max(maxTime, this->sensors[i]->prepare());
    }
    return maxTime;
  } else {
    Serial.println("ERROR: SensorGroup:prepare There are no sensors");
    return 0;
  }  
}

void SensorGroup::sleep(){
  unsigned long maxTime = 0;
  if (this->numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      this->sensors[i]->sleep();
    }
  } else {
    Serial.println("ERROR: SensorGroup:prepare There are no sensors");
  }  
}

void SensorGroup::clear(){
  unsigned long maxTime = 1;
  if (this->numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      this->sensors[i]->clear();
    }
  } else {
    Serial.println("ERROR: SensorGroup:prepare There are no sensors");
  }  
}

void SensorGroup::toString(char* buff){
}
