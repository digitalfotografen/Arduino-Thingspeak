#include "GsmSensor.h"
#include "Arduino.h"

GsmSensor::GsmSensor(char *_label) : Sensor(_label){
  active = false;
}

#ifdef GSM_H
void GsmSensor::measure(){
  Serial.println("GSM sensor measure ");
  byte status;
  
  if (this->active){
    gsm.RxInit(5000, 100);
    gsm.SimpleWriteln("AT+CSQ");
    do {
      status = gsm.IsRxFinished();
    } while (status == RX_NOT_FINISHED);
  
    if (status == RX_FINISHED) {
      char * pch;
      pch = strstr((char *)gsm.comm_buf,"CSQ:");
      if (pch != NULL){
        Serial.print("CSQ:");
        Serial.println(pch+4);
        float value = atof(pch+4);
        this->statistic.add( value);
      } else {
        Serial.println("CSQ failed");
      }
    } else {
      Serial.println("CSQ timout");
    }
  }
}
#endif

unsigned long GsmSensor::prepare(){
  return 0;
}

void GsmSensor::sleep(){
}

void GsmSensor::activate(){
  this->active = true;
}

void GsmSensor::deactivate(){
  this->active = false;
}
