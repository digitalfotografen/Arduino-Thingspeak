#ifndef CSV_H
#define CSV_H

#include "SensorGroup.h"
#include "Arduino.h"
#include <SD.h>
#include <Time.h>

class Csv : public SensorGroup {
  public:
    Csv();
    virtual boolean begin(const char* filename);
    virtual void toString(char *buff);
    virtual void save();
    
  protected:
    File _file;
};

#endif
