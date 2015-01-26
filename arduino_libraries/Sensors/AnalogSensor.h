#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "Sensor.h"
#include "Arduino.h"
#include "MultiLog.h"

static byte keep_ADCSRA; // ADC power control


class AnalogSensor : virtual public Sensor {
  public: 
    AnalogSensor(char *_label, int _adcPinNumber);

    /* prepare sensor for measurement, turn on power
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    virtual void measure();

    /* Powersave
    */
    virtual void sleep();
    
  private:
    int pin;
};

#endif

