#ifndef GSM_SENSOR_H
#define GSM_SENSOR_H

#include "Sensor.h"
#include "Arduino.h"

class GsmSensor : virtual public Sensor {
  public: 
    GsmSensor(char *_label);

    /* prepare sensor for measurement, turn on power
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    virtual void measure();

    /* Powersave
    */
    virtual void sleep();
    
    void activate();

    void deactivate();
        
  private:
    boolean active;
};


#endif

