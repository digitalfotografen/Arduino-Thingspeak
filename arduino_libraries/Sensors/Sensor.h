/*
* Base class for all sensors.
*/

#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Statistic.h>

enum SensorValue {SENSOR_AVERAGE, SENSOR_MIN, SENSOR_MAX};

static boolean sensorPower = false;

class Sensor {
  public:
    Sensor(char *_label);
    
    /* prepare sensor for measurement, turn on power
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    /* set time in ms to wait between prepare an measure
    * default 0
    */
    void setPrepareTime(unsigned long t); 

    /* Measure and store value in statistics
    */
    virtual void measure();
    
    /* Powersave
    */
    virtual void sleep();
    
    /*Return value as char string
    */
    virtual void toString(char *buff, int maxLength, SensorValue valueType);

    /* Clear statistics
    */
    void clear();

    /* Scale sensor values
    *
    */
    float map(float x, float in_min, float in_max, float out_min, float out_max);

    char* trim(char *str);

    void getLabel(char *buff);

    void getLabelMin(char *buff);
    void setLabelMin(char *buff);
    void getLabelMax(char *buff);
    void setLabelMax(char *buff);

    static int sensorPowerPin;
  
  protected:
    char label[10];
    char labelMax[10];
    char labelMin[10];
    unsigned long prepareTime;
    Statistic statistic;
};

#endif
