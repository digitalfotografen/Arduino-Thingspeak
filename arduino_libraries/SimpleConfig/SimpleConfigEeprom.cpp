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
* file must be at root level
*/
SimpleConfigEeprom::SimpleConfigEeprom(int _baseAddress)
{
  this->baseAddress = _baseAddress;
  this->pos = 0;
  this->endAddress = 1024;
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
  Serial.println("SimpleConfigEeprom::Initializing...");
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
    EEPROM.write(this->baseAddress + this->pos, char(0x04));
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
    Serial.println("SimpleConfigEeprom::readln NOT OPEN");
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
  return (!this->eof & (this->baseAddress + this->pos <= this->endAddress));
}

int SimpleConfigEeprom::writeln(const char *buff,int maxlen){
  if (!this->opened | !this->writeMode){
    Serial.println("SimpleConfigEeprom::writeln NOT OPEN FOR WRITE");
    return 0;
  }
  
  int length = min(strlen(buff), maxlen);
    if (this->baseAddress + this->pos + length > this->endAddress){
    return 0;
  }

  int i = 0;
  while (i < length){
    EEPROM.write(this->baseAddress + this->pos++, buff[i++]);
  }
  
  // make sure we terminate with new line
  if (!(buff[length -1] == '\n') || (buff[length -1] == '\r')){
    Serial.println("terminate line");
    EEPROM.write(this->baseAddress + this->pos++, '\n');
  }
  return this->endAddress - this->pos - this->baseAddress;
}