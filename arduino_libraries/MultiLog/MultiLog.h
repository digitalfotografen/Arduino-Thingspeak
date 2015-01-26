/*
  Multilog.h - Library for standard logging convention.
  Created by Ulrik Södergren, DigitalFotografen AB
  Inspired by LOG Created by Meir Michanie.
  Released into the public domain.
  Version 0.2

  History
  Added support for SD-card MultiLogging
*/

#ifndef MultiLog_h
#define MultiLog_h
#include "Arduino.h"
#include <SD.h>

#define LOG_SERIAL 1
#define LOG_SOFTWARESERIAL 2
#define LOG_FILE 4

#define LOG_CRITICAL 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4
#define LOG_TRACE 5



class MultiLog {
  public:
    MultiLog(int level, byte targets = LOG_SERIAL);
    boolean openFile();

    void TRACE(const __FlashStringHelper* string, boolean label = false);
    void TRACE(const char* string, boolean label = false);
    void TRACE(int number, boolean label = false);

    void DEBUG(const __FlashStringHelper* string, boolean label = true);
    void DEBUG(const char* string, boolean label = true);
    void DEBUG(int number, boolean label = false);

    void INFO(const __FlashStringHelper* string, boolean label = true);
    void INFO(const char* string, boolean label = true);
    void INFO(int number, boolean label = false);

    void WARNING(const __FlashStringHelper* string, boolean label = true);
    void WARNING(const char* string, boolean label = true);
    void WARNING(int number, boolean label = false);

    void CRITICAL(const __FlashStringHelper* string, boolean label = true);
    void CRITICAL(const char* string, boolean label = true);
    void CRITICAL(int number, boolean label = false);


    inline int  getLevel(void)      {
      return _level;
    }
    inline void setLevel(int level) {
      _level = level;
    }

    inline byte  getTargets(void)      {
      return _targets;
    }
    inline void setTargets(byte targets) {
      _targets = targets;
    }

  protected:
    File _file;
    int _level;
    byte _targets;

    inline void fprint_P(const __FlashStringHelper *ifsh){
      const char PROGMEM *p = (const char PROGMEM *)ifsh;
      size_t n = 0;
      while (1) {
        char c = pgm_read_byte(p++);
        if (c == 0x00) break;
        n += _file.print(c);
      }
    }
    
    inline void print(const __FlashStringHelper* string, 
                      const __FlashStringHelper* label = NULL){
        if (_targets & LOG_SERIAL){
          if (label){
            Serial.print(label);
          }
          Serial.print(string);
        }
        if (_targets & LOG_FILE){
          if (label){
            fprint_P(label);
          }
          fprint_P(string);
          _file.flush();
        }
    }


    inline void print(const char* string, 
                      const __FlashStringHelper* label = NULL){
        if (_targets & LOG_SERIAL){
          if (label){
            Serial.print(label);
          }
          Serial.print(string);
        }
        if (_targets & LOG_FILE){
          if (label){
            fprint_P(label);
          }
          _file.print(string);
          _file.flush();
        }
    }

    inline void print(int number, 
                      const __FlashStringHelper* label = NULL){
        if (_targets & LOG_SERIAL){
          if (label){
            Serial.print(label);
          }
          Serial.print(number);
        }
        if (_targets & LOG_FILE){
          if (label){
            fprint_P(label);
          }
          _file.print(number);
          _file.flush();
        }
    }
};

static MultiLog mlog = MultiLog(LOG_DEBUG, LOG_SERIAL);

#endif
