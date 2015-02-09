#include "SensorGroup.h"

SensorGroup::SensorGroup(){
  //this->lastUpdate = 0;
  //this->period = 1000;
  //this->failCounter = 0;
  this->numberOfSensors = 0;
}

void SensorGroup::addSensor(Sensor *sensor){
  if (this->numberOfSensors < MAX_SENSORS){
    this->sensors[this->numberOfSensors++] = sensor;
  } else {
    mlog.CRITICAL(F("SensorGroup::addSensor tried to add to many sensors"));
  }
}

void SensorGroup::measureAll(){
  if (numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      sensors[i]->measure();
    }
  } else {
     mlog.WARNING(F("SensorGroup:measureAll There are no sensors"));
  }
}

unsigned long SensorGroup::prepare(){
  unsigned long maxTime = 0;
  unsigned long t = 0;
  if (numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      t = this->sensors[i]->prepare();
      maxTime = max(maxTime, t);
    }
    return maxTime;
  } else {
    mlog.WARNING(F("SensorGroup:prepare There are no sensors"));
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
    mlog.WARNING(F("ERROR: SensorGroup:prepare There are no sensors"));
  }  
}

void SensorGroup::clear(){
  unsigned long maxTime = 1;
  if (this->numberOfSensors > 0){
    for (int i = 0; i < this->numberOfSensors; i++){
      this->sensors[i]->clear();
    }
  } else {
    mlog.WARNING(F("ERROR: SensorGroup:prepare There are no sensors"));
  }  
}

void SensorGroup::toString(char* buff){
}
