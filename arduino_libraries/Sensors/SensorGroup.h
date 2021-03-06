
#ifndef SENSOR_GROUP_H
#define SENSOR_GROUP_H

#define MAX_SENSORS 10

#include "Sensor.h"
#include "Arduino.h"
#include "MultiLog.h"


class SensorGroup {
  public:
    SensorGroup();
    void addSensor(Sensor *sensor);
    //virtual void update();
    virtual void toString(char *buff);
    unsigned long prepare(); // prepare sensors for measurement, returns ms until ready
    void measureAll();
    void sleep();
    void clear();
    
  protected:
    //unsigned long lastUpdate; // millis for last successfull update
    //unsigned long period; // update period in millis
    //int failCounter; // number of failed updates
    Sensor* sensors[MAX_SENSORS];
    int numberOfSensors;
};

#endif
