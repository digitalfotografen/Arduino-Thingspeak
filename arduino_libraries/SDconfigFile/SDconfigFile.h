#if !defined(__SD_CONFIG_FILE_H__)
#define __SD_CONFIG_FILE_H__ 1

#include "Arduino.h"
#include <SD.h>

class SDconfigFile {
  public:
    SDconfigFile(char *filename);
    // default ss and cs pins for SD card shield
    boolean begin(int ss = 10, int cs = 4); 
    boolean open();
    void close();
    int readln(char *buff,int maxlen = 256);
    char* get(char *value, char *key, char* group = "");
    boolean available();

  private:
    File file;
    char *filename;
    boolean opened;
    static boolean initialised;
};

#endif
