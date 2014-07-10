#ifndef THINGSPEAK_H
#define THINGSPEAK_H

#include "SensorGroup.h"
#include "Arduino.h"


class Thingspeak : public SensorGroup {
  public:
    Thingspeak(const char* _apiKey);
    void setApiKey(const char* _apiKey);
    //virtual void update();
    virtual void toString(char *buff);
    
  protected:
    char apiKey[20];
};

#endif
