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
#if !defined(__SIMPLE_CONFIG_EEPROM_H__)
#define __SIMPLE_CONFIG_EEPROM_H__ 1

#include "SimpleConfig.h"
#include <EEPROM.h>


#define ROW_LENGTH 80 // max row length

class SimpleConfigEeprom : virtual public SimpleConfig{
  public:
    SimpleConfigEeprom(int baseAddress = 0);
    virtual boolean begin();
    /* Open file
    * write flag is set true to write file from start
    * Returns true on success
    */
    virtual boolean open(boolean write = false);
    virtual boolean close();
    virtual boolean available();
    /* Writes on line to storage. 
    * Returns 0 on fail, returns remaining free space on success
    */
    virtual int writeln(const char *buff,int maxlen = ROW_LENGTH);
    virtual int readln(char *buff,int maxlen = ROW_LENGTH);

  protected:
    int baseAddress;
    int endAddress;
    int pos;
    boolean eof;
};

#endif
