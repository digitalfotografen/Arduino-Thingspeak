#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include "Arduino.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Sensor.h"

// Setup a oneWire instance to communicate with any OneWire devices
static OneWire oneWire(8);

// Pass our oneWire reference to Dallas Temperature. 
static DallasTemperature dallasSensors(&oneWire);



class TempSensor : virtual public Sensor {
  public: 
    TempSensor(char *_label,  const byte * address = NULL);

    /* Set sensor address from a string of hex digits
    * sample string: char s[] = "0x28, 0xDE, 0x15, 0x5D, 0x05, 0x00, 0x00, 0x17";
    */
    virtual void setAddress(char *strAddress);

    /* prepare sensor for measurement, turn on power
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    virtual void measure();

    /* Powersave
    */
    virtual void sleep();

    static void discoverOneWireDevices(void);
        
  private:
    byte address[8];
    static boolean busActivated;
};


#endif

