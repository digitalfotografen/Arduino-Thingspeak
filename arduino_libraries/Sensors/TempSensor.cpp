#include "TempSensor.h"
#include "Arduino.h"

boolean TempSensor::busActivated = false;

TempSensor::TempSensor(char *_label, const byte * address) : Sensor(_label){
  if (address != NULL){
    for (int i=0; i<8; i++){
      this->address[i] = address[i];
    }
  }
  this->setPrepareTime(750);
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
    Serial.println("Error TempSensor::setAddress empty or to short string");
  }
}

void TempSensor::measure(){
  if (!busActivated){
    this->prepare();
  }
  Serial.print("TempSensor measure ");
  Serial.print(label);
  Serial.print(": ");
  float value = dallasSensors.getTempC(this->address);  
  Serial.println(value);
  this->statistic.add( value );
}

unsigned long TempSensor::prepare(){
  Serial.print("TempSensorSensor prepare ");
  Serial.println(this->label);
  unsigned long t = Sensor::prepare();
  if (!busActivated){
    Serial.println("preparing TempSensor");
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
  
  Serial.print("Looking for 1-Wire devices...\n\r");
  while(oneWire.search(addr)) {
    Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
  }
  Serial.print("\n\r\n\rThat's it.\r\n");
  oneWire.reset_search();
  return;
}
