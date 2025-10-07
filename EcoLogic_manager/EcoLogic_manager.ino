//----------------------------------------defines-------------------------------//
//  #define ws2811_include// активировать для ws2811
#define will_use_serial
// #define timerAlarm
//  #define use_telegram
// #define USE_SPIFFS
#define USE_LITTLEFS
// #define USE_DS18B20
// #define USE_DNS_SERVER
// #define USE_UDP
// #define USE_PUBSUBCLIENT  //mqtt possibility
#define USE_IRUTILS
//  #define USE_PLAY_AUDIO_WAV  // for player.ino
//  #define USE_PLAY_AUDIO_MP3  // for player.ino
//  #define USE_TINYMQTT
//  #define USE_PICOMQTT
//  #define USE_AS5600
//  #define wakeOnLan
 #define USE_DHT // library version: 1.19 (dht sensor library for ESPx)
//  #define ads1115 # CHANGE TO USE_ADS1115
//  #define USE_EMON  //electric monitor
//  #define ws433 # CHANGE TO USE_WS433

//#define ONE_WIRE_BUS 2  // D4 pin ds18b20
#define RECV_PIN 5      // IR recieve d1
#define SEND_PIN 4     // IR send d2
#define N_WIDGETS 12
#define RESET_PIN 5  // D1 pin reset button

// #include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>  //iotmanager
#include <EEPROM.h>
#include <ESP8266WebServer.h>         //Local WebServer used to
#include <ESP8266HTTPUpdateServer.h>  //OTA needs

ESP8266HTTPUpdateServer httpUpdater;  // OTA
// ###############################
#if defined(USE_DS18B20)
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
#endif

#if defined(USE_UDP)
#include <WiFiUdp.h>
WiFiUDP Udp;
const int UDP_PORT = 4210;  // UDP port
char incomingPacket[255];
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
#if defined(USE_PUBSUBCLIENT)
#include <PubSubClient.h>
PubSubClient client(wclient);
#endif

//////////////////////////////////compass
#if defined(USE_AS5600)
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

extern "C" {
#include <user_interface.h>
}
#if defined(USE_DHT)
#include "DHTesp.h"
DHTesp dht;
#endif
////////////////////
/// emon//
#if defined(USE_EMON)
#include "EmonLib.h"  // Include Emon Library
EnergyMonitor emon1;  // Create an instance
float PowerCorrection = 111.1;
/////////
#endif
#include <ESP8266SSDP.h>

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
bool try_MQTT_access = false;
bool IOT_Manager_loop = false;
char no_internet_timer = 0;
bool internet = false;
// Флаги для отключения блокирующих операций
bool enable_http_requests = false;
bool enable_email_sending = false;
bool enable_geo_location = false;
bool enable_mqtt_reconnect = true;
//////////////////CaptivePortalAdvanced
char softAP_ssid[32] = "dev_001";
char softAP_password[32] = "12345678";
char ssid[32] = "";
char password[32] = "";
uint8_t ipport = 80;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";
char deviceID[20] = "dev01";  // thing ID - unique device id in our project
char mqttServerName[60] = "177e3ee7cf004e6ebed04b25d4c51a26.s1.eu.hivemq.cloud";
unsigned int mqttport = 8883;
char mqttuser[15] = "dev01";
char mqttpass[15] = "5506487";
uint8_t type_mqtt = 1;
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
uint8_t mqttspacing = 60;  // seconds between mqtt publish
///////////////////433
#if defined(ws433)
char w433rcv = 255;
uint8_t w433send = 255;
#endif
//////////////////////////////
// String jsonConfig = "{}";
////////////TimeAlarmString/////////
const uint8_t Numbers = 1;    // количество условий в каждой кнопке
const uint8_t Condition = 1;  // количество кнопок
uint8_t save_stat_long = 0;   // only initialized once
///////////////////////////////////////
uint8_t pwm_delay_long = 10;
///////////////////////////////////////////
// char trying_attempt_mqtt = 0;
uint8_t router = 255;
// замок:

unsigned long millis_strart_one_sec;
unsigned long millis_offset = 0;
uint8_t onesec_255, onesec, lastConnectTry = 0;

bool license = 0;
bool test_action = false;

uint8_t PWM_frequency = 1;
// telegram global
#ifdef use_telegram
String BOTtoken = "7256850489:AAHinhWkzRbfSdNcBcjlcH1-cvDFLMEbR38";
#endif

bool send_to_nodeRed = false;  // отправлять ли данные в nodeRed

/////////////////////////////ads
// ads1115
#include <Wire.h>
#if defined(ads1115)
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads(0x48);
#endif

///////////////////////////////
char nodered_address[32] = { 0 };

void setup() {
pinMode(0, OUTPUT); //hardcode pin D3 (GPIO 0) as output
digitalWrite(0, HIGH);
delay(100); // Время для стабилизации
#if defined(will_use_serial)
  Serial.begin(115200);
#endif
#if defined(USE_LITTLEFS)
  Serial.println("LittleFS init");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS!");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("LittleFS mount failed after format!");
      return;
    }
  }
  fileSystem = &LittleFS;
  checkAndRestoreDefaults();
#endif
#if defined(USE_SPIFFS)
  Serial.println("SPIFFS init");
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS!");
    SPIFFS.format();
    if (!SPIFFS.begin()) {
      Serial.println("SPIFFS mount failed after format!");
      return;
    }
  }
  fileSystem = &SPIFFS;
#endif
  wclient.setInsecure();  // Disables certificate verification for testing purposes

  delay(10);
  Serial.println();
  Serial.println();
  setup_FS();
#if defined(USE_UDP)
  Udp.begin(UDP_PORT);
  Serial.println("UDP listening on port " + String(UDP_PORT));
#endif
  MD5Builder md5;
  md5.begin();
  md5.add(WiFi.macAddress() + "password");
  md5.calculate();
  if (readCommonFiletoJson("activation") == md5.toString()) {
    license = 1;
  }

  if (loadConfig(fileSystem->open("/other_setup.txt", "r"))) {
  }
  captive_setup();
#if defined(USE_PICOMQTT)
  load_picoMqtt_config();
#endif
#if defined(ws2811_include)
  setup_ws2811();  // include ws2811.in
#endif
#if defined(wakeOnLan)
  setup_WOL();
#endif
#if defined(USE_AS5600)
  setup_compass();
#endif
  // Настраиваем и запускаем SSDP интерфейс
  //   Serial.println("Start 3-SSDP");
#if defined(USE_SSDP)
  SSDP_init();
#endif

#if defined(USE_IRUTILS)
  if (IR_recieve) {
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

#if defined(USE_DS18B20)
  sensors.begin();  // Start up the library
#endif

#if defined(USE_DHT)
  // Дополнительная инициализация DHT11 для GPIO2
  Serial.println("Initializing DHT11 sensor...");
  // Если у вас DHT11 подключен к GPIO2, раскомментируйте следующую строку:
  dht.setup(2, DHTesp::DHT11);
  delay(2000); // Даем время датчику для стабилизации
#endif

// Hardcoding pins
pinMode(0, OUTPUT); //hardcode pin D3 (GPIO 0) as output
digitalWrite(0, HIGH);
delay(100); // Время для стабилизации
}
void resetMillis() {
  millis_offset = millis();
}
unsigned long getMillis() {
  return millis() - millis_offset;
}
void loop() {

  captive_loop();
#if defined(USE_IRUTILS)
  if (IR_recieve) {
    loop_IR();
  }
#endif
#if defined(ws433)
  if (w433rcv != 255) {
    loop_w433();
  }
#endif
#if defined(ws2811_include)
  if (ws8211_loop == true) {
    loop_ws2811();  // include ws2811.in
  }
#endif
#if defined(timerAlarm)
  if (loop_alarm_active) {
    loop_alarm();
  }
#endif
  if ((unsigned long)(getMillis() - millis_strart_one_sec) > 1000L) {
    {
      onesec++;
#ifdef use_telegram
      loop_telegram();
#endif
#if defined(timerAlarm)
      check_for_changes();
#endif
#if defined(USE_PUBSUBCLIENT)
      client.loop();
      pubClientOneSecEvent();
#endif
#if defined(USE_PICOMQTT)
      loop_picoMqtt();
#endif
      onesec_255++;
#if defined(timerAlarm)
      check_if_there_next_times();
#endif
#if defined(ws2811_include)
      one_sec();
#endif
      millis_strart_one_sec = getMillis();
      check_new_status_and_send_nodeRed();
    }
    // int interim = onesec - lastConnectTry;
    // Serial.print(interim, DEC);
    yield();
  }
  
  // Watchdog для перезагрузки при зависании
  static unsigned long lastLoopTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime > 30000) { // Если loop не выполнялся 30 сек
    Serial.println("Loop timeout - restarting");
    ESP.restart();
  }
  lastLoopTime = currentTime;
  
  yield(); // Позволяем ESP8266 обрабатывать WiFi и другие задачи

#if defined(USE_UDP)
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
      incomingPacket[len] = '\0';

    // Пример: "6:589"
    uint8_t topic = atoi(strtok(incomingPacket, ":"));
    int value = atoi(strtok(NULL, ":"));

    callback_socket(topic, value);
  }
#endif
#if defined(USE_PLAY_AUDIO_WAV) || defined(USE_PLAY_AUDIO_MP3)
  loop_player();
#endif
}