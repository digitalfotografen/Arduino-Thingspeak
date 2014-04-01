/***********************************************************************************
  SDconfigFile is an Arduino targeted library to load configurations from an SD-card.
  It pareses a text file for key value pairs.
  
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
  
  
  This design goal of this library was to keep RAM memory use low. 
  The penalty is slow loading. The file is read for every requested key and,
  parsing is performed line by line ignoring any line with the wrong key. 
  File is closed after each key.
  
  Written by Ulrik Södergren, DigitalFotografen AB, 2014

*************************************************************************************/

#include "SDconfigFile.h"

boolean SDconfigFile::initialised = false;
//#define DEBUG 1

/*
* Constructor
* file must be at root level
*/
SDconfigFile::SDconfigFile(char *filename)
{
  SDconfigFile::filename = filename;
  SDconfigFile::opened = false;
}

/*
* Initialize instance and SD card reader
* cs chip select pin - usually 4
* ss pin - usually 10
*
* Returns true on success
*/
boolean SDconfigFile::begin(int ss, int cs)
{
  // only need to execute this once
  if (initialised)
    return true;

#ifdef DEBUG
  Serial.println("SDconfigFile::Initializing SD card...");
#endif

  pinMode(ss, OUTPUT);
  if (SD.begin(cs))
  {
    initialised = true;
    return true;
  }
  return false;
}


/*
* Open file for reading
*
* Returns true on success
*/
boolean SDconfigFile::open()
{
  if (!initialised)
    return false;
  if (file = SD.open(SDconfigFile::filename, FILE_READ))
  {
    opened = true;
    return true;
  }
  return false;
}


/*
* Close file
*/
void SDconfigFile::close()
{
  SDconfigFile::file.close();
  opened = false;
}


/*
* Read one null terminated line from file to buff
*
* Returns line length, NULL on failure
*/
int SDconfigFile::readln(char *buff, int maxlen)
{
  int i = 0;
  boolean comment = false;
  
  buff[i] = '\0';
  if (!opened)
  {
    Serial.println("SDconfigFile::file not open for read");
    return NULL;
  }

  while (file.available() && (i < maxlen)) {
    char c = file.read();
    switch (c)
    {
      case '\r':
      case '\n':
        return i;
        break;
        
      case ' ': // skip white space at line begining
      case '\t':
        if (i)
        {
          buff[i++] = c;
        }
        break;
        
      case '#': // comment, skip rest of line
        comment = true;
        break;
        
      default:
        if (!comment)
        {
          buff[i++] = c;
        }
    }
    buff[i] = '\0';
  }
  return i;
}


/* 
* Test if more characters are available in file
*/
boolean SDconfigFile::available()
{
  return SDconfigFile::file.available();
}

/* 
* Get one key value pair from file
* 
*/
char* SDconfigFile::get(char *value, char *key, char* group){
  boolean inGroup = strlen(group) ? false : true;
  char *saveptr1;
  
  SDconfigFile::open();
  
  if (!opened)
  {
    Serial.println("SDconfigFile::file not open for read");
    return NULL;
  }

  char row[256] = "";
  char * token;
  
  while(SDconfigFile::available())
  {
    if (SDconfigFile::readln(row, 256)){
      if (row[0] == '[')
      {
        token = strtok_r(row, "[]", &saveptr1);
        inGroup = strcasecmp(group, token) ? false : true;
      }
      
      if (inGroup){
        char * token = strtok_r(row, " :\t", &saveptr1);
        if (!strcasecmp(key, token))
        {
          value = strtok_r(NULL, " :\t", &saveptr1);
          return value;
        }
      }
    }
  }
  
  SDconfigFile::file.close();
  return NULL;
}
