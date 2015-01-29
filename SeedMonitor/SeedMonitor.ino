#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
//#include <Statistic.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <TempSensor.h>
//#include <Thingspeak.h>
#include <SimpleConfig.h>
#include <SimpleConfigEeprom.h>
#include <Metro.h>
//#include <SD.h>
//#include <SdFat.h>
#include <MultiLog.h>
#include <MemoryFree.h>

#define LED 13

/*
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Data wire is plugged into pin 9 on the Arduino
 * Relays are on pin 5,6,7,8
*/
#define HEATER_PIN   5
#define LIGHT_PIN    6
#define ONE_WIRE_BUS 9

//SdFat SD;

/** Ethernet stuff
* MAC addres is read from simple config [ethernet] mac. You will find the MAC address on a label on you ethernet board
* The IP address will be dependent on your local network.
*/
// Initialize Arduino Ethernet Client
EthernetClient client;

#define BUFF_LEN 255
char buff[BUFF_LEN] = "";
unsigned int failedCounter = 0;
unsigned int successCounter = 0;
boolean configurationMode = false;

//TempSensor temp1 = TempSensor("field1", NULL);
//Thingspeak thingspeak = Thingspeak("");

Metro sendMetro = Metro(60000);

#define SERIALNO_ADDR 0x0000
#define CONFIG_URL_ADDR 0x0010
#define CONFIG_START 0x0080

SimpleConfigEeprom simpleConfig = SimpleConfigEeprom(CONFIG_START);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

uint8_t airTempAddr[8];
volatile float airTemp = -127;
float airTarget = 15;
uint8_t soilTempAddr[8];
volatile float soilTemp = -127;
float soilTarget = 12;
uint8_t ambientTempAddr[8];
volatile float ambientTemp = 20; // replace with sensor
float hysteres = 3;
float freezeAlarm = 5;
float heatAlarm = 25;
boolean lightOn = false; // Should light be on? Light is also used as extra heater
//boolean heatOn = false;

boolean lastConnected = false;

enum tempStatus_t {
  FREEZE_ALARM,
  HEATER_LIGHT,
  HEATER,
  GREAT,
  HEAT_ALARM,
};



tempStatus_t state = HEATER_LIGHT;
tempStatus_t lastState = state;

void setup()
{
  //Serial connection.
  Serial.begin(9600);
  noInterrupts(); // stäng av interrupts innan vi ändrar register
  mlog.setLevel(LOG_TRACE);
  mlog.INFO(F("====== SeedMonitor setup ====="));

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(HEATER_PIN, OUTPUT);
  switchHeat(true);
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, HIGH);
  //TempSensor::discoverOneWireDevices();
  mlog.INFO(F("Free memory:"));
  mlog.INFO(freeMemory(), false);

  mlog.INFO(F("Serial number:"));
  mlog.INFO(serialNumber(buff), false);

  simpleConfig.display();
  
  // Initialize Ethernet
  resetEthernetShield();

  discoverOneWireDevices();

  mlog.INFO(F("Initialize sensors"));
  sensors.begin();

  airTarget = simpleConfig.getFloat("airTarget", "temperature", 15);
  setAddress(airTempAddr, "airTempAddr");
  soilTarget = simpleConfig.getFloat("soilTarget", "temperature", 15);
  setAddress(soilTempAddr, "soilTempAddr");


  // Sätt controllregister för timer 1  TCCR1A TCCR1B
  // Den ska räkna pulser på D5 och ge interrupt
  // Nollställ först registren och sätt sen bitar
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= bit(CS10) | bit(CS12); // 1024 prescale
  TCCR1B |= bit(WGM12); // CTC - Nolställ vid match
  TIMSK1 = bit(TOIE1);  // enable Timer1 overflow interrupt:
  TCNT1 = 0x0000;  // nollställ timer1

  /*
  simpleConfig.getStr(buff, "apikey", "thingspek");
  thingspeak.setApiKey(buff);
  simpleConfig.getStr(buff, "temp1", "1wire");
  temp1.setAddress(buff);
  thingspeak.addSensor(&temp1);
  */
  interrupts(); // aktivera interrupt
}

void loop()
{
  if(Serial.available()){
    terminalHandler();
  }

  if ((airTemp > airTarget + hysteres) && (soilTemp > soilTarget + hysteres)){
    state = GREAT;
  }

  if (airTemp > heatAlarm){
    state = HEAT_ALARM;
  }

  //mlog.DEBUG((float) airTemp);
  if ((airTemp < airTarget) || (soilTemp < soilTarget)){
    state = HEATER;
  }

  //mlog.DEBUG((float) airTemp);
  if (airTemp < airTarget - hysteres){
    state = HEATER_LIGHT;
  }
  
  if (airTemp < freezeAlarm){
    state = FREEZE_ALARM;
  }
  
  if (state != lastState){
    mlog.INFO(F("Air temp:"));
    mlog.DEBUG((float) airTemp);
    mlog.INFO(F("Soil temp:"));
    mlog.DEBUG((float) soilTemp);
    lastState = state;
  }

  switch(state){
    case HEAT_ALARM:
      switchHeat(false);
      switchLight(false);
      break;

    case GREAT:
      switchHeat(false);
      if (lightOn) 
        switchLight(true);
      else 
        switchLight(false);
      break;

    case HEATER:
      switchHeat(true);
      if (lightOn) 
        switchLight(true);
      else 
        switchLight(false);
      break;
      
    case HEATER_LIGHT:
      switchHeat(true);
      switchLight(true);
      break;

    case FREEZE_ALARM:
      switchHeat(true);
      switchLight(true);
      break;
  }

  // Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();    
    client.stop();
  }

  
  if (sendMetro.check()){
    mlog.DEBUG(F("Time to send"));
    mlog.DEBUG(F("Air temp[C]: "));
    mlog.DEBUG((float) airTemp);
    mlog.DEBUG(F("Soil temp[C]: "));
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
    if (digitalRead(HEATER_PIN)){
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
    
    updateThingSpeak(buff);
  }
  lastConnected = client.connected();
}

void updateThingSpeak(char* tsData)
{
  mlog.DEBUG(F("Update thingspeak"));
  mlog.DEBUG(tsData);
  mlog.DEBUG((int) strlen(tsData));
  if (client.connect("api.thingspeak.com", 80))
  {
    client.print(F("POST /update HTTP/1.1\n"));
    client.print(F("Host: api.thingspeak.com\n"));
    client.print(F("Connection: close\n"));
    //client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print(F("Content-Type: application/x-www-form-urlencoded\n"));
    client.print(F("Content-Length: "));
    client.print(strlen(tsData));
    client.print(F("\n\n"));

    client.print(tsData);
    
    //lastConnectionTime = millis();
    
    if (client.connected())
    {
      mlog.DEBUG(F("Connecting to ThingSpeak..."));
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      mlog.INFO(F("Connection to ThingSpeak Failed, failedCounter: "));
      mlog.INFO((int) failedCounter, false);
    }
  }
  else
  {
    failedCounter++;
    
    mlog.INFO(F("Connection to ThingSpeak Failed, failedCounter: "));
    mlog.INFO((int) failedCounter, false);
    
    //lastConnectionTime = millis();
  }  
}


/**
* Reset ethernet shield
* All network settings are defined by DHCP
* MAC address is hard coded at top of this file
*/
void resetEthernetShield()
{
  byte mac[6];
  mlog.INFO(F("Resetting Ethernet Shield."));   
  
  client.stop();
  delay(1000);
  
   // start the Ethernet connection:
  mlog.INFO(F("Initialize ethernet..."));
  //simpleConfig.getStr(buff, "mac", "ethernet", "");
  simpleConfig.getStr(buff, "mac", "", "");
  char *saveptr;
  char * token;
  token = strtok_r(buff, ":-,", &saveptr);
  for (int i = 0; i < 6; i++){
    mac[i] = strtoul(token, NULL, 16);
    token = strtok_r(NULL, ":-,", &saveptr);
  }
  
  if (Ethernet.begin(mac) == 0) {
    mlog.WARNING(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  
  IPAddress myIPAddress = Ethernet.localIP();
  Serial.print("IP adress:"); 
  Serial.println(myIPAddress); 
}

void terminalHandler(){
  int i=0;
  char commandbuffer[80] = "";
  
  //Serial.println("Terminal handler activated");

  while( Serial.available() && i< 80) {
    commandbuffer[i++] = Serial.read();
    delay(50);
  }
  commandbuffer[i++]='\0';

  if (strstr(commandbuffer, "crash")){
    Serial.println(F("=== Entering WDT test mode ===="));
    Serial.println(F("Crashing"));
    int i = 0;
    while (1==1){
      Serial.println(i++);
      delay(1000);
    }
  }

  if (strstr(commandbuffer, "setserial")){
    Serial.println(F("=== Setting serial number ===="));
    unsigned int offset = 0;
    char * pch;
    pch = strstr(commandbuffer, "\"");
    if (pch != NULL){
      while((*++pch != 0x22) && (offset < CONFIG_URL_ADDR)){
        EEPROM.write(SERIALNO_ADDR + offset++, (char) *pch);
      }
      EEPROM.write(SERIALNO_ADDR + offset++, 0x00);
    }
    Serial.println(serialNumber(buff));
  }

  if (strstr(commandbuffer, "seturl")){
    Serial.println(F("=== Setting config url ===="));
    unsigned int offset = 0;
    char * pch;
    pch = strstr(commandbuffer, "\"");
    if (pch != NULL){
      while((*++pch != 0x22) && (offset < CONFIG_START)){
        EEPROM.write(CONFIG_URL_ADDR + offset++, (char) *pch);
      }
      EEPROM.write(CONFIG_URL_ADDR + offset++, 0x00);
    }
    Serial.println(configUrl(buff));
  }

  if (strstr(commandbuffer, "getconfig")){
    Serial.println(F("=== Get config from server ===="));
    //getConfig();
  }


  if (configurationMode && strlen(commandbuffer)){
    if (strstr(commandbuffer, "##simpleconfig end")){
      Serial.println(F("Closing config for write"));
      configurationMode = false;
      simpleConfig.close();
      simpleConfig.open();
      simpleConfig.display();
      //softwareReboot();
    } 
    else {
      simpleConfig.writeln(commandbuffer, 80);
      strcpy(commandbuffer, "");
    }
  } 
  else {
    if (strstr(commandbuffer, "##simpleconfig start")){
      Serial.println(F("Opening config for write"));
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
  simpleConfig.getStr(strAddress, key, "temperature");
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
    mlog.WARNING(F("Error TempSensor::setAddress empty or to short string"));
  }
}

void discoverOneWireDevices(void) {
  
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  char toHex[5] = "";
  
  mlog.INFO(F("Looking for 1-Wire devices..."));
  while(oneWire.search(addr)) {
    mlog.INFO(F("Found \'1-Wire\' device with address:"));
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
        mlog.WARNING(F("CRC is not valid!\n"));
        return;
    }
  }
  mlog.INFO(F("That was all devices"));
  oneWire.reset_search();
  return;
}

ISR (TIMER1_OVF_vect)
{
  TCNT1 = 0x0000; // nollställ timer1
  TIFR1 = 0x00; // nollställ timer1 interrupt flaggor
  airTemp = sensors.getTempC(airTempAddr);
  soilTemp = sensors.getTempC(soilTempAddr);
  sensors.requestTemperatures();
  //Serial.println(".");
  /*
  ++overflowCount2;  // räkna antalet timer 2 ovweflow

  if (overflowCount2 % 61 == 0){ // 61 gånger = 1 sekund
    // beräkna flöde senaste sekunden
    unsigned long currentCounter = totalCounter();
    currentFlow = 3600 * (currentCounter - lastCounter); // per timme
    lastCounter = currentCounter;
    digitalWrite(ledPin, digitalRead( ledPin ) ^ 1); // toggle led
  }
  */
}

void switchHeat(boolean on){
  if (digitalRead(HEATER_PIN) != on){
    if (on){
      digitalWrite(HEATER_PIN, HIGH);
    //heatOn = true;
      mlog.INFO("Heater ON");
    } else {
      digitalWrite(HEATER_PIN, LOW);
    //heatOn = false;
      mlog.INFO("Heater OFF");
    }
  }
}  

void switchLight(boolean on){
  if (digitalRead(LIGHT_PIN) != on){
    if (on){
      digitalWrite(LIGHT_PIN, HIGH);
    //heatOn = true;
      mlog.INFO("Light ON");
    } else {
      digitalWrite(LIGHT_PIN, LOW);
    //heatOn = false;
      mlog.INFO("LIGHT OFF");
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

