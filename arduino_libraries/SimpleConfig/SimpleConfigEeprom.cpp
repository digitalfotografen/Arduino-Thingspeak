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


#include "SimpleConfigEeprom.h"

/*
* Constructor
* baseAddress defines where in eeprom config starts.
*/
SimpleConfigEeprom::SimpleConfigEeprom(int _baseAddress)
{
  this->baseAddress = _baseAddress;
  this->pos = 0;
}

/*
* Initialize instance 
*
* Returns true on success
*/
boolean SimpleConfigEeprom::begin()
{
  // only need to execute this once
  if (initialised)
    return true;

#ifdef DEBUG
  Serial.println(F("SimpleConfigEeprom::Initializing..."));
#endif
  // not much to do in base class 

  return true;
}


/*
* Open for reading
*
* Returns true on success
*/
boolean SimpleConfigEeprom::open(boolean write)
{
  this->opened = true;
  this->writeMode = write;
  this->pos = 0;
  this->eof = false;
  return true;
}


/*
* Close file
*/
boolean SimpleConfigEeprom::close()
{
  // if writeMode, then remember to terminate file 
  if (this->writeMode){
    //EEPROM.write(this->baseAddress + this->pos++, '\n');
    EEPROM.write(this->baseAddress + this->pos++, char(0x04));
    this->eof = true;
  }
  this->writeMode = false;
  this->opened = true;
  return true;
}


/*
* Read one null terminated line from file to buff
*
* Returns line length, NULL on failure
*/
int SimpleConfigEeprom::readln(char *buff, int maxlen)
{
  if (!this->opened){
    Serial.println(F("SimpleConfigEeprom::readln NOT OPEN"));
    return 0;
  }
  
  int i = 0;
  boolean comment = false;
  
  buff[i] = '\0';

  while (this->available() && (i < maxlen)) {
    char c = (char) EEPROM.read(this->baseAddress + this->pos++);
    switch (c)
    {
      case '\r':
      case '\n':
        return i;
        break;
        
      case ' ': // skip white space at line begining
      case '\t':
        if (i > 0)
        {
          buff[i++] = c;
        }
        break;
        
      case '#': // comment, skip rest of line
        comment = true;
        break;
        
      case char(0x04):
        this->eof = true;
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
* Test if more characters are available
*/
boolean SimpleConfigEeprom::available()
{
  return (!this->eof && (this->baseAddress + this->pos <= E2END));
}

int SimpleConfigEeprom::writeln(const char *buff,int maxlen){
  int length = min(strlen(buff), maxlen);
  int start = -1;
  int end = length;

  if (!this->opened | !this->writeMode){
    Serial.println(F("SimpleConfigEeprom::writeln NOT OPEN FOR WRITE"));
    return 0;
  }
  

  if (this->baseAddress + this->pos + length > E2END){
    return 0;
  }

  if (length > 0){
    // trim white space at both ends of buffer
    while (isspace(buff[++start]));
    while (isspace(buff[--end]) && (end != start));

    // write data
    int i = start;
    int rowLength = 0;
    while ((i <= end) && (buff[i] != '#')){  // skip comments
      Serial.print(buff[i]);
      EEPROM.write(this->baseAddress + this->pos++, buff[i++]);
    }
  
    // terminate with new line, but only lines with content
    if (i > start){
      EEPROM.write(this->baseAddress + this->pos++, '\n');
      Serial.println();
    }
  }
  return E2END - this->pos - this->baseAddress;
}
