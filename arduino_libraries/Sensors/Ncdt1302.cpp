#include "Ncdt1302.h"
#include "Arduino.h"


Ncdt1302::Ncdt1302(char *label,
                  byte portNumber) : Sensor(label){
  _portNumber = portNumber;
  this->setPrepareTime(1000);
  this->setRangeIn(40, 4055);
  this->setRangeOut(20, 50);
}

boolean Ncdt1302::begin(long baudrate){
  enable();
  mlog.DEBUG(F("Ncdt1302 begin"));
  switch (_portNumber){
    case 0:
      _serial = &Serial;
      mlog.DEBUG(F("Ncdt1302 using Serial0"));
      break;
    
    case 1:
      _serial = &Serial1;
      mlog.DEBUG(F("Ncdt1302 using Serial1"));
      break;

    case 2:
      _serial = &Serial2;
      mlog.DEBUG(F("Ncdt1302 using Serial2"));
      break;

    case 3:
      _serial = &Serial3;
      mlog.DEBUG(F("Ncdt1302 using Serial3"));
      break;
  }
  

  unsigned long baudrateCode = 0x00000000;
  switch (baudrate/100){
    case 1152:
      baudrateCode = 0x00000000;
      break;

    case 576:
      baudrateCode = 0x00000001;
      break;

    case 384:
      baudrateCode = 0x00000002;
      break;

    case 192:
      baudrateCode = 0x00000003;
      break;

    case 96:
      baudrateCode = 0x00000004;
      break;
      
    default:
      mlog.CRITICAL(F("Ncdt1302 Illegal baudrate"));
  }

  mlog.DEBUG(F("Baudrate code"));
  mlog.DEBUG((int) baudrateCode);
  // try selected baudrate
  _serial->begin(baudrate);
  _serial->setTimeout(100);

  mlog.DEBUG(F("Ncdt1302 DATA_OUT_OFF"));
  if (command(NCDT1302_DAT_OUT_OFF)){
    mlog.INFO(F("Ncdt1302 Correct baudrate"));
  } else {
    mlog.INFO(F("Ncdt1302 not correct baudrate"));
    for (int i = 0; i<6; i++){
      //_serial->end();
      switch (i){
        case 0:
          _serial->begin(115200);
          mlog.INFO(F("115200 baud"));
          break;
      
        case 1:
          _serial->begin(57600);
          mlog.INFO(F("57600 baud"));
          break;
      
        case 2:
          _serial->begin(38400);
          mlog.INFO(F("38400 baud"));
          break;
      
        case 3:
          _serial->begin(19200);
          mlog.INFO(F("19200 baud"));
          break;
      
        case 4:
          _serial->begin(9600);
          mlog.INFO(F("9600 baud"));
          break;
      
        default:
          _serial->begin(9600);
          mlog.CRITICAL(F("Ncdt1302 Could not communicate with sensor"));
          _serial = NULL;
          return false;
          break;
      
      }
      _serial->setTimeout(50);
      _serial->flush();

      // found baudrate, setup sensor
      if (command(NCDT1302_DAT_OUT_OFF)){
        mlog.INFO(F("Ncdt1302 found baudrate"));
        if (command(NCDT1302_SET_DEFAULT)){
          mlog.INFO(F("Ncdt1302 set_default, baudrate 115200"));
          _serial->flush();
          delay(1000);
          _serial->begin(115200);
          _serial->setTimeout(50);
          delay(1000);
        }
        
        if (!setParameter(NCDT1302_SET_BAUDRATE, baudrateCode)){
          mlog.INFO(F("Ncdt1302 failed to set baudrate"));
          _serial = NULL;
          return false;
        } else {
          mlog.INFO(F("Ncdt1302 changing baudrate"));
          _serial->flush();
          delay(1000);
          _serial->begin(baudrate);
          _serial->setTimeout(50);
          delay(1000);
        }
        
        if (setParameter(NCDT1302_SET_OUTPUT_CHANNEL, NCDT1302_DIGITAL)){
          mlog.DEBUG(F("Ncdt1302 SET_OUTPUT_CHANNEL"));
        } else {
          mlog.CRITICAL(F("Ncdt1302 Failed to SET_OUTPUT_CHANNEL"));
        }

        if (setParameter(NCDT1302_SET_EXT_INPUT_MODE, NCDT1302_TRIGGER)){
          mlog.DEBUG(F("Ncdt1302 SET_EXT_INPUT_MODE TRIGGER"));
        } else {
          mlog.CRITICAL(F("Ncdt1302 Failed to SET_EXT_INPUT_MODE"));
        }

        i = 5;
      }
    }
  }
  
  if (command(NCDT1302_DAT_OUT_OFF)){
    mlog.DEBUG(F("Ncdt1302 DATA_OUT_OFF"));
  }

  laserOnOff(true);
  return true;
}

void Ncdt1302::measure(){
/*
  0     39 SMR back-up
  40    4055 Measurement range
  4056  4095 EMR back-up
  16370 16383 Error codes
*/
  int high = 0;
  int low = 0;
  int value = 0;
  unsigned long timeout = millis() + 500;

  mlog.DEBUG(F("Ncdt1302 measure:"));
  mlog.DEBUG(label, false);
  if (_serial == NULL){
    mlog.WARNING(F("sensor not connected"));
    return;
  }

  enable();
  dumpBuffer();
  while ((_serial->available() < 2) && (millis() < timeout)){
    // wait for data
  }
  timeout = millis() + 100;
  do {
    high = _serial->read();
    if (high < 0x80) high = _serial->read();
    low = _serial->read();
    value = (unsigned long) high & 0x7F;
    value = value << 7;
    value += (unsigned long) low;
  } while (((high < 0x80) || (low > 0x7F) || (value > 4095)) && (millis() < timeout));
  
  if (value > 4095){
    // error
    switch (value){
      case 16370: 
        mlog.WARNING(F("no object detected"));
        break;
        
      case 16372: 
        mlog.WARNING(F("too close to the sensor"));
        break;
        
      case 16374:
        mlog.WARNING(F("too far from the sensor"));
        break;
        
      case 16376: 
        mlog.WARNING(F("target can not be evaluated"));
        break;
        
      case 16380:
        mlog.WARNING(F("target moves towards the sensor"));
        break;
        
      case 16382: 
        mlog.WARNING(F("target moves away from sensor"));
        break;
        
      default:
        mlog.WARNING(F("unknown sensor error "));
        mlog.WARNING((int) value);
        break;
    }
  } else {
    //Serial.println(result, DEC);
    mlog.DEBUG(" in:", false);
    mlog.DEBUG(value);
    float fValue = this->map((float) value, 
                              this->rangeInMin,
                              this->rangeInMax, 
                              this->rangeOutMin, 
                              this->rangeOutMax);
    mlog.DEBUG(" out:", false);
    mlog.DEBUG(fValue);
    this->putValue( fValue );
  }
  disable();
}

unsigned long Ncdt1302::prepare(){
  enable();
  mlog.DEBUG("Ncdt1302 prepare:");
  mlog.DEBUG(this->label, false);
  laserOnOff(true);

  mlog.DEBUG(F("Ncdt1302 DATA_OUT_ON"));
  if (!command(NCDT1302_DAT_OUT_ON)){
    mlog.WARNING(F("DATA_OUT_ON failed"));
  }
  unsigned long t = Sensor::prepare();
  return max(t, this->prepareTime); // Laser sensor needs 500 ms
  disable();
}

void Ncdt1302::sleep(){
  mlog.DEBUG(F("Ncdt1302 sleep"));
  mlog.DEBUG(F("Ncdt1302 DATA_OUT_OFF"));
  enable();
  if (!command(NCDT1302_DAT_OUT_OFF)){
    mlog.WARNING(F("DATA_OUT_OFF failed"));
  }
  laserOnOff(false);
  Sensor::sleep();
  disable();
}

boolean Ncdt1302::laserOnOff(boolean on){
  boolean result = true;
  enable();
  if (on){
    mlog.INFO(F("Laser on"));
    if (!command(NCDT1302_LASER_ON)){
      mlog.WARNING(F("Failed to switch laser on"));
      result = false;
    }
  } else {
    mlog.INFO(F("Laser off"));
    if (!command(NCDT1302_LASER_OFF)){
      mlog.WARNING(F("Failed to switch laser off"));
      result = false;
    }
  }
  return result;
}

void Ncdt1302::setEnablePins(byte rxEnablePin, byte txEnablePin){
  _rxEnablePin = rxEnablePin;
  pinMode(_rxEnablePin, OUTPUT);
  _txEnablePin = txEnablePin;
  pinMode(_txEnablePin, OUTPUT);
  digitalWrite(_rxEnablePin, HIGH);
  digitalWrite(_txEnablePin, LOW);
}


boolean Ncdt1302::command(unsigned long com){
  for (int i = 0; i < 3; i++){
    dumpBuffer();
    // a command starts with 0x20
    write(com | 0x20000000, true);
    // response is same as command but first byte is A0
    if (readResponseValue() == (com| 0xA0000000)) 
      return true;
  }
  return false;
}

boolean Ncdt1302::setParameter(unsigned long com, unsigned long data){
  for (int i = 0; i < 3; i++){
    dumpBuffer();
    write(com | 0x20000000, true);
    write(data);
    // response is same as command but last bit is zero and first byte is A0
    if (readResponseValue() == (com & 0xFFFFFFFE | 0xA0000000)) 
      return true;
  }
  return false;
}

void Ncdt1302::write(unsigned long data, boolean start){
  if (_serial == NULL){
    mlog.WARNING(F("sensor not connected"));
    return;
  }

  if (start){
    write(NCDT1302_START, false);
    write(NCDT1302_ID, false);
  }
  
  for (int i = 0; i < 4; i++){
    _serial->write((data & 0xFF000000) >> 24);
    data = data << 8;
  }
}

boolean Ncdt1302::readResponse(byte *response, int length){
  if (_serial == NULL){
    mlog.WARNING(F("sensor not connected"));
    return false;
  }
  
  byte buff[20] = "";
  byte pos = 0;
  char start[5] = "ILD1";
  char end[5] = {0x20, 0x20, 0x0D, 0x0A, 0x00};

  unsigned long timout =  millis() + 300; 

  // workaround due to trouble with serial->find(s) and timeout
  while ((millis() < timout) && (pos < 4)){
    if(_serial->available()){
      buff[pos++] = _serial->read();
      for (int i = 0; i < pos; i++){
        if (buff[i] != start[i]){
          pos = 0;
        }
      }
    }
  }
  
  if (pos == 4){
    _serial->readBytes(response, length);
    if (_serial->find(end)){
      return true;
    }
    mlog.DEBUG(F("readresponse - no termination"));
    return false;
  }

  mlog.DEBUG(F("readresponse - no start"));
  return false;
}

unsigned long Ncdt1302::readResponseValue(){
  byte buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  unsigned long result = 0;
  readResponse(buff, 4);
  //Serial.println();
  for (int i=0; i<4; i++){
    result = result << 8;
    result +=  buff[i];
  }
  //Serial.println(result, HEX);
  return result;
}

void Ncdt1302::dumpBuffer(){
  if (_serial == NULL){
    return;
  }

  char dump[65];
  int i = min(_serial->available(),64);
  if (i > 0){
    //mlog.DEBUG("dump buffer:");
    //mlog.DEBUG((int)  _serial->readBytes(dump, i), false);
    _serial->readBytes(dump, i);
  }
}

void Ncdt1302::enable(){
  if (_rxEnablePin > 0 && _txEnablePin > 0){
    digitalWrite(_rxEnablePin, LOW);
    digitalWrite(_txEnablePin, HIGH);
  }
  delay(50);
}

void Ncdt1302::disable(){
  if (_rxEnablePin > 0 && _txEnablePin > 0){
    digitalWrite(_rxEnablePin, HIGH);
    digitalWrite(_txEnablePin, LOW);
  }
  dumpBuffer();
}
