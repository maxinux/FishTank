////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\
// Fish Tank 
// Monitors and controls lights on one fish tank
// Libraries Used:
// http://www.pjrc.com/teensy/td_libs_OneWire.html
// http://milesburton.com/Dallas_Temperature_Control_Library
//
// Thanks to Skippy for help with the json and charting stuff
// PH PRobe info to be added some time: 
//     https://www.atlas-scientific.com/index.php/store#ecwid:category=1812609&mode=product&product=7766829
//
////////////////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\

#define APPNAME "MaxPeppr Aquarium Controller, v 0.1a"
#define LICENSE "GPLv3 / CopyRight 02/29/2012 - wtiemann@me.com"

#include <DallasTemperature.h>  // DS18B20
#include <OneWire.h>    // Dallas 1-Wire / DS18B20
#include <Wire.h>       // RTC One Wire Protocol
#include <RTClib.h>     // RTC Library
#include <SPI.h>        // SinglePinInterface
#include <Ethernet.h>   // Ethernet and TCP/IP (and UDP)
#include <HTTPClient.h> // Lazy HTTP Requests
#include "config.h"     // Configuration options

//Constructors
OneWire oneWire(ONE_WIRE_BUS);          // Construct One Wire Bus for Dallas/Maxim ICs and more 
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature
DeviceAddress tankThermometer, roomThermometer;  // Thermometers
EthernetClient client;
RTC_DS1307 RTC;                         // Clock

// Lights
byte dayLight, dayLightState, nightLight, nightLightState=0, nightDirection=0;

///////////////////// Begin Program, Starting with Setup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void setup(void)
{
  // start serial port
  Serial.begin(57600); // For BT go down to 9600
  Serial.println(APPNAME);

  sensors.begin();
  if (!sensors.getAddress(tankThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(roomThermometer, 1)) Serial.println("Unable to find address for Device 1");   
 
  Serial.println("Updating every 10 seconds...");
  
  // Initialize RTC
  Wire.begin();
  RTC.begin();
  
  // Initialize Ethernet
  Ethernet.begin(mac, ip, gwip);
  
  // Begin Light prep
  pinMode(DAYLIGHTPIN, OUTPUT);
  pinMode(NIGHTLIGHTPIN, 0);      
  analogWrite(DAYLIGHTPIN, 0);  
  analogWrite(NIGHTLIGHTPIN, 0);  
}

//////////////////////// Begin functions that other stuff calls \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void jsonPush(float tankTemp0, float roomTemp) {
  char output[16];
  HTTPClient hclient(WEBSERVER, mserver);
  String json = "";
  
  //hclient.debug(true);
    
  json += "{\"tankTemp0\":";  
    dtostrf(tankTemp0, 10, 5, output); 
    json += output;
  
    json += ",\"roomTemp\":";  
    dtostrf(roomTemp, 10, 5, output); 
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
    Serial.println("Failed to connect!");
  }

  if (returnCode==200) {
    Serial.println("Data uploaded.");
  } 
  else {
    Serial.print("ERROR! Server returned:");
    Serial.println(returnCode);
  }
}

void checkLights(byte hour, byte phase)
{
  if (hour > 8 && hour <  20){
    nightLight = 0;
    dayLight = DAYLIGHTLEVEL;
  }
  else {
       dayLight = 0;
       nightLight = NIGHTLIGHTLEVEL/phase;    
  }
  if (dayLight != dayLightState){
    dayLightState=dayLight;
    analogWrite(DAYLIGHTPIN, dayLightState);
  }
   if (nightLight != nightLightState){
    nightLightState=nightLight/phase;
    analogWrite(NIGHTLIGHTPIN, nightLightState);
  }
}
  
byte MoonPhase(int days)
{
  byte phase=days%4+1;
 // return phase;
 return 2;
}

/* // http://technology-flow.com/articles/aquarium-lights/
// Put 4.7k resistor between transistor and arduino
int moonPhase(int moonYear, int moonMonth, int moonDay)
{
  int dayFromYear, dayFromMonth;
  double julianDay;
  int phase;
 
  if (moonMonth < 3)			//keep the month before march
  {
    moonYear--;			//take away a year
    moonMonth += 12;		//add an extra 12 months (the year taken away from before)
  }
  ++moonMonth;
  dayFromYear = 365.25 * moonYear; //get days from current year
  dayFromMonth = 30.6 * moonMonth; //get number of days from the current month
  julianDay = dayFromYear + dayFromMonth + moonDay - 694039.09; //add them all  
  julianDay /= 29.53;		//divide by the length of lunar cycle
  phase = julianDay;		//take integer part
  julianDay -= phase;		//get rid of the int part
  phase = julianDay*8 + 0.5;	//get it between 0-8 and round it by adding .5
  phase = phase & 7;		//get a number between 1-7
  return phase; 		//1 == new moon, 4 == full moon
}*/

void loop(void)
{   
  DateTime now = RTC.now(); //Get new time
  sensors.requestTemperatures(); // Request Temps so we can get them

  jsonPush(sensors.getTempC(tankThermometer), sensors.getTempC(roomThermometer));
  checkLights(now.hour(), MoonPhase(now.day()));
  //checkLights(now.hour(), 2);
 
  delay(29400); // Wait 30s after two DHT pauses and sensor reads
}


