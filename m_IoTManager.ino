//4mb 1mb 1mb
//arduino 2.6.3
//----------------------------------------defines-------------------------------//
//#define ws2811_include// активировать для ws2811
#define will_use_serial
#define use_telegram
//#define pubClient
//#define ds18b20
//#define ads1115
//#define emon
#define ws433
//------------------------------------------------------------------------------//

//#include <Adafruit_GFX.h>
//#include <gfxfont.h>

//#include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> //iotmanager
#include <EEPROM.h>
//#include <WiFiClientSecure.h>



//###############################
#if defined(ds18b20)
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
#endif
//###############################



#if defined(pubClient)
#include <PubSubClient.h>
#endif
#include <ESP8266WebServer.h>     //Local WebServer used to 
//////////////////////////////////compass
//#include <AS5600.h>
//AS5600 encoder;
////////////////////////////////////////////
//serve the configuration portal
#include <FS.h>
//#include <ESP8266WiFiMulti.h>


#define DBG_OUTPUT_PORT Serial

//Encoder//
//bool EncoderA_prev = false;
//unsigned char EncoderIA = 12;
//unsigned char EncoderIB = 14;
//unsigned char engineA = 13;
//unsigned char engineB = 15;
//unsigned char speed_Enc = 255;

//void ICACHE_RAM_ATTR doEncoderA();
//void ICACHE_RAM_ATTR doEncoderB();


//////////compass/////////////
//#include <QMC5883L.h>
//#include <Wire.h>
//#include <rBase64.h>
#include <MD5.h>


//QMC5883L compass;

//ws8211//////////////////////////////
//void loadLimits();
//void setup_ws2811();
//bool LoadData_set_leds(char json[400]);
//bool LoadData(char json[200]);
//void one_sec();
//void loop_ws2811();

///////////////////////////////

#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
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
#if defined(emon)
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
float PowerCorrection = 111.1;
/////////
#endif
#include <ESP8266SSDP.h>

///////////////////////////////////Dimmer
//#include <RBDdimmer.h>//

//#define USE_SERIAL  SerialUSB //Serial for boards whith USB serial port

//#define outputPinDimmer  16//D0
//#define zerocrossDimmer  5//D1 // for boards with CHANGEBLE input pins
//dimmerLamp dimmer(outputPinDimmer, zerocrossDimmer); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
//uint8_t DimmerVal = 0;
/////////////////////////////////////////



/////Ultrasonic///////////////
//////////////////////////////
/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
bool try_MQTT_access = false;

bool IOT_Manager_loop = 0;
int no_internet_timer = 0;
bool internet = false;
//////////////////CaptivePortalAdvanced
char softAP_ssid[32] = "ESP_ap_dev_001";
char softAP_password[32] = "12345678";
char ssid[32] = "";
char password[32] = "";
short unsigned int ipport = 80;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";
char deviceID[20] = "dev01-kitchen";   // thing ID - unique device id in our project
#if defined(pubClient)
char prefix[20]  = "/IoTmanager";     // global prefix for all topics - must be some as mobile device

char mqttServerName[25] = "m20.cloudmqtt.com";
unsigned int mqttport = 16238;
//String mqttuser = "spspider";
char mqttuser[15] = "spspider";
//String mqttpass = "5506487";
char mqttpass[15] = "5506487";
unsigned char type_mqtt = 1;
#endif
//////////////Email///////////
char smtp_arr[] = "mail.smtp2go.com";
short unsigned int smtp_port = 2525;
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
#if defined(ws433)
bool loop_433 = true;
#endif
//bool ir_loop = false;
bool wifi_scan = true;
bool ws8211_loop = true;
bool save_stat = false;
bool IR_recieve = false;
bool loop_alarm_active = true;
bool check_internet = true;
short unsigned int mqttspacing = 60;
///////////////////433
#if defined(ws433)
char w433rcv = 255;
uint8_t w433send = 255;
#endif
//////////////////////////////
//String jsonConfig = "{}";
////////////TimeAlarmString/////////
const unsigned char Numbers = 1;//количество условий в каждой кнопке
const unsigned char Condition = 1;//количество кнопок
unsigned char save_stat_long = 0;                     // only initialized once
short int bySignalPWM[Condition][Numbers];
///////////////////////////////////////
uint8_t pwm_delay_long = 10;
///////////////////////////////////////////
//char trying_attempt_mqtt = 0;
uint8_t router = 255;
//замок:
unsigned char countdown_lock = 0;
unsigned int onesec;
unsigned long millis_strart_one_sec;
//uint8_t onesec_240;
//uint8_t check_my_alarm;
uint8_t onesec_255;

bool license = 0;
//bool get_new_pin_value_ = true;

bool test_action = false;
//bool internet_pin_inv=0;
//char internet_cycle = 255;

unsigned char PWM_frequency = 1;
//telegram global
#ifdef use_telegram
String BOTtoken = "";
String CHAT_ID = "";
#endif

/////////////////////////////ads
//ads1115
#include <Wire.h>
#if defined(ads1115)
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads(0x48);
#endif
///////////////////////////////
void setup() {
#if defined(will_use_serial)
  Serial.begin(115200);
#endif
  delay(10);
  Serial.println();
  Serial.println();
  //getEEPROM_char(0) == 13 ? license = 1 : license = 0;
  setup_FS();
  //test_setup();
  //analogWriteFreq(150);
  MD5Builder md5;
  md5.begin();
  md5.add(WiFi.macAddress() + "password");
  md5.calculate();
  if (readCommonFiletoJson("activation") == md5.toString()) {
    license = 1;
  }

  //char* testString="123";
  //short int testInt = atoi(testString);
  //Serial.print("!!!!!!!!!!!!!!!!!!!!!testInt:");
  //Serial.println(testInt);
  if (loadConfig(readCommonFiletoJson("other_setup"))) { }
  //server = ESP8266WebServer (ipport);


  captive_setup();
#if defined(ws2811_include)
  setup_ws2811();//include ws2811.in
#endif
  setup_WOL();

  //  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)
  setup_compass();
  //Настраиваем и запускаем SSDP интерфейс
  //  Serial.println("Start 3-SSDP");
  //SSDP_init();

  if (IR_recieve) {
    setup_IR();
  }
  //////////////////////////////////////
#if defined(ws433)
  setup_w433();
#endif
  //setup_wg();
  /////////////////////////////////////
  callback_from_stat();

#if defined(ds18b20)
  //setup_ds();
  sensors.begin();  // Start up the library
  //unsigned char deviceCount = sensors.getDeviceCount();
#endif
#ifdef use_telegram
  setup_telegram();
#endif
}
void loop() {
  captive_loop();
  test_loop();
  loop_websocket();
  if (IR_recieve) {
    loop_IR();
  }
#if defined(ws433)
  if (w433rcv != 255) {
    loop_w433();
  }
#endif
  if (ws8211_loop == true) {
#if defined(ws2811_include)
    loop_ws2811();//include ws2811.in
#endif
  }

  if (loop_alarm_active) {
    loop_alarm();
  }

  if (millis() > 1000L + millis_strart_one_sec) {
    onesec++;
    onesec_255++;
    check_if_there_next_times();
    //Serial.println(onesec_255);
#if defined(ws2811_include)
    one_sec();//include ws2811.in
#endif
    one_sec_lock();
    millis_strart_one_sec = millis();
  }
  loop_ota();
  //EncoderCalc();
  //ultrasonic_loop();
#ifdef use_telegram
  loop_telegram();
#endif
  //loop_wg();
}

