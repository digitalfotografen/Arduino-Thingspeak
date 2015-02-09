/*
* Base class for all sensors.
*/

#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Statistic.h>
#include <MultiLog.h>

enum SensorValue {SENSOR_LAST, SENSOR_AVERAGE, SENSOR_MIN, SENSOR_MAX};

static boolean sensorPower = false;

class Sensor {
  public:
    Sensor(const char *_label = NULL);
    
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
    
    /* Measure and store value
    */
    virtual void putValue(int number);
    virtual void putValue(float number);

    /* get last value
    */
    virtual float getLast();
    
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

    char *getLabel();
    void setLabel(const char *buff);

    void getLabelMin(char *buff);
    void setLabelMin(const char *buff);
    void getLabelMax(char *buff);
    void setLabelMax(const char *buff);

    /*Set output range for mapping av measured value
    */
    void setRangeOut(float rangeMin, float rangeMax);
    /*Set input range for mapping av measured value
    */
    void setRangeIn(float rangeMin, float rangeMax);
    
    
    static int sensorPowerPin;
  
  protected:
    char label[8];
    char labelMax[8];
    char labelMin[8];
    unsigned long prepareTime;
    float last;
    Statistic statistic;
    float rangeOutMin;
    float rangeOutMax;
    float rangeInMin;
    float rangeInMax;
};

#endif
