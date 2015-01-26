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


#include "SimpleConfig.h"

boolean SimpleConfig::initialised = false;
boolean SimpleConfig::opened = false;
boolean SimpleConfig::writeMode = false;

//#define DEBUG 1

/*
* Constructor
* file must be at root level
*/
SimpleConfig::SimpleConfig()
{
  
}

/*
* Initialize instance 
*
* Returns true on success
*/
boolean SimpleConfig::begin()
{
  // only need to execute this once
  if (initialised)
    return true;

  mlog.INFO(F("SimpleConfig::Initializing..."));
  // not much to do in base class 

  return true;
}

/* 
* Get one key value pair from file
* 
*/
char* SimpleConfig::get(char *value, const char *key, const char* group){
  boolean inGroup = strlen(group) ? false : true;
  char *saveptr1;
  
  this->open(false);
  
  char row[ROW_LENGTH+1] = "";
  char * token;
  
  while(this->available())
  {
    if (this->readln(row, ROW_LENGTH)){
      if (row[0] == '[')
      {
        token = strtok_r(row, "[]", &saveptr1);
        inGroup = strcasecmp(group, token) ? false : true;
      }
     
      if (inGroup){
        token = strtok_r(row, " :\t", &saveptr1);
        if (!strcasecmp(key, token))
        {
          token = strtok_r(NULL, " :\t", &saveptr1);
          // make sure we return a proper null terminated string
          strcpy(value, "");
          strcat(value, token);
          this->close();
          return value;
        }
      }
    }
  }
  
  this->close();
  return NULL;
}

char* SimpleConfig::getStr(char *value, const char *key, const char* group, const char* defaultValue){
  char* result = this->get(value, key, group);
  if (result == NULL){
    debugDefaultKey(key, group);
    strcpy(value, defaultValue);
 }
  return result;
}

int SimpleConfig::getInt(const char *key, const char* group, int defaultValue){
  char s[ROW_LENGTH] = "";
  if (this->get(s, key, group)){
    return atoi(s);
  }
  
  debugDefaultKey(key, group);
  return defaultValue;
}


float SimpleConfig::getFloat(const char *key, const char* group, float defaultValue){
  char s[ROW_LENGTH] = "";
  if (this->get(s, key, group)){
    float value = atof(s);
    return value;
  }
  
  debugDefaultKey(key, group);
  return defaultValue;
}

boolean SimpleConfig::getBoolean(const char *key, const char* group, boolean defaultValue){
  char s[ROW_LENGTH] = "";
  if (this->get(s, key, group)){
    boolean value = false;
    if (!strncmp(s, "true", 10)) value = true;
    if (!strncmp(s, "on", 10)) value = true;
    if (!strncmp(s, "1", 10)) value = true;
    if (!strncmp(s, "enabled", 10)) value = true;
    return value;
  }
  
  debugDefaultKey(key, group);
  return defaultValue;  
}

void SimpleConfig::display(){
  char row[ROW_LENGTH+1] = "";
  mlog.INFO(F("=== Configuration ==="));
  this->open(false);
  while(this->available()){
    this->readln(row, ROW_LENGTH);
    mlog.INFO(row);
  }
  mlog.INFO(F("=== END of Configuration ==="));
}

/*
* Open for reading
*
* Returns true on success
*/
boolean SimpleConfig::open(boolean write)
{
  mlog.WARNING(F("SimpleConfig::base class called"));
  return false;
}


/*
* Close file
*/
boolean SimpleConfig::close()
{
  mlog.WARNING(F("SimpleConfig::base class called"));
  // if writeMode, then remember to terminate file 
  return false;
}


/*
* Read one null terminated line from file to buff
*
* Returns line length, NULL on failure
*/
int SimpleConfig::readln(char *buff, int maxlen)
{
  mlog.WARNING(F("SimpleConfig::base class called"));
  buff[0] = '\0';
  return NULL;
}

/* 
* Test if more characters are available
*/
boolean SimpleConfig::available()
{
  mlog.WARNING(F("SimpleConfig::base class called"));
  return false;
}

int SimpleConfig::writeln(const char *buff, int maxlen){
  mlog.WARNING(F("SimpleConfig::base class called"));
  return 0;
}

void SimpleConfig::debugDefaultKey(const char *key, const char* group){
  mlog.DEBUG(F("Default config key:"));
  mlog.DEBUG(key, false);
  mlog.DEBUG(F(" group:"), false);
  mlog.DEBUG(group, false);
}
