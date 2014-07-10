#include <Arduino.h>

class MyMetro{
  public:
    MyMetro(unsigned long period);
    void add(unsigned long ms);
    boolean check();
    void setPeriod(unsigned long period);
  private:
    unsigned long period;
    unsigned long last;
    unsigned long lastMillis;
    unsigned long myMillis;
};
