/**
* Reads values from multiple 1-wire DS18B20 temperature sensors and stores data at ThingSpeak
* Used to monitor temperatures in our water pipe and well
* Controls a heater depending on the lowest read temperature.
* © Ulrik Södergren 2013
* The latest source code and more samples is available at https://github.com/digitalfotografen/Arduino-Thingspeak
*
* Inspired by the following tutorials and code samples
* https://github.com/iobridge/ThingSpeak-Arduino-Examples/blob/master/Ethernet/Arduino_to_ThingSpeak.ino
* http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html
* http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
*
* I have modified the code after article was sent to Datormagazin. 
* The main reason was to reduce the risk of buffer overflows using char buffers. Instead I now use Strings objects except for float to String conversion.
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>
#include <stdlib.h>

/*
 * ThingSpeak Settings
 * Register a free account on www.thingspeak.com and create a channel
 * Replace writeAPIKey below with your channel API Write key before testing.
 */
String writeAPIKey = "put key here";    
char thingSpeakAddress[] = "api.thingspeak.com";
#define updateThingSpeakInterval 600000 // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
#define tempReadInterval 10000 // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

/** Ethernet stuff
* Enter a MAC address for your controller below. You will find it on a label on you ethernet board
* The IP address will be dependent on your local network.
*/
byte mac[] = {  
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB
};
// Initialize Arduino Ethernet Client
EthernetClient client;

// Number of sensors used
#define NUM_SENSORS 4

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html
// modify this to your senores or this will not work
// values below are only a sample and reads the same sensor 4 times. You should have diffrent addresses on each row.
static uint8_t adresses[NUM_SENSORS][8] = {
  { 0x28, 0xDC, 0xC2, 0xB0, 0x04, 0x00, 0x00, 0x09 }, // temp 0 inner hull
  { 0x28, 0xDC, 0xC2, 0xB0, 0x04, 0x00, 0x00, 0x09 }, // temp 1 Outer hull 
  { 0x28, 0xDC, 0xC2, 0xB0, 0x04, 0x00, 0x00, 0x09 }, // temp 2 Ground
  { 0x28, 0xDC, 0xC2, 0xB0, 0x04, 0x00, 0x00, 0x09 }  // temp 3 Pump house
};

/*
 Circuit definition:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Data wire is plugged into pin 3 on the Arduino
 * Heat cable realy control is connected to pin 4 and normally closed (high is off)
*/
#define HEATCABLE 4
#define ONE_WIRE_BUS 3
#define INTERNAL_LED 13

// activate heater if the lowesest temperature is below this value
#define THERMOSTAT_TEMP 2.0
// deactive heater if the lowesest temperature is above THERMOSTAT_TEMP+THERMOSTAT_HYSTERES
#define THERMOSTAT_HYSTERES 1.0  


/** 
* Initate array of temperature readings
* we don't really need more than the number of used sensors, but Thingspeak supports maximum 8 fields
* -127.0 inidicates an error
*/
static float temps[] = 
{
  -127.0, 
  -127.0, 
  -127.0, 
  -127.0,
  -127.0, 
  -127.0, 
  -127.0,
  -127.0
};

/* 
* Attribute names for Thingspeak api call
* we don't really need more than the number of used sensors, but Thingspeak supports maximum 8 fields
*/
const static char* fields[] = 
{
   "field1",
   "field2",
   "field3",
   "field4",
   "field5",
   "field6",
   "field7",
   "field8"
};

/** 
* Labels for presentation i Serial monitor
* Adjust to your own implementation
*/
const char* labels[] = 
{
  "Inner hull",
  "Outer hull",
  "Ground",
  "Pump house",
  "Heat cable",
  "6",
  "7",
  "8"
};

// definitions specific for choosing fields to controll heater
#define INNER_HULL 0
#define PUMP_HOUSE 3

// Variable Setup
unsigned long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
unsigned long lastTempUpdate = 0;

/**
* Initate 1-wire devices
* Setup a oneWire instance to communicate with any OneWire devices
* Pass our oneWire reference to Dallas Temperature.
*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// setup and initialization
void setup(void)
{
  // start serial port
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Setup start");

  Serial.println("Initialize sensors");
  sensors.begin();
  // initialize all channels
  for (int i=0; i < NUM_SENSORS; i++){
     sensors.setResolution(adresses[i], 12); // use 12 bit resolution
  }

  pinMode(HEATCABLE, OUTPUT);
  digitalWrite(HEATCABLE, LOW); // turn relay off = heat on
  Serial.println("Heatcable on");
  temps[HEATCABLE] = 1;
  pinMode(INTERNAL_LED, OUTPUT);
  
  // Initialize Ethernet
  resetEthernetShield();
  
  Serial.println("Setup finished");
}

/**
* This is the Arduino main loop
* Delays are used to slow things down during debug, it still fast enoght for normal operation
* Printing of http-response is slowed down to reduce risk of buffer overflow in Serial Monitor
*/
void loop(void)
{ 
  delay(100);
  
  // update array of temp readings
  if (millis() > (lastTempUpdate + tempReadInterval))
  {
    Serial.println("Getting temperatures...\n\r");
    sensors.requestTemperatures();
    for (int i=0; i < NUM_SENSORS; i++){
      float t = sensors.getTempC(adresses[i]);
      if (t == -127.00) {
        temps[i] = -127.00;
        Serial.print("Error reading sensor on channel ");
        Serial.println(labels[i]);
      } else {
        if (temps[i] !=  -127.00){
          temps[i] = (temps[i] + t) / 2; //average with last
        } else {
          temps[i] = t;
        } 
        Serial.print(labels[i]);
        Serial.print(" ");
        Serial.print(temps[i]);
        Serial.println("C");
        lastTempUpdate = millis();
      }
    }
    delay(100);
  }
 
  // we use temp readings from pump house and inner hull to control heat cable
  // originally we used all sensors, but found out that using ground and outer hull temepratures
  // is not important in our use.
  float tempLow = array_min(temps, NUM_SENSORS);
  //float tempLow = min(temps[INNER_HULL], temps[PUMP_HOUSE]);
  
  // check if temp low and turn on heatcable
  if (tempLow < THERMOSTAT_TEMP){
    if (digitalRead(HEATCABLE)){
      digitalWrite(HEATCABLE, LOW); // turn on
      temps[HEATCABLE] = 1;
      Serial.println("Heatcable on");
    }
  }
  
  // check if temp above hysteres and turn off heatcable
  if (tempLow > THERMOSTAT_TEMP + THERMOSTAT_HYSTERES){
    if (!digitalRead(HEATCABLE)){
      digitalWrite(HEATCABLE, HIGH); // turn off
      temps[HEATCABLE] = 0;
      Serial.println("Heatcable off");
    }
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

 
  // Update ThingSpeak  
  if(!client.connected() && (lastTempUpdate > (lastConnectionTime + updateThingSpeakInterval)))
  {
    Serial.println("Create field string");
    String s = "";
    char charBuffer[20] = "";

    // Create parameter string
    // Modified to use String objects instead of tradiononal char arrays to minimize risk of buffer overflows
    for (int i=0; i <= NUM_SENSORS; i++){
      if (temps[i] != -127.00){
             
        if (i > 0){
          s.concat("&");
        }

        s.concat(fields[i]);
        s.concat("=");        
    
        // here all values are floats and the Arduino sprinf() functions doesn't support floats.
        // this is a an uggly workaround.
        dtostrf(temps[i], 6, 2, charBuffer);
        String v = String(charBuffer);
        v.trim();
        s.concat(v);
      }
    }
    
    delay(100);
    updateThingSpeak(s);
  }

  // Check if Arduino Ethernet needs to be restarted  
  if (failedCounter > 3 ) {resetEthernetShield();}

  lastConnected = client.connected();
}

/**
* Reset ethernet shield
* All network settings are defined by DHCP
* MAC address is hard coded at top of this file
*/
void resetEthernetShield()
{
  Serial.println("Resetting Ethernet Shield.");   
  Serial.println();
  
  client.stop();
  delay(1000);
  
   // start the Ethernet connection:
  Serial.println("Initialize ethernet...");

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  IPAddress myIPAddress = Ethernet.localIP();
  Serial.print("IP adress:"); 
  Serial.println(myIPAddress); 
}

/**
* Send data with http-post to Thingspeak
* Authenticateion is performed via API write key
*/
void updateThingSpeak(String tsData)
{
  Serial.println("Update thingspeak");
  Serial.println(tsData);
  Serial.println(tsData.length());
  if (client.connect(thingSpeakAddress, 80))
  {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
    Serial.println();
    
    lastConnectionTime = millis();
  }  
}

/**
* Find lowest value in an array of floats 
*/
float array_min(float *values, int length){ 
  float result = __builtin_inf();
  for (int i=0; i < length; i++){
    result = min(result, values[i]);
  }
  return result;
}


