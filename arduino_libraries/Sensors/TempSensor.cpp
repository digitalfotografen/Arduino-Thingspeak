#include "TempSensor.h"
#include "Arduino.h"

boolean TempSensor::busActivated = false;

TempSensor::TempSensor(char *_label, const byte * address) : Sensor(_label){
  if (address != NULL){
    for (int i=0; i<8; i++){
      this->address[i] = address[i];
    }
  }
  this->setPrepareTime(1000);
}

void TempSensor::setAddress(char *strAddress){
  if (strlen(strAddress) > 23){
    int pos = 0;
    char * tok;
    tok = strtok (strAddress," ,.\t");
    while ((pos < 8) && (tok != NULL)){
      this->address[pos++] = (byte) strtoul(tok, NULL, 16);
      tok = strtok(NULL," ,.\t");
    }
  } else {
    mlog.WARNING(F("Error TempSensor::setAddress empty or to short string"));
  }
}

void TempSensor::measure(){
  if (!busActivated){
    this->prepare();
  }
  mlog.DEBUG(F("TempSensor measure:"));
  mlog.DEBUG(this->label, false);
  mlog.DEBUG(F(" = "), false);
  float value = dallasSensors.getTempC(this->address);  
  mlog.DEBUG(value);
  this->putValue( value );
}

unsigned long TempSensor::prepare(){
  unsigned long t = Sensor::prepare();
  if (!busActivated){
    mlog.DEBUG(F("Preparing TempSensors"));
    dallasSensors.begin();
    busActivated = true;
    delay(10);
    dallasSensors.setResolution(12); // global for all sensors
    delay(10);
    dallasSensors.requestTemperatures();
  }
  return t;
}

void TempSensor::sleep(){
  busActivated = 0;
  Sensor::sleep();
}


void TempSensor::discoverOneWireDevices(void) {
  
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  char toHex[5] = "";
  
  mlog.INFO(F("Looking for 1-Wire devices..."));
  while(oneWire.search(addr)) {
    mlog.INFO(F("Found \'1-Wire\' device with address:"));
    for( i = 0; i < 8; i++) {
      mlog.INFO(F("0x"),false);
      if (addr[i] < 16) {
        mlog.INFO(F("0"),false);
      }
      itoa(addr[i], toHex, 16);
      mlog.INFO(toHex, false);
      if (i < 7) {
        mlog.INFO(F(","),false);
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        mlog.WARNING(F("CRC is not valid!\n"));
        return;
    }
  }
  mlog.INFO(F("That was all devices"));
  oneWire.reset_search();
  return;
}
