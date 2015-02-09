#include "Csv.h"

Csv::Csv(const char* filename) : SensorGroup(){
  _file = SD.open(filename, FILE_WRITE);
  _file.flush();
}

void Csv::toString(char* buff){
  char tmp[20] = "";
  strcpy(buff, "date");
  if (numberOfSensors > 0){
    for (int i = 0; i < numberOfSensors; i++){
      strcpy(tmp, "");
      strcat(buff, ",");
      sensors[i]->toString(tmp, 20, SENSOR_LAST);
      strcat(buff, tmp);
    }
  }
  strcat(buff, "\r");
}

void Csv::save(){
  char buff[25] = "";
  //mlog.TRACE(F("CSV::save:"));
  _file.print(year());
  //mlog.TRACE(year(), false);
  //mlog.TRACE(F("-"), false);
  _file.print("-");
  _file.print(month());
  //mlog.TRACE(month(), false);
  //mlog.TRACE(F("-"), false);
  _file.print("-");
  _file.print(day());
  //mlog.TRACE(day(), false);
  //mlog.TRACE(F(" "), false);
  _file.print(" ");
  _file.print(hour());
  //mlog.TRACE(hour(), false);
  //mlog.TRACE(F(":"), false);
  _file.print(":");
  _file.print(minute());
  //mlog.TRACE(minute(), false);
  //mlog.TRACE(F(":"), false);
  _file.print(" ");
  _file.print(second());
  //mlog.TRACE(second(), false);
  if (numberOfSensors > 0){
    for (int i = 0; i < numberOfSensors; i++){
      strcpy(buff, ",");
      sensors[i]->toString(buff, 20, SENSOR_LAST);
      _file.print(buff);
      //mlog.TRACE(F(","), false);
      //mlog.TRACE(buff, false);
    }
  }
  _file.println();
  _file.flush();
}