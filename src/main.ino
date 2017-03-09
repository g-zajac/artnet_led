//******************************************************************************
#define FIRMWARE_VERSION "1.1.4"  //MAJOR.MINOR.PATCH more info on: http://semver.org
#define SERIAL_SPEED 115200
//#define PRODUCTION true         //uncoment to turn the serial debuging
//******************************************************************************

// Libraries
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <Adafruit_NeoPixel.h>    // neopixel lib from: https://github.com/adafruit/Adafruit_NeoPixel.git

// neopixel setup
#define NUMPIXELS 1
#define NEOPIXEL_PIN 12 //D6 on nodeMCU
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

extern "C" {
#include "user_interface.h"  //NOTE needed for esp info
}

// ------------------------------ Network --------------------------------------
//create credentials.h file in src folder with ssid and pass formated like below:
// const char* wifi_ssid = "yournetworkssid";
// const char* wif_password = "password";
#include "credentials.h"  //ignored by git to keep your network details private

#include <ArtnetWifi.h>   //clonedfrom https://github.com/rstephan/ArtnetWifi.git
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

void setup()
{
  // Initialize the Serial (debuging use only, not need in production)
  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.begin(SERIAL_SPEED);
    // compiling and esp info
    Serial.println("\r\n---------------------------------------"); //NOTE \r\n - new line, return
    Serial.println("Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__);
    Serial.print("Version: "); Serial.print(FIRMWARE_VERSION); Serial.println("   by Grzegorz & Anthony");
    Serial.println("---------------------------------------");
    Serial.println( F("\r\n--- ESP Info --- ") );
    Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size()); //IDEA add code size and free ram info
    Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
    Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
    Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
    Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
    Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
    Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
    Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
    Serial.println();
  #endif

  // Connecting to a WiFi network with static IP
  // ---------------------------------------------------------------------------
  // IP settings stored in creditentials file in format i.e:
  // #include "IPAddress.h" //add IPaddress class
  // IPAddress ip(192,168,1,101); //ip address of the unit
  // IPAddress gateway(192,168,1,1);
  // IPAddress subnet(255,255,255,0);

  WiFi.config(ip, gateway, subnet);

  WiFi.mode(WIFI_STA); // http://esp8266.github.io/Arduino/versions/2.1.0-rc1/doc/libraries.html
  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.print("Connecting to ");
    Serial.print(wifi_ssid);
  #endif

  WiFi.begin(wifi_ssid, wif_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifndef PRODUCTION // Not in PRODUCTION
      Serial.print(".");
    #endif
  }

  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.println("");
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  #endif

  //initialize neopixel
  pixels.begin();
  neopixelTest();

  artnet.begin();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop(){
  // we call the read function inside the loop
  artnet.read();
}

void neopixelTest(){

  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.println("\n\rNeo pixel test:");
    Serial.print("RED ");
  #endif
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(2000);

  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.print("GREEN ");
  #endif
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(2000);

  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.print("BLUE ");
  #endif
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(2000);

  #ifndef PRODUCTION // Not in PRODUCTION
    Serial.println("OFF");
  #endif
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  boolean tail = false;

  Serial.print("DMX: Univ: ");
  Serial.print(universe, DEC);
  Serial.print(", Seq: ");
  Serial.print(sequence, DEC);
  Serial.print(", Data (");
  Serial.print(length, DEC);
  Serial.print("): ");

  if (length > 16) {
    length = 16;
    tail = true;
  }
  // send out the buffer
  for (int i = 0; i < length; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  if (tail) {
    Serial.print("...");
  }
  Serial.println();
}
