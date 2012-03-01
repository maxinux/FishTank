////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Fish Tank 
// Monitors and controls lights on one fish tank
// Libraries Used:
// http://www.pjrc.com/teensy/td_libs_OneWire.html
// http://milesburton.com/Dallas_Temperature_Control_Library
// Thanks to Skippy for help with the json and charting stuff
////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#define APPNAME "MaxPeppr Aquarium Controller, v 0.1a"
#define LICENSE "GPLv3 / CopyRight 02/29/2012 - wtiemann@me.com"

#include <DallasTemperature.h>  // DS18B20
#include <OneWire.h>    // Dallas 1-Wire / DS18B20
#include <Wire.h>       // RTC One Wire Protocol
#include <RTClib.h>     // RTC Library
#include <DHT11.h>      // Room Temp
#include <SPI.h>        // SinglePinInterface
#include <Ethernet.h>   // Ethernet and TCP/IP (and UDP)
#include <HTTPClient.h> // Lazy HTTP Requests
#include "config.h" 

// Begin Definitions, starting with OneWire Addresses
#define ADDR1 "28:A0:CA:6F:03:00:00:33"
#define ADDR2 "28:21:80:A3:03:00:00:91"
#define ADDR3 "28:E7:73:41:03:00:00:1A"

#define ONE_WIRE_BUS 2  // Where are the DS18B20's plugged in 
#define SENSORS         // OneWire Sensors
#define DHT11PIN A0     // Where is the DHT11
#define DAYLIGHTPIN 5   // Day Light Transistor control of 12v lights
#define NIGHTLIGHTPIN 6 // Night Light control
#define DAYLEVEL 96     // Bright lights
#define NIGHTLEVEL 32   // Night light

// Configure Ethernet
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Made Up MAC
static uint8_t gwip[4] = {192,168,1,1};                // GW IP
IPAddress server(216,129,119,242);                     // Nizzles
IPAddress ip(192,168,1,15);                            // Arduino Wiz IP
EthernetClient client;

//OneWire, RTC, DHT11
OneWire oneWire(ONE_WIRE_BUS);          // Construct One Wire Bus for Dallas/Maxim ICs and more 
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature
DeviceAddress insideThermometer0, insideThermometer1;  // Thermometers
RTC_DS1307 RTC;                         // Clock
dht11 DHT11;                            // Room Temperature/Humidity

// Lights
byte dayLight, dayLightState, nightLight, nightLightState, nightDirection;

///////////////////// Begin Program, Starting with Setup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void setup(void)
{
  // start serial port
  Serial.begin(57600); // For BT go down to 9600
  Serial.print(APPNAME);
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  if (!sensors.getAddress(insideThermometer0, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(insideThermometer1, 1)) Serial.println("Unable to find address for Device 0");   
 
  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer0);
  sensors.setResolution(insideThermometer0, 9);
  Serial.print(" Resolution: ");  
  Serial.print(sensors.getResolution(insideThermometer0), DEC); 
  Serial.println();
  
  Serial.print("Device 1 Address: ");
  printAddress(insideThermometer1);
  sensors.setResolution(insideThermometer1, 9);
  Serial.print(" Resolution: ");  
  Serial.print(sensors.getResolution(insideThermometer1), DEC); 
  Serial.println();

  Serial.print("Updating every 10 seconds...");
  Serial.println();
  // Initialize RTC
  Wire.begin();
  RTC.begin();
  // Initialize Ethernet
  Ethernet.begin(mac, ip, gwip);
  // Begin Light prep
  pinMode(DAYLIGHTPIN, OUTPUT);
  pinMode(NIGHTLIGHTPIN, 0);      
  analogWrite(NIGHTLIGHTPIN, 0);  
}

//////////////////////// Begin functions that other stuff calls \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
// function to print the temperature for a device
float getTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  return tempC;
}

// function to print a OneWire address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

float getRoomTemp(void) { 
  int chk = DHT11.read(DHT11PIN); 
  float roomTemp=(float)DHT11.temperature;
  switch (chk) {
    case 0:  break;
    case -1: Serial.println("Checksum error"); break;
    case -2: Serial.println("Time out error1"); break;
    default: Serial.println("Unknown error"); break;
  }
  return roomTemp;
}

int getRoomHumidity(void) {  
  int chk = DHT11.read(DHT11PIN);
  float roomHumidity= (float)DHT11.humidity ;
  delay(300);
  switch (chk) {
    case 0:  break;
    case -1: Serial.println("Checksum error"); break;
    case -2: Serial.println("Time out error2"); break;
    default: Serial.println("Unknown error"); break;
  }
  return roomHumidity;
}

void jsonPush(float tankTemp0, float tankTemp1, float roomTemp, int roomHumidity) {
  char output[16];
  HTTPClient hclient("nizzles.net",mserver);
  String json = "";
  
  hclient.debug(true);
    
  json += "{\"tankTemp0\":";  
    dtostrf(tankTemp0, 10, 5, output); 
    json += output;
  
    json += ",\"tankTemp1\":";  
    dtostrf(tankTemp1, 10, 5, output); 
    json += output;
  
    json += ",\"roomTemp\":";  
    dtostrf(roomTemp, 10, 5, output); 
    json += output;
  
    json += ",\"roomHumidity\":";  
    dtostrf(roomHumidity, 10, 5, output); 
    json += output;
  json += "}\n";

  char data[json.length()];
  json.toCharArray(data, json.length());

  http_client_parameter parameters[] = {{"data",data},{NULL,NULL}};

  FILE* result = hclient.getURI(URISTEM, parameters, NULL);

  int returnCode = hclient.getLastReturnCode();

  if (result!=NULL) {
    hclient.closeStream(result);
  } 
  else {
    Serial.println("failed to connect");
  }

  if (returnCode==200) {
    Serial.println("data uploaded");
  } 
  else {
    Serial.print("ERROR: Server returned ");
    Serial.println(returnCode);
  }
}

void checkLights(byte hour, byte phase)
{
  Serial.println(hour);
  if (hour > 8 && hour <  20)
    dayLight = DAYLEVEL;
  else {
       dayLight = 0;
       nightLight = ((NIGHTLEVEL*phase)+5);    
  }
  if (dayLight != dayLightState){
    dayLightState=dayLight;
    analogWrite(DAYLIGHTPIN, dayLightState);
  }
   if (nightLight != nightLightState){
    nightLightState=nightLight;
    analogWrite(NIGHTLIGHTPIN, nightLightState);
  }
}
  
byte MoonPhase(int days)
{
  byte phase=days%4;
  return phase;
}

void loop(void)
{   
  DateTime now = RTC.now(); //Get new time
  sensors.requestTemperatures(); // Request Temps so we can get them

  jsonPush(getTemperature(insideThermometer0), getTemperature(insideThermometer1),  getRoomTemp(),       getRoomHumidity() );
  checkLights(now.hour(), MoonPhase(now.day()));
  delay(29400); // Wait 30s after two DHT pauses and sensor reads
}


