#define WEBSERVER "YOURWEVSERVER"
#define URISTEM "/PATH/TO/PHPscript.php"



/*Begin Definitions, starting with OneWire Addresses
#define ADDR1 "28:A0:CA:6F:03:00:00:33"
#define ADDR2 "28:21:80:A3:03:00:00:91"
#define ADDR3 "28:E7:73:41:03:00:00:1A"

*/
#define ONE_WIRE_BUS 2       // Where are the DS18B20's plugged in 

#define DAYLIGHTPIN 5        // Day Light Transistor control of 12v lights
#define NIGHTLIGHTPIN 6      // Night Light control
#define DAYLIGHTLEVEL 96     // Bright lights
#define NIGHTLIGHTLEVEL 32   // Night light

// Configure Ethernet
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Made Up MAC
static uint8_t gwip[4] = {192,168,1,1};                // GW IP
IPAddress ip(192,168,1,15);                            // Arduino Wiz IP
byte mserver[] = { 123,13,123,123 };                  // Remote Server IP

