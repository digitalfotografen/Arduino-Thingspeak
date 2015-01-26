/***********************************************************************************
  SimpleConfig is an Arduino library to load/save configurations from textfiles.
  It pareses a text file or memory area for key value pairs.
  Text file can be stored on SDcard or EEPROM memory
  This design goal of this library was to keep RAM memory use low. 
  The penalty is slow loading. The file is read for every requested key and,
  parsing is performed line by line ignoring any line with the wrong key. 
  
  Comments starts with #
  Keys and values are separated by spaces or colons.
  Keys are not case sensitive, all are converted to lower case
  Configuration kan be divided in groups by adding [groupname] lines.
  
  #sample file
  
  [user1]
  user one
  password secret
  name Uno

  [user2]
  user two
  password topSecret
  name Due
  
  
  Written by Ulrik Södergren, DigitalFotografen AB, 2014

*************************************************************************************/
#if !defined(__SIMPLE_CONFIG_H__)
#define __SIMPLE_CONFIG_H__ 1

#include "Arduino.h"
#include <MultiLog.h>

#define ROW_LENGTH 80 // max row length

class SimpleConfig {
  public:
    SimpleConfig();
    virtual boolean begin();
    /* Open file
    * write flag is set true to write file from start
    * Returns true on success
    */
    virtual boolean open(boolean write = false);
    virtual boolean close();

    /* Retrieves value for selected key and group
    * Returns null on fail
    */
    virtual char* get(char *value, const char *key, const char* group = "");

    /* Retrieves cstring value for selected key and group
    * Returns defaultValue on fail
    */
    virtual char* getStr(char *value, const char *key, const char* group = "", const char* defaultValue = "");

    /* Retrieves integer value for selected key and group
    * Returns defaultValue on fail
    */
    virtual int getInt(const char *key, const char* group = "", int defaultValue = 0);

    /* Retrieves float value for selected key and group
    * Returns defaultValue on fail
    */
    virtual float getFloat(const char *key, const char* group = "", float defaultValue = 0);

    /* Retrieves boolean value for selected key and group
    * Returns true if value is any of: on, enabled, true, 1
    * Returns defaultValue on fail
    */
    virtual boolean getBoolean(const char *key, const char* group = "", boolean defaultValue = false);

    virtual boolean available();
    /* Displays configuration to terminal 
    */
    virtual void display();

    /* Writes one line to storage. 
    * Returns 0 on fail, returns remaining free space on success
    */
    virtual int writeln(const char *buff,int maxlen = ROW_LENGTH);

    /* Reads one line from storage. 
    * Returns 0 on fail, returns length on success
    */
    virtual int readln(char *buff,int maxlen = ROW_LENGTH);

  protected:
    static boolean opened;
    static boolean writeMode;
    static boolean initialised;
    
    virtual void debugDefaultKey(const char *key, const char* group);
};

#endif
