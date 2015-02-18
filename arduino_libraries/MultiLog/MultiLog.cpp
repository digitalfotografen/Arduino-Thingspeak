/*
  Multilog.cpp - Library for standard logging convention.
  Created by Ulrik Södergren, DigitalFotografen AB
  Inspired by LOG Created by Meir Michanie.
  Released into the public domain.
  Version 0.1
*/

#include <MultiLog.h>

//byte MultiLog::_level = LOG_DEBUG;
//byte MultiLog::_targets = LOG_SERIAL;
//File MultiLog::_file = File();

MultiLog mlog;
#ifdef SD_SUPPORT
//SdFat SD;
#endif

MultiLog::MultiLog(byte level, byte targets)
{
     setLevel(level);
     setTargets(targets);
}

boolean MultiLog::openFile(){
/*
  if (!SD.begin(8)){
     Serial.println("SD begin - failed");
  }
  delay(500);
*/
  #ifdef SD_SUPPORT
  _file = SD.open("logfile.txt", FILE_WRITE);
  
  if (_file.size() > 1000000000){ // remove logfiles over 1GB
    _file.close();
    SD.remove("logfile.txt");
    _file = SD.open("logfile.txt", FILE_WRITE);
  }
  if (_file == NULL){
      Serial.println("openfile - failed");
  }
  _file.println("");
  _file.println("");
  _file.println("=== NEW LOG =====================");
  _file.flush();
  #endif
  return true;
}

void MultiLog::TRACE(const __FlashStringHelper* string, boolean label){
  if (_level > 4) {
    if (label){
      print(string, F("\r\n[TRACE]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::TRACE(const char* string, boolean label)
{
  if (_level > 4) {
    if (label){
      print(string, F("\r\n[TRACE]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::TRACE(float number, boolean label)
{
  if (_level > 4) {
    if (label){
      print(number, F("\r\n[TRACE]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::TRACE(int number, boolean label)
{
  if (_level > 4) {
    if (label){
      print(number, F("\r\n[TRACE]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::DEBUG(const __FlashStringHelper* string, boolean label){
  if (_level > 3) {
    if (label){
      print(string, F("\r\n[DEBUG]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::DEBUG(const char* string, boolean label)
{
  if (_level > 3) {
    if (label){
      print(string, F("\r\n[DEBUG]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::DEBUG(int number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[DEBUG]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::DEBUG(float number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[DEBUG]: "));
    } else {
      print(number);
    }
  }
}


void MultiLog::INFO(const __FlashStringHelper* string, boolean label){
  if (_level > 2) {
    if (label){
      print(string, F("\r\n[INFO]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::INFO(const char* string, boolean label)
{
  if (_level > 2) {
    if (label){
      print(string, F("\r\n[INFO]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::INFO(int number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[INFO]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::INFO(float number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[INFO]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::WARNING(const __FlashStringHelper* string, boolean label){
  if (_level > 1) {
    if (label){
      print(string, F("\r\n[WARNING]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::WARNING(const char* string, boolean label)
{
  if (_level > 1) {
    if (label){
      print(string, F("\r\n[WARNING]: "));
    } else {
      print(string);
    }
  }
}

void MultiLog::WARNING(int number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[WARNING]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::WARNING(float number, boolean label){
  if (_level > 3) {
    if (label){
      print(number, F("\r\n[WARNING]: "));
    } else {
      print(number);
    }
  }
}

void MultiLog::CRITICAL(const __FlashStringHelper* string, boolean label){
  if (label){
    print(string, F("\r\n[CRITICAL]: "));
  } else {
    print(string);
  }
}

void MultiLog::CRITICAL(const char* string, boolean label)
{
  if (label){
    print(string, F("\r\n[CRITICAL]: "));
  } else {
    print(string);
  }
}

void MultiLog::CRITICAL(int number, boolean label){
  if (label){
    print(number, F("\r\n[CRITICAL]: "));
  } else {
    print(number);
  }
}

void MultiLog::CRITICAL(float number, boolean label){
  if (label){
    print(number, F("\r\n[CRITICAL]: "));
  } else {
    print(number);
  }
}

