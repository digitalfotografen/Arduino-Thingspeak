#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SimpleConfig.h>
#include <SimpleConfigEeprom.h>
#include <MultiLog.h>
//#include <MemoryFree.h>

/*
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Data wire is plugged into pin 9 on the Arduino
 * Relays are on pin 5,6,7,8
*/
#define DOOR_PIN         2  // detect open door
#define LIGHT_PIN        7  // lights
#define HEAT_SOIL_PIN    6  // internal heater (heat cable) on/off
#define HEAT_AIR_PIN     5  // external heater, freeze guard
#define SPARE_PIN        8  // spare relay, no in use
#define ONE_WIRE_BUS     9  // temperature sensors

/** Ethernet stuff
* MAC addres is read from simple config [ethernet] mac. You will find the MAC address on a label on you ethernet board
* The IP address will be dependent on your local network.
*/
// Initialize Arduino Ethernet Client
EthernetClient client;
#define HOST "api.thingspeak.com"

#define BUFF_LEN 400
char buff[BUFF_LEN+1] = "";
unsigned int failedCounter = 0;
unsigned int successCounter = 0;
boolean configurationMode = false;

// EEPROM base address room
#define SERIALNO_ADDR 0x0000
#define CONFIG_URL_ADDR 0x0020
#define CONFIG_START 0x0080

SimpleConfigEeprom simpleConfig = SimpleConfigEeprom(CONFIG_START);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

uint8_t airTempAddr[8];
volatile float airTemp;
float airTarget, airMin, airMax;
uint8_t soilTempAddr[8];
volatile float soilTemp;
float soilTarget;
uint8_t ambientTempAddr[8];
volatile float ambientTemp;
//float ambientTarget;
float hysteres;
boolean lightOn = true; // Should light be on? Light is also used as extra heater
unsigned long updatePeriod = 60000;
unsigned long nextTime = millis() + updatePeriod;


boolean lastConnected = false;

enum tempStatus_t {
  FREEZE_ALARM,
  NORMAL,
  HEAT_ALARM,
};

tempStatus_t state = NORMAL;
tempStatus_t lastState = state;

void setup()
{
  //Serial connection.
  Serial.begin(9600);
  noInterrupts(); // stäng av interrupts innan vi ändrar register
  mlog.setLevel(LOG_TRACE);
  mlog.INFO(F("SeedBox setup"));

  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, HIGH);
  pinMode(HEAT_SOIL_PIN, OUTPUT);
  digitalWrite(HEAT_SOIL_PIN, HIGH);
  pinMode(HEAT_AIR_PIN, OUTPUT);
  digitalWrite(HEAT_AIR_PIN, HIGH);
  pinMode(SPARE_PIN, OUTPUT);
  digitalWrite(SPARE_PIN, LOW);

  mlog.INFO(F("Serial no:"));
  mlog.INFO(serialNumber(buff), false);
  mlog.INFO(F("Config url:"));
  mlog.INFO(configUrl(buff), false);

  discoverOneWireDevices();

  loadConfig();
  sensors.begin();

  // Sätt controllregister för timer 1  TCCR1A TCCR1B
  // Nollställ först registren och sätt sen bitar
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= bit(CS10) | bit(CS12); // 1024 prescale
  TCCR1B |= bit(WGM12); // CTC - Nolställ vid match
  TIMSK1 = bit(TOIE1);  // enable Timer1 overflow interrupt:
  TCNT1 = 0x0000;  // nollställ timer1
  interrupts(); // aktivera interrupt
  delay(1000); // to be nice to relaysjm
}

void loop()
{
  pinMode(DOOR_PIN, INPUT_PULLUP);

  if (failedCounter > 5){
    loadConfig(); // also calles ethernet reset
  }
  
  if(Serial.available()){
    terminalHandler();
  }

  state = NORMAL;
  if (airTemp > airMax){
    state = HEAT_ALARM;
  }
  if (airTemp < airMin){
    state = FREEZE_ALARM;
  }
  
  if (state != lastState){
    mlog.INFO(F("Air temp:"));
    mlog.DEBUG((float) airTemp);
    mlog.INFO(F("Soil temp:"));
    mlog.DEBUG((float) soilTemp);
    mlog.INFO(F("Ambient temp:"));
    mlog.DEBUG((float) ambientTemp);
    lastState = state;
  }
  
  if (lightOn)
    switchLight(true);
  else 
    switchLight(false);

  switch(state){
    case HEAT_ALARM:
      switchHeatSoil(false);
      switchHeatAir(false);
      break;

    case NORMAL:
      if (airTemp < airTarget){
        switchHeatAir(true);
      }
      if (airTemp > airTarget + hysteres){
        switchHeatAir(false);
      }
      if (soilTemp < soilTarget){
        switchHeatSoil(true);
      }
      if (soilTemp > soilTarget + hysteres){
        switchHeatSoil(false);
      }
      break;
      
    case FREEZE_ALARM:
      switchHeatSoil(true);
      switchHeatAir(true);
      switchLight(true);
      break;
  }

  
  if (millis() > nextTime){
    nextTime = millis() + updatePeriod;
    // get next command
    if (getTalkback()){
      int code = parseHttpResponse(buff, BUFF_LEN);
      mlog.DEBUG(F("Response:"));
      mlog.DEBUG((int) code);
      if (code != 200) {
        failedCounter++;
      } else {
        mlog.DEBUG(F("Body:"));
        mlog.DEBUG(buff,false);
        commandParser(buff);
      }
    }   

    // Prepare post data for send
    mlog.DEBUG(F("Time to send"));
    mlog.DEBUG(F("Air temp: "));
    mlog.DEBUG((float) airTemp);
    mlog.DEBUG(F("Soil temp: "));
    mlog.DEBUG((float) soilTemp);
    char temp[20] = "";
    strcpy(buff, "");
    catBuff("api_key=");
    catBuff( simpleConfig.get(temp, "apikey", "thingspeak") );
    catBuff("&field1=");
    dtostrf(soilTemp, 6, 2, temp);
    catBuff( trim(temp));
    catBuff("&field2=");
    dtostrf(airTemp, 6, 2, temp);
    catBuff( trim(temp));
    catBuff("&field3=");
    dtostrf(ambientTemp, 6, 2, temp);
    catBuff( trim(temp));
    catBuff("&field4=");
    if (digitalRead(HEAT_SOIL_PIN)){
      catBuff("1");
    } else {
      catBuff("0");
    }
    catBuff("&field5=");
    if (digitalRead(LIGHT_PIN)){
      catBuff("1");
    } else {
      catBuff("0");
    }
    catBuff("&field6=");
    if (digitalRead(HEAT_AIR_PIN)){
      catBuff("1");
    } else {
      catBuff("0");
    }
    catBuff("&field7=");
    if (digitalRead(DOOR_PIN)){
      catBuff("1");
    } else {
      catBuff("0");
    }
    
    // send values to Thingspeak
    if (updateThingSpeak(buff)){
      int code = parseHttpResponse(buff, BUFF_LEN);
      mlog.DEBUG(F("Response:"));
      mlog.DEBUG((int) code);
      if (code != 200) {
        failedCounter++;
      } else {
        mlog.DEBUG(F("Body:"));
        mlog.DEBUG(buff,false);
      }
    }
  }
  //lastConnected = client.connected();
}

boolean updateThingSpeak(char* tsData)
{
  mlog.DEBUG(F("Update thingspeak"));
  mlog.DEBUG(tsData);
  mlog.DEBUG((int) strlen(tsData));
  if (client.connect(HOST, 80)) {
    client.println(F("POST /update HTTP/1.1"));
    client.print(F("Host: "));
    client.println(HOST);
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    client.print(F("Content-Length: "));
    client.println(strlen(tsData));
    client.println();

    client.print(tsData);
    
    //lastConnectionTime = millis();
    
    if (client.connected()){
      //mlog.DEBUG(F("Connecting to ThingSpeak..."));
      
      failedCounter = 0;
      return true;
      
    }
  }
  failedCounter++;
  mlog.INFO(F("Update failed"));
  return false;
}

boolean getTalkback()
{
  char temp[20] = "";
  mlog.DEBUG(F("Get talkback"));
  strcpy(buff, "");
  catBuff("api_key=");
  catBuff( simpleConfig.get(temp, "apikey", "talkback") );

  if (client.connect(HOST, 80)) {
    client.print(F("POST /talkbacks/ "));
    client.print(simpleConfig.get(temp, "id", "talkback"));
    client.println(F("/commands/execute HTTP/1.1"));
    client.print(F("Host: "));
    client.println(HOST);
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    client.print(F("Content-Length: "));
    client.println(strlen(buff));
    client.println();
    client.print(buff);    

    if (client.connected())
    {
      mlog.DEBUG(F("Connecting to Talkback"));   
      failedCounter = 0;
      return true;
      
    }
  } 
  failedCounter++;
  mlog.INFO(F("Talkback Failed"));
  return false;
}


int parseHttpResponse(char *body, int maxLength){
  unsigned long timeout = millis() + 60000; // wiath max 60 seconds
  int rowLength = maxLength;
  int result = 0;
  char *tok;

  while ((client.available() < 10) && (millis() < timeout)){
    delay(100);
  }
  
  rowLength = client.readBytesUntil('\n', body, maxLength-1);
  body[rowLength] = 0x00;
  tok = strtok (body, " ");
  tok = strtok(NULL, " ");
  result = atoi(tok);
  
  if (result != 200){
    return result;
  }
  
  while (client.connected() && (rowLength > 1)  && (millis() < timeout)) {
    rowLength = client.readBytesUntil('\n', body, maxLength-1);
    body[rowLength] = 0x00;
  }
  
  if (millis() > timeout) {
    client.stop(); 
    return 0;
  }

  delay(500);
  if (client.connected()){
    rowLength = client.readBytes(body, maxLength);
    body[rowLength] = 0x00;
  }

  while (client.connected()){
    client.read();
  }
  client.stop();
  return result;  
}

/**
* Reset ethernet shield
* All network settings are defined by DHCP
* MAC address is hard coded at top of this file
*/
void resetEthernetShield()
{
  byte mac[6];
  //mlog.INFO(F("Resetting Ethernet Shield."));   
  
  client.stop();
  delay(1000);
  
   // start the Ethernet connection:
  mlog.INFO(F("Initialize ethernet..."));
  simpleConfig.getStr(buff, "mac");
  char *saveptr;
  char * token;
  token = strtok_r(buff, ":-,", &saveptr);
  for (int i = 0; i < 6; i++){
    mac[i] = strtoul(token, NULL, 16);
    token = strtok_r(NULL, ":-,", &saveptr);
  }
  
  if (Ethernet.begin(mac) == 0) {
    mlog.WARNING(F("DHCP failed"));
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
}

void terminalHandler(){
  int i=0;
  char commandbuffer[80] = "";
  
  while( Serial.available() && i< 80) {
    commandbuffer[i++] = Serial.read();
    delay(50);
  }
  commandbuffer[i++]='\0';
  commandParser(commandbuffer);
}

void commandParser(char *cmd){
  if (strstr(cmd, "setserial")){
    Serial.println(F("Setting serial number"));
    unsigned int offset = 0;
    char * pch;
    pch = strstr(cmd, "\"");
    if (pch != NULL){
      while((*++pch != 0x22) && (offset < CONFIG_URL_ADDR)){
        EEPROM.write(SERIALNO_ADDR + offset++, (char) *pch);
      }
      EEPROM.write(SERIALNO_ADDR + offset++, 0x00);
    }
    Serial.println(serialNumber(buff));
  }

  if (strstr(cmd, "seturl")){
    Serial.println(F("Setting config ur"));
    unsigned int offset = 0;
    char * pch;
    pch = strstr(cmd, "\"");
    if (pch != NULL){
      while((*++pch != 0x22) && (offset < CONFIG_START)){
        EEPROM.write(CONFIG_URL_ADDR + offset++, (char) *pch);
      }
      EEPROM.write(CONFIG_URL_ADDR + offset++, 0x00);
    }
    Serial.println(configUrl(buff));
  }

  if (strstr(cmd, "getconfig")){
    Serial.println(F("Get config"));
    if (getConfig()){
      loadConfig();
    }
  }

  if (strstr(cmd, "lighton")){
    lightOn = true;
    switchLight(lightOn);
  }

  if (strstr(cmd, "lightoff")){
    lightOn = false;
     switchLight(lightOn);
  }

  if (configurationMode && strlen(cmd)){
    if (strstr(cmd, "##simpleconfig end")){
      Serial.println(F("Closing config"));
      configurationMode = false;
      simpleConfig.close();
      simpleConfig.open();
      simpleConfig.display();
      //softwareReboot();
    } 
    else {
      simpleConfig.writeln(cmd, 80);
      strcpy(cmd, "");
    }
  } 
  else {
    if (strstr(cmd, "##simpleconfig start")){
      Serial.println(F("Opening config"));
      configurationMode = true;
      simpleConfig.close(); 
      simpleConfig.open(true);
    }
  }
}


char* serialNumber(char *buff){
  int i = 0;
  char c = 0x00;
  do{
    buff[i] = (char) EEPROM.read(SERIALNO_ADDR + i);
    c = buff[i];
  } while ((c != 0x00) && (i++ < CONFIG_URL_ADDR));
  buff[i] = 0x00;
  return buff;
}

char* configUrl(char *buff){
  int i = 0;
  char c = 0x00;
  do{
    buff[i] = (char) EEPROM.read(CONFIG_URL_ADDR + i);
    c = buff[i];
  } while ((c != 0x00) && (i++ < CONFIG_START));
  buff[i] = 0x00;
  return buff;
}

boolean setAddress(uint8_t *addr, const char *key){
  mlog.DEBUG(key);
  char strAddress[40] = "";
  simpleConfig.getStr(strAddress, key, "temp");
  mlog.DEBUG(strAddress);
  if (strlen(strAddress) > 23){
    int pos = 0;
    char * tok;
    tok = strtok (strAddress," ,.\t");
    while ((pos < 8) && (tok != NULL)){
      addr[pos++] = (byte) strtoul(tok, NULL, 16);
      tok = strtok(NULL," ,.\t");
    }
  } else {
    mlog.WARNING(F("Sensor Addr empty or to short"));
  }
}

void discoverOneWireDevices(void) {
  
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  char toHex[5] = "";
  
  mlog.INFO(F("Search 1-Wire devices..."));
  oneWire.reset_search();
  while(oneWire.search(addr)) {
    mlog.INFO(F("Found:"));
    for( i = 0; i < 8; i++) {
      mlog.INFO(F("0x"), false);
      if (addr[i] < 16) {
        mlog.INFO(F("0"), false);
      }
      itoa(addr[i], toHex, 16);
      mlog.INFO(toHex, false);
      if (i < 7) {
        mlog.INFO(F(", "), false);
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        mlog.WARNING(F("CRC not valid!"));
        return;
    }
  }
  mlog.INFO(F("End"));
  oneWire.reset_search();
  return;
}

ISR (TIMER1_OVF_vect)
{
  TCNT1 = 0x0000; // nollställ timer1
  TIFR1 = 0x00; // nollställ timer1 interrupt flaggor
  sensors.requestTemperatures();
  airTemp = sensors.getTempC(airTempAddr);
  soilTemp = sensors.getTempC(soilTempAddr);
  ambientTemp = sensors.getTempC(ambientTempAddr);
}

void switchHeatSoil(boolean on){
  if (digitalRead(HEAT_SOIL_PIN) != on){
    if (on){
      digitalWrite(HEAT_SOIL_PIN, HIGH);
      mlog.INFO(F("Soil Heat ON"));
    } else {
      digitalWrite(HEAT_SOIL_PIN, LOW);
      mlog.INFO(F("Soil Heat OFF"));
    }
  }
}  

void switchLight(boolean on){
  if (digitalRead(LIGHT_PIN) != on){
    if (on){
      digitalWrite(LIGHT_PIN, HIGH);
      mlog.INFO(F("Light ON"));
    } else {
      digitalWrite(LIGHT_PIN, LOW);
      mlog.INFO(F("LIGHT OFF"));
    }
  }
}

/* Control external heater
*/
void switchHeatAir(boolean on){
  if (digitalRead(HEAT_AIR_PIN) != on){
    if (on){
      digitalWrite(HEAT_AIR_PIN, HIGH);
      mlog.INFO(F("Ext heat ON"));
    } else {
      digitalWrite(HEAT_AIR_PIN, LOW);
      mlog.INFO(F("Air heat OFF"));
    }
  }
}  


boolean catBuff(const char *str){
  int space = BUFF_LEN - strlen(buff) - 1; 
  if (str && (space > strlen(str))){
    strcat(buff, str);
    return true;
  } else {
    return false;
  }
}

/**
 * trim string from white space
 * Manipulates original char array
*/

char *trim(char *str)
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


/*
 * Fetch new config
 */
int getConfig()
{
  char *tok;
  mlog.INFO(F("getConfig"));
  //mlog.DEBUG(F("Free memory:"));
  //mlog.DEBUG(freeMemory(), false);

  configUrl(buff);
  tok = strtok(buff, "/");

  if (client.connect(tok, 80)) {
    mlog.DEBUG(F("connected"));
    client.print(F("GET /"));  
    tok = strtok(NULL, "");
    client.print(tok);
    client.print(F("/"));

    serialNumber(buff);
    strcat(buff, ".cfg");
    client.print(buff);
    client.println(F(" HTTP/1.1"));

    configUrl(buff);
    tok = strtok(buff, "/");
    client.print(F("Host: "));
    client.println(tok);
    
    client.print(F("Connection: close\r\n\r\n"));

    delay(1000);
    if (client.connected())
    {
      mlog.DEBUG(F("Connect to server..."));
      int code = parseHttpResponse(buff, BUFF_LEN);
      mlog.DEBUG(F("Response code"));
      mlog.DEBUG((int) code);

      mlog.INFO(F("Opening config for write"));
      simpleConfig.close();
      simpleConfig.open(true);

      tok = strtok (buff, "\r\n");
      while (tok) {
        simpleConfig.writeln(tok, 80);
        tok = strtok (NULL, "\n\r");
      }
      simpleConfig.close();

      return true;
      
    }
  }
  mlog.INFO(F("Connection to configserver Failed"));
  return false;
}

/*
* Load config and initialize global variables
*/
void loadConfig(){
  simpleConfig.open(false);
  simpleConfig.display();

  airTarget = simpleConfig.getFloat("airTarget", "temp", 15);
  airMin = simpleConfig.getFloat("airMin", "temp", 10);
  airMax = simpleConfig.getFloat("airMax", "temp", 30);
  setAddress(airTempAddr, "airTempAddr");
  soilTarget = simpleConfig.getFloat("soilTarget", "temp", 15);
  setAddress(soilTempAddr, "soilTempAddr");
  //ambientTarget = simpleConfig.getFloat("ambTarget", "temp", 5);
  setAddress(ambientTempAddr, "ambTempAddr");
  hysteres = simpleConfig.getFloat("hysteres", "temp", 3);
  updatePeriod = 1000 * (unsigned long) simpleConfig.getInt("updateperiod", "", 300);
  resetEthernetShield();
}
