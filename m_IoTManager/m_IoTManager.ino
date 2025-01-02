
//----------------------------------------defines-------------------------------//
//  #define ws2811_include// активировать для ws2811
#define will_use_serial
// #define timerAlarm
//  #define use_telegram
// #define USE_SPIFFS
#define USE_LITTLEFS
// #define ds18b20
#define USE_DNS_SERVER
// #define pubClient
// #define ir_code
// #define as
// #define wakeOnLan
// #define dht
//  #define ads1115
//  #define emon
//  #define ws433
//------------------------------------------------------------------------------//

// #include <Adafruit_GFX.h>
// #include <gfxfont.h>

// -----------------------DEFINING PINS----------------------------------
#define ONE_WIRE_BUS 0 // D3 pin
#define RECV_PIN 14    // IR recieve
#define SEND_PIN 15    // IR send

// #include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h> //iotmanager
#include <EEPROM.h>
#include <ESP8266WebServer.h>        //Local WebServer used to
#include <ESP8266HTTPUpdateServer.h> //OTA needs

ESP8266HTTPUpdateServer httpUpdater; // OTA
// ###############################
#if defined(ds18b20)
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
#endif
// ########spiffs

#if defined USE_SPIFFS
#include <FS.h>
const char *fsName = "SPIFFS";
FS *fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#endif
#if defined USE_LITTLEFS
#include <LittleFS.h>
const char *fsName = "LittleFS";
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
#endif
// ##end spiffs

// ###############################
WiFiClientSecure wclient;
#if defined(pubClient)
#include <PubSubClient.h>
PubSubClient client(wclient); // for cloud broker - by hostname
#endif

//////////////////////////////////compass
#if defined(as5600)
#include <AS5600.h>
AS5600 encoder;
#endif
////////////////////////////////////////////
// serve the configuration portal
#include <FS.h>

#define DBG_OUTPUT_PORT Serial
#include <MD5.h>

#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(timeLibraryUsing)
#include <TimeLib.h>
#endif
ESP8266WebServer server(80);
// ESP8266WebServer server = new ESP8266Webserver(80);

extern "C"
{
#include <user_interface.h>
}
#if defined(dht)
#include "DHTesp.h"
DHTesp dht;
#endif
////////////////////
/// emon//
#if defined(emon)
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
float PowerCorrection = 111.1;
/////////
#endif
#include <ESP8266SSDP.h>

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
bool try_MQTT_access = false;
bool IOT_Manager_loop = false;
int no_internet_timer = 0;
bool internet = false;
//////////////////CaptivePortalAdvanced
char softAP_ssid[32] = "dev_001";
char softAP_password[32] = "12345678";
char ssid[32] = "";
char password[32] = "";
uint8_t ipport = 80;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";
char deviceID[20] = "dev-"; // thing ID - unique device id in our project
#if defined(pubClient)
char mqttServerName[60] = "m20.cloudmqtt.com";
unsigned int mqttport = 16238;
char mqttuser[15] = "spspider";
char mqttpass[15] = "5506487";
uint8_t type_mqtt = 1;
#endif

/////////////IR
bool Page_IR_opened = false;
bool geo_enable = false;
#if defined(ws433)
bool loop_433 = true;
#endif
// bool ir_loop = false;
bool wifi_scan = true;
bool ws8211_loop = true;
bool save_stat = false;
bool IR_recieve = false;
bool loop_alarm_active = true;
bool check_internet = true;
uint8_t mqttspacing = 60;
///////////////////433
#if defined(ws433)
char w433rcv = 255;
uint8_t w433send = 255;
#endif
//////////////////////////////
// String jsonConfig = "{}";
////////////TimeAlarmString/////////
const uint8_t Numbers = 1;   // количество условий в каждой кнопке
const uint8_t Condition = 1; // количество кнопок
uint8_t save_stat_long = 0;  // only initialized once
///////////////////////////////////////
uint8_t pwm_delay_long = 10;
///////////////////////////////////////////
// char trying_attempt_mqtt = 0;
uint8_t router = 255;
// замок:
uint8_t countdown_lock = 0;
uint8_t onesec;
unsigned long millis_strart_one_sec;
unsigned long millis_offset = 0;
uint8_t onesec_255;

bool license = 0;
bool test_action = false;

uint8_t PWM_frequency = 1;
// telegram global
#ifdef use_telegram
String BOTtoken = "";
#endif

/////////////////////////////ads
// ads1115
#include <Wire.h>
#if defined(ads1115)
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads(0x48);
#endif

///////////////////////////////
void setup()
{
#if defined(USE_LITTLEFS)
  if (!LittleFS.begin())
  {
    LittleFS.format();
  };
#endif
  wclient.setInsecure(); // Disables certificate verification for testing purposes
#if defined(will_use_serial)
  Serial.begin(115200);
#endif
  delay(10);
  Serial.println();
  Serial.println();
  setup_FS();
  MD5Builder md5;
  md5.begin();
  md5.add(WiFi.macAddress() + "password");
  md5.calculate();
  if (readCommonFiletoJson("activation") == md5.toString())
  {
    license = 1;
  }

  if (loadConfig(fileSystem->open("/other_setup.txt", "r")))
  {
  }
  captive_setup();
#if defined(ws2811_include)
  setup_ws2811(); // include ws2811.in
#endif
#if defined(wakeOnLan)
  setup_WOL();
#endif
#if defined(as5600)
  setup_compass();
#endif
  // Настраиваем и запускаем SSDP интерфейс
  //   Serial.println("Start 3-SSDP");
#if defined(USE_SSDP)
  SSDP_init();
#endif

#if defined(ir_code)
  if (IR_recieve)
  {
    setup_IR();
  }
#endif
//////////////////////////////////////
#if defined(ws433)
  setup_w433();
#endif
  // setup_wg();
  /////////////////////////////////////
  // callback_from_stat();

#if defined(ds18b20)
  sensors.begin(); // Start up the library
#endif
}
void resetMillis()
{
  millis_offset = millis();
}
unsigned long getMillis()
{
  return millis() - millis_offset;
}
void loop()
{
  captive_loop();
#if defined(ir_code)
  if (IR_recieve)
  {
    loop_IR();
  }
#endif
#if defined(ws433)
  if (w433rcv != 255)
  {
    loop_w433();
  }
#endif
#if defined(ws2811_include)
  if (ws8211_loop == true)
  {
    loop_ws2811(); // include ws2811.in
  }
#endif
#if defined(timerAlarm)
  if (loop_alarm_active)
  {
    loop_alarm();
  }
#endif
  if ((unsigned long)(getMillis() - millis_strart_one_sec) > 1000L)
  {
    {
      onesec++;
#ifdef use_telegram
      loop_telegram();
#endif
#if defined(timerAlarm)
      check_for_changes();
#endif
#if defined(pubClient)
      //    subscr_loop_PLUS();
      pubClientOneSecEvent();
#endif
      onesec_255++;
#if defined(timerAlarm)
      check_if_there_next_times();
#endif
#if defined(ws2811_include)
      one_sec();
#endif
      millis_strart_one_sec = getMillis();
    }
    yield();
  }
  if (getMillis() > 18000000L)
  {
    ESP.restart();
  }
}