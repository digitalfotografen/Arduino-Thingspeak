#include "Sensor.h"

Sensor::Sensor(char *_label){
  strcpy(this->label, _label);
  strcpy(this->labelMin, "");
  strcpy(this->labelMax, "");
  statistic.clear();
}

unsigned long Sensor::prepare(){
  return 0;
}

void Sensor::measure(){
}

void Sensor::sleep(){
}

float Sensor::map(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}    

void Sensor::toString(char *buff, int maxLength, SensorValue valueType){
  float value;
  switch (valueType){
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

void Sensor::getLabel(char* buff){
  strcpy(buff, this->label);
}

void Sensor::getLabelMin(char* buff){
  strcpy(buff, this->labelMin);
}

void Sensor::setLabelMin(char* buff){
  strcpy(this->labelMin, buff);
}

void Sensor::getLabelMax(char* buff){
  strcpy(buff, this->labelMax);
}

void Sensor::setLabelMax(char* buff){
  strcpy(this->labelMax, buff);
}

void Sensor::clear(){
  statistic.clear();
}