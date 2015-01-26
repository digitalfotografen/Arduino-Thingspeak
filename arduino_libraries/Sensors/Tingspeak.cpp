#include "Thingspeak.h"

Thingspeak::Thingspeak(const char* _apiKey) : SensorGroup(){
  this->setApiKey(_apiKey);
}

void Thingspeak::setApiKey(const char* _apiKey){
  strcpy(apiKey, _apiKey);
}

void Thingspeak::toString(char* buff){
  char tmp[20] = "";
  strcpy(buff, "api_key=");
  strcat(buff, apiKey);
  if (numberOfSensors > 0){
    for (int i = 0; i < numberOfSensors; i++){
      strcpy(tmp, sensors[i]->getLabel());      
      if (tmp[0] != 0x00){
        strcat(buff, "&");
        strcat(buff, tmp);
        strcat(buff, "=");
        strcpy(tmp, "");
        sensors[i]->toString(tmp, 20, SENSOR_AVERAGE);
        strcat(buff, tmp);
      }
      
      strcpy(tmp, "");
      sensors[i]->getLabelMin(tmp);
      if (tmp[0] != 0x00){
        strcat(buff, "&");
        strcat(buff, tmp);
        strcat(buff, "=");
        strcpy(tmp, "");
        sensors[i]->toString(tmp, 20, SENSOR_MIN);
        strcat(buff, tmp);
      }
      
      strcpy(tmp, "");
      sensors[i]->getLabelMax(tmp);
      if (tmp[0] != 0x00){
        strcat(buff, "&");
        strcat(buff, tmp);
        strcat(buff, "=");
        strcpy(tmp, "");
        sensors[i]->toString(tmp, 20, SENSOR_MAX);
        strcat(buff, tmp);
      }
      
    }
  }
  
}