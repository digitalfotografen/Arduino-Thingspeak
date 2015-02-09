#include "Sensor.h"

int Sensor::sensorPowerPin = -1;


Sensor::Sensor(const char *_label){
  if (_label != NULL){
    strcpy(this->label, _label);
  } else {
    strcpy(this->label, "");
  }
  strcpy(this->labelMin, "");
  strcpy(this->labelMax, "");
  statistic.clear();
  this->prepareTime = 0;
  this->setRangeOut(0,1023);
  this->setRangeIn(0,1023);
}

unsigned long Sensor::prepare(){
  unsigned long t = this->prepareTime;
  if (!sensorPower && (sensorPowerPin >= 0)){
    mlog.DEBUG(F("sensor power on"));
    digitalWrite(sensorPowerPin, HIGH);
    t+= 50;
    sensorPower = true;
  }
  return t;
}

void Sensor::setPrepareTime(unsigned long t){
  this->prepareTime = t;
}

void Sensor::measure(){
}

void Sensor::putValue(int number){
  this->last = (float) number;
  this->statistic.add((float) number);
}

void Sensor::putValue(float number){
  this->last = number;
  this->statistic.add(number);
}

float Sensor::getLast(){
  return this->last;
}

void Sensor::sleep(){
  if (sensorPower && (sensorPowerPin >= 0)){
    mlog.DEBUG(F("sensor power off"));
    digitalWrite(sensorPowerPin, LOW);
    sensorPower = false;
  }
}

float Sensor::map(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}    

void Sensor::toString(char *buff, int maxLength, SensorValue valueType){
  float value;
  switch (valueType){
    case SENSOR_LAST:
      value = this->last;
      break;

    case SENSOR_MIN:
      value = this->statistic.minimum();
      break;
      
    case SENSOR_MAX:
      value = this->statistic.maximum();
      break;
      
    case SENSOR_AVERAGE:
    default:
      value = this->statistic.average();
      break;
    
  }
  dtostrf(value, maxLength, 2, buff);
  this->trim(buff);
}

/**
 * trim string from white space
 * Manipulates original char array
*/

char * Sensor::trim(char *str)
{
    size_t len = 0;
    char *frontp = str - 1;
    char *endp = NULL;

    if( str == NULL )
            return NULL;

    if( str[0] == '\0' )
            return str;

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address
     * the first non-whitespace characters from
     * each end.
     */
    while( isspace(*(++frontp)) );
    while( isspace(*(--endp)) && endp != frontp );

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so
     * that if it's dynamically allocated, we can
     * still free it on the returned pointer.  Note
     * the reuse of endp to mean the front of the
     * string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) *endp++ = *frontp++;
            *endp = '\0';
    }


    return str;
}

char* Sensor::getLabel(){
  if (!strlen(this->label)){
    return NULL;
  }
//  strcpy(buff, this->label);
  return this->label;
}

void Sensor::setLabel(const char* buff){
  strcpy(this->label, buff);
}

void Sensor::getLabelMin(char* buff){
  strcpy(buff, this->labelMin);
}

void Sensor::setLabelMin(const char* buff){
  strcpy(this->labelMin, buff);
}

void Sensor::getLabelMax(char* buff){
  strcpy(buff, this->labelMax);
}

void Sensor::setLabelMax(const char* buff){
  strcpy(this->labelMax, buff);
}

/*Set output range for mapping av measured value
*/
void Sensor::setRangeOut(float rangeMin, float rangeMax){
  this->rangeOutMin = rangeMin;
  this->rangeOutMax = rangeMax;
}

/*Set input range for mapping av measured value
*/
void Sensor::setRangeIn(float rangeMin, float rangeMax){
  this->rangeInMin = rangeMin;
  this->rangeInMax = rangeMax;
}

void Sensor::clear(){
  this->statistic.clear();
}