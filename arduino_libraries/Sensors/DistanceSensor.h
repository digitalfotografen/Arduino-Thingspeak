#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include "Sensor.h"
#include "Arduino.h"
#include "MultiLog.h"


class DistanceSensor : virtual public Sensor {
  public: 
    DistanceSensor(char *_label,
                  int _inputPinNumber,
                  int _errorPinNumber,
                  int _laserOnPinNumber);

    /* prepare sensor for measurement, turn on laser
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    virtual void measure();

    /* Powersave
    */
    virtual void sleep();
    
  private:
    int inputPin;
    int errorPin;
    int laserOnPin;
};

#endif

