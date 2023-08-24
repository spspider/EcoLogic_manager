//#include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> //iotmanager
#include <EEPROM.h>
//#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <FS.h>
//#include <ESP8266WiFiMulti.h>
#define DBG_OUTPUT_PORT Serial

#define WS8211_PIN 15;
//////////compass/////////////
#include <QMC5883L.h>
#include <Wire.h>

QMC5883L compass;

//////////////////////////////


#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>
#include <TimeLib.h>
ESP8266WebServer server(80);
//ESP8266WebServer server = new ESP8266Webserver(80);
/////////////////dht
extern "C" {
#include <user_interface.h>
}
#include "DHTesp.h"
DHTesp dht;
////////////////////
///emon//
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
float PowerCorrection = 111.1;
/////////
#include <ESP8266SSDP.h>
/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
bool try_MQTT_access = false;

bool IOT_Manager_loop = 0;
unsigned int no_internet_timer = 0;
bool internet = false;
//////////////////CaptivePortalAdvanced
char softAP_ssid[32] = "ESP_ap_dev_001";
char softAP_password[32] = "12345678";
char ssid[32] = "";
char password[32] = "";
int ipport = 80;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";
///////////////////////////IOT///////////
String prefix   = "/IoTmanager";     // global prefix for all topics - must be some as mobile device
String deviceID = "dev01-kitchen";   // thing ID - unique device id in our project
/////////////////////////////////////////
String mqttServerName = "m20.cloudmqtt.com";
unsigned int mqttport = 16238;
//String mqttuser = "spspider";
char mqttuser[15] = "spspider";
//String mqttpass = "5506487";
char mqttpass[15] = "5506487";
unsigned char type_mqtt = 1;
//////////////Email///////////
char smtp_arr[] = "mail.smtp2go.com";
unsigned int smtp_port = 2525;
String to_email_addr = "spspider@mail.ru"; // destination email address
String from_email_addr = "spspider95@smtp2go.com"; //source email address
//Use this site to encode: http://webnet77.com/cgi-bin/helpers/base-64.pl/
String emaillogin = "c3BzcGlkZXI5NUBnbWFpbC5jb20="; //username
String password_email = "NTUwNjQ4Nw=="; //password_email
char timezone = 2;               // часовой пояс GTM
//String deviceID = deviceID; // Имя SSDP
/////////////IR
bool Page_IR_opened = false;
bool geo_enable = false;
bool loop_433 = false;
bool ir_loop = false;
bool wifi_scan = true;
bool ws8211_loop = true;
///////////////////////////
bool loop_alarm_active = true;
bool check_internet = false;
unsigned int mqttspacing = 60;
///////////////////433
char w433rcv = 255;
uint8_t w433send = 255;
//////////////////////////////
//String jsonConfig = "{}";
////////////TimeAlarmString/////////
const unsigned char Numbers = 3;//количество условий в каждой кнопке
const unsigned char Condition = 3;//количество кнопок
int bySignalPWM[Condition][Numbers];
///////////////////////////////////////
uint8_t pwm_delay_long = 10;
///////////////////////////////////////////
//char trying_attempt_mqtt = 0;
uint8_t router = 255;
//замок:
unsigned char countdown_lock = 0;
unsigned int onesec;
unsigned long millis_strart_one_sec;

//bool internet_pin_inv=0;
//char internet_cycle = 255;
void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();


  setup_FS();
  //test_setup();

  if (loadConfig(readCommonFiletoJson("other_setup"))) { }
  //server = ESP8266WebServer (ipport);


  captive_setup();
  setup_ws2811();

  //setup_compass();
  //Настраиваем и запускаем SSDP интерфейс
  //  Serial.println("Start 3-SSDP");
  //SSDP_init();
  // setup_IR(true);
  //////////////////////////////////////
  // setup_w433();
  //setup_wg();
  /////////////////////////////////////


}
void loop() {
  captive_loop();
  //test_loop();
  loop_websocket();
  if (ir_loop) {
    //loop_IR();
  }
  if (w433rcv != 255) {
    //loop_w433();
  }
  if (ws8211_loop == true) {
    loop_ws2811();
  }

  if (loop_alarm_active) {
    loop_alarm();
  }

  if (millis() > 1000L + millis_strart_one_sec) {
    onesec++;
    one_sec();
    one_sec_lock();
    millis_strart_one_sec = millis();
  }
  loop_ota();
  //loop_wg();
}

