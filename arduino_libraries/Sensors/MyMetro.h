#include <Arduino.h>

class MyMetro{
  public:
    MyMetro(unsigned long period);
    void add(unsigned long ms);
    boolean check();
    void setPeriod(unsigned long period);
    unsigned long getPeriod();
    void reset();
  private:
    volatile unsigned long period;
    volatile unsigned long last;
    volatile unsigned long lastMillis;
    volatile unsigned long myMillis;
};
