#ifndef NCDT1302_SENSOR_H
#define NCDT1302_SENSOR_H

#include "Sensor.h"
#include "Arduino.h"
#include "MultiLog.h"

#define NCDT_BAUDRATE 115200

#define NCDT1302_START 0x2B2B2B0D
#define NCDT1302_ID 0x494C4431
#define NCDT1302_SET_DEFAULT 0x00F10002
#define NCDT1302_LASER_ON 0x00870002
#define NCDT1302_LASER_OFF 0x00860002
#define NCDT1302_DAT_OUT_ON 0x00770002
#define NCDT1302_DAT_OUT_OFF 0x00760002
#define NCDT1302_SET_OUTPUT_CHANNEL 0x00900003
#define NCDT1302_DIGITAL 0x00000001
#define NCDT1302_SET_BAUDRATE 0x00800003
#define NCDT1302_SET_SAVE_SETTINGS_MODE 0x00F70002
#define NCDT1302_RAM 0x00000000
#define NCDT1302_FLASH 0x00000001
#define NCDT1302_SET_EXT_INPUT_MODE 0x00F80003
#define NCDT1302_SCALING 0x00000000
#define NCDT1302_TRIGGER 0x00000001

class Ncdt1302 : virtual public Sensor {
  public: 
    Ncdt1302(char *label,
                  byte portNumber);

    virtual boolean begin(long baudrate = 115200);
    
    /* prepare sensor for measurement, turn on laser
    * returns ms until ready
    */
    virtual unsigned long prepare(); 

    virtual void measure();

    /* Powersave
    */
    virtual void sleep();
    
    boolean laserOnOff(boolean on);
    void setEnablePins(byte rxEnablePin, byte txEnablePin);
    
  protected:
    byte _portNumber = 0xFF; // default = disabled
    HardwareSerial *_serial;
    byte _rxEnablePin = 0;
    byte _txEnablePin = 0;
    
    boolean command(unsigned long com);
    boolean setParameter(unsigned long com, unsigned long data);
    void write(unsigned long data, boolean start = false);
    boolean readResponse(byte *buff, int length = 10);
    unsigned long readResponseValue();
    void dumpBuffer();
    void enable();
    void disable();
};

#endif

