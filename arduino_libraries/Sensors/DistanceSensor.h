#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

#include "Sensor.h"
#include "Arduino.h"


class DistanceSensor : virtual public Sensor {
  public: 
    DistanceSensor(char *_label,
                  int _inputPinNumber,
                  int _errorPinNumber,
                  int _laserOnPinNumber,
                  float _rangeMin, 
                  float _rangeMax,
                  float _adcMin = 0, 
                  float _adcMax = 1023);

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
    float rangeMin;
    float rangeMax;
    float adcMin;
    float adcMax;
};

#endif

