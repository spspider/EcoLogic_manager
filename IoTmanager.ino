/*
  IoT Manager mqtt device client https://play.google.com/store/apps/details?id=ru.esp8266.iotmanager
  Based on Basic MQTT example with Authentication
  PubSubClient library v 1.91.1 https://github.com/Imroy/pubsubclient
  - connects to an MQTT server, providing userdescr and password
  - publishes config to the topic "/IoTmanager/config/deviceID/"
  - subscribes to the topic "/IoTmanager/hello" ("hello" messages from mobile device)

  Tested with Arduino IDE 1.6.6 + ESP8266 Community Edition v 2.0.0-stable and PubSubClient library v 1.91.1 https://github.com/Imroy/pubsubclient
  ESP8266 Community Edition v 2.0.0-stable have some HTTPS issues. Push notification temporary disabled.

  sketch version : 1.5
  IoT Manager    : any version

  toggle, range, small-badge widgets demo
*/


//const char *ssid =  "Home";            // cannot be longer than 32 characters!
//const char *pass =  "sp5506487";       // WiFi password



WiFiClient wclient;

// config for cloud mqtt broker by DNS hostname ( for example, cloudmqtt.com use: m20.cloudmqtt.com - EU, m11.cloudmqtt.com - USA )
//String mqttServerName = "m20.cloudmqtt.com";            // for cloud broker - by hostname, from CloudMQTT account data
//unsigned int    mqttport = 16238;                                // default 1883, but CloudMQTT.com use other, for example: 13191, 23191 (SSL), 33191 (WebSockets) - use from CloudMQTT account data
//String mqttuser =  "spspider";                              // from CloudMQTT account data
//String mqttpass =  "5506487";
// from CloudMQTT account data
#if defined(pubClient)
PubSubClient client(wclient); // for cloud broker - by hostname
#endif

//String val;
//String ids = "";
unsigned long  newtimePWM, oldtimePWM;
unsigned int oldtime = 0;
//int newValue;
//char client_iot_busy = 0;
//const String stat1 = "{\"status\":\"1\"}";
//const String stat0 = "{\"status\":\"0\"}";

char nWidgets = 9;
const char nWidgetsArray = 9;
short int stat        [nWidgetsArray];
//String sTopic      [nWidgetsArray];

#if defined(pubClient)
char sTopic_ch      [nWidgetsArray][50];
  #endif
char widget      [nWidgetsArray];// inputWidjet[0] = 'unknown';1 = 'toggle';2 = 'simple-btn';4 = 'range';4 = 'small-badge';5 = 'chart';
char descr       [nWidgetsArray][10];
//String thing_config[nWidgetsArray];
char    id          [nWidgetsArray];
unsigned char    pin         [nWidgetsArray];
short int    defaultVal  [nWidgetsArray];
char    IrButtonID  [nWidgetsArray];
float analogDivider = 1.0F;
short int analogSubtracter = 0;
unsigned int low_pwm[nWidgetsArray];
bool low_pwm_off = false; //low_pwm
unsigned char   pinmode[nWidgetsArray];//inputPinmode[0] = "no pin";inputPinmode[1] = "in";inputPinmode[2] = "out";inputPinmode[3] = "pwm";inputPinmode[4] = "adc";inputPinmode[5] = "low_pwm";inputPinmode[6] = "IR";inputPinmode[7] = "датчик газа MQ7";

unsigned char subscribe_loop = 0;
uint8_t subscr_sec = 5;
uint8_t mqttspacing_oldtime;
/*
  char stat[nWidgetsArray][200];
  char sTopic[nWidgetsArray][200];
  char widget[nWidgetsArray][200];
  char descr[nWidgetsArray][200];
  char page[nWidgetsArray][200];
  char thing_config[nWidgetsArray][200];
  char pinmode[nWidgetsArray][200];
  //char test8[nWidgetsArray][200];
*/
// Push notifications
#if defined(pubClient)
char* setStatus ( char* s ) {
  char stat[40];
  if (type_mqtt == 0) {
    //stat = "{\"status\":\"" + s + "\"}";
    sprintf(stat, "{\"status\":\"%s\"}", s);
  }
  else if (type_mqtt == 1) {
    //stat = s;
    strncpy(stat, s, sizeof(stat));
  }
  return stat;
}


char* setStatus ( int s ) {
  char stat[40];
  if (type_mqtt == 0) {
    //stat = "{\"status\":\"" + String(s) + "\"}";
    sprintf(stat, "{\"status\":\"%d\"}", s);
  }
  if (type_mqtt == 1) {
    //stat = String(s);
    sprintf(stat, "%d", s);
  }
  return stat;
}
char* setStatus ( float s ) {
  char stat[40];
  if (type_mqtt == 0) {
    //if (s%)
    //stat = "{\"status\":\"" + String(s) + "\"}";
    sprintf(stat, "{\"status\":\"%.2f\"}", s);
  }
  if (type_mqtt == 1) {
    //stat = String(s);
    sprintf(stat, "%.2f", s);
  }
  return stat;
}

void initVar() {
  initThingConfig();
}

void initThingConfig() {
  for (char i = 0; i < nWidgets; i++) {
    //sTopic_ch[i]   = prefix + "/" + deviceID + "/" + descr[i];
    //const char* __deviceID = deviceID.c_str();
    //const char* __descr = descr[i].c_str();
    //const char* __prefix = prefix.c_str();
    ////////////////////
    memset(sTopic_ch[i], 0, sizeof sTopic_ch[i]);
    sprintf(sTopic_ch[i], "%s/%s/%s", prefix, deviceID, descr[i]);
    Serial.println(sTopic_ch[i]);

    /////////////////////
    if (type_mqtt == 1) {
      //sTopic[i] = deviceID + "/" + descr[i] + "/" + String(id[i], DEC);
      memset(sTopic_ch[i], 0, sizeof sTopic_ch[i]);//очищаем
      sprintf(sTopic_ch[i], "%s/%s/%d", deviceID, descr[i], id[i]);
      Serial.println(sTopic_ch[i]);
    }
    //sTopic[i]   = prefix + "/" + String(id[i], DEC);
  }
  //  unsigned int *value_back = getSPIFFS_JSON_VALUE(nWidgets);


}
// send confirmation
String make_thing_config(char i) {
  String this_thing_config;
  //for (char i = 0; i < nWidgets; i++) {


  String ifsimple_btn;

  String widget_decode = "unknown";
  ifsimple_btn = "";
  switch (widget[i]) {
      break;
    case 1:
      widget_decode = "toggle";
      break;
    case 2:
      widget_decode = "simple-btn";
      ifsimple_btn = ",\"class3\":\"button button-balanced\"";
      ifsimple_btn += ",\"title\":\"" + String(descr[i]) + "\"";
      //ifsimple_btn += ,\"label\":" + descr[i];
      break;
    case 3:
      widget_decode = "range";
      break;
    case 4:
      widget_decode = "small - badge";
      break;
    case 5:
      widget_decode = "chart";
      // ifsimple_btn = ", \"widgetConfig\":{\"type\": \"'line'\"}";
      break;
    case 6:
      widget_decode = "gauge";
      ifsimple_btn = ",\"class1\":\"text-center col-xs-6 no-padding-left no-padding-right\"";
      break;
    default:
      widget_decode = "unknown";

      break;
  }
  //    thing_config[i] = "{\"id\":\"" + String(id[i], DEC) + "\",\"page\":\"" + deviceID + "\",\"descr\":\"" + descr[i] + "\",\"widget\":\"" + widget_decode + "\",\"topic\":\"" + sTopic[i] + "\"" + ifsimple_btn + "}"; // GPIO switched On/Off by mobile widget toggle
  //  this_thing_config = "{\"id\":\"" + String(id[i], DEC) + "\",\"page\":\"" + deviceID + "\",\"descr\":\"" + descr[i] + "\",\"widget\":\"" + widget_decode + "\",\"topic\":\"" + sTopic[i] + "\"" + ifsimple_btn + "}"; // GPIO switched On/Off by mobile widget toggle
  this_thing_config = "{\"id\":\"" + String(id[i], DEC) + "\",\"page\":\"" + String(deviceID) + "\",\"descr\":\"" + String(descr[i]) + "\",\"widget\":\"" + widget_decode + "\",\"topic\":\"" + String(sTopic_ch[i]) + "\"" + ifsimple_btn + "}"; // GPIO switched On/Off by mobile widget toggle

  //}

  return this_thing_config;
}

void pubStatus(char t[], char* payload) {
  if (!client.connected())  return;

  if (type_mqtt == 0) {
    //t += "/status";
    strcat(t, "/status");
  }

  if (client.publish(t, payload)) {
    Serial.println("Publish new status for " + String(t) + ", value: " + String(payload));
  } else {
    Serial.println("Publish new status for " + String(t) + " FAIL!");
  }
}

void pubConfig() {
  bool success;

  /*
    char __prefix[prefix.length() + 1];
    prefix.toCharArray(__prefix, prefix.length() + 1);

    char __deviceID[deviceID.length() + 1];
    deviceID.toCharArray(__deviceID, prefix.length() + 1);
  */

  //const char* __prefix = prefix.c_str();
  //const char* __deviceID = deviceID.c_str();

  success = client.publish(prefix, deviceID);

  if (success) {
    if (type_mqtt == 0) {
      delay(500);
      //String topic_str = String(prefix) + "/" + deviceID + "/config";
      //const char * __topic_str = topic_str.c_str();
      char topic_str[50];
      sprintf(topic_str, "%s/%s/config", prefix, deviceID);
      for (uint8_t i = 0; i < char(nWidgets); i = i + 1) {
        String msg_str = make_thing_config(i);
        /*
              char __topic_str[topic_str.length() + 1];
              topic_str.toCharArray(__topic_str, topic_str.length() + 1);

              char __msg_str[msg_str.length() + 1];
              msg_str.toCharArray(__msg_str, msg_str.length() + 1);
        */

        const char * __msg_str = msg_str.c_str();

        //success = client.publish(__topic_str, __msg_str);
        if (client.publish(topic_str, __msg_str)) {
          Serial.println("Publish config: Success (topic:" + String(topic_str) + " msg:" + String(msg_str) + ")");
        } else {
          Serial.println("Publish config FAIL! (topic:" + String(topic_str) + " msg:" + String(msg_str) + ")");
        }
        delay(150);
      }
    }
  }

  if (success) {
    Serial.println("Publish config: Success");
  } else {
    Serial.println("Publish config: FAIL");
  }
  for (char i = 0; i < nWidgets; i = i + 1) {
    if ((pinmode[i] == 2) || (pinmode[i] == 1)) {//out, in
      pubStatus(sTopic_ch[i], setStatus(stat[i]^defaultVal[i]));
    } else {
      pubStatus(sTopic_ch[i], setStatus(stat[i]));
    }




    delay(150);
  }
}

/*
  String getValue(String data, char separator, int index)
  {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
  }
*/


void callback(char* topic_char, byte * Byte, unsigned char length) {

  char convertedChar[length];
  char oneByte;
  unsigned char i1;
  for (unsigned char i = 0; i < length; i++) {
    oneByte = (char)Byte[i];
    if ((oneByte >= '0' && oneByte <= 'z') || (oneByte == ' ')) {
      convertedChar[i1] = oneByte;
      i1++;
    }
    else {
      Serial.println("!!break at:" + String(i1, DEC) + " position");
      break;
    }
  }
  convertedChar[i1] = '\0';
  //Serial.println("convertedChar:" + String(convertedChar));

  //String topic = String(topic_char);
  //String payload = String(convertedChar);
  //char myarray[5] = {'-', '1', '2', '3', '\0'};
  int newValue;
  //sscanf(convertedChar, "%d", &newValue);

  //newValue =  atoi(convertedChar);
  newValue = strtol(convertedChar, NULL, 10);
  //newValue = String(convertedChar).toInt();
  //newValue = payload.toInt();
  //Serial.print("Get data from subscribed topic ");
  Serial.print(topic_char);
  Serial.print(" => ");
  Serial.println(convertedChar);
  //pubStatus(deviceID + "/PLUS/" + String(thatCondition, DEC) + "/" + String(i, DEC), setStatus(bySignalPWM[thatCondition][i]));

  char buff[50];
  strncpy (buff, topic_char, sizeof(buff) - 1);  char* deviceID_topic = delimeter(buff, "/", 0);
  strcpy (buff, topic_char);  char* option_topic = delimeter(buff, "/", 1);
  strcpy (buff, topic_char);  char* thatCondition = delimeter(buff, "/", 2);
  strcpy (buff, topic_char);  char* thatConditionNumber = delimeter(buff, "/", 3);
  Serial.print(deviceID_topic);
  Serial.print(" ");
  Serial.print(option_topic);
  Serial.print(" ");
  Serial.print(option_topic);
  Serial.print(" ");
  Serial.print(thatConditionNumber);
  Serial.println(" ");
  // Serial.println(option_topic + " " + thatCondition + " " + thatConditionNumber);
  //if (option_topic == "PLUS") {
  if (strcmp (option_topic, "PLUS") == 0) {
    // Serial.println("Set PLUS cond:" + thatCondition + " n:" + thatConditionNumber);
    char thatConditionInt;           //sscanf(thatCondition, "%d", thatConditionInt);
    char thatConditionNumberInt;    // sscanf(thatConditionNumber, "%d", thatConditionNumberInt);
    //thatConditionInt = atoi(thatCondition);
    //thatConditionNumberInt = atoi(thatConditionNumber);
    //thatConditionInt = String(thatCondition).toInt();
    thatConditionInt = strtol(thatCondition, NULL, 10);
    thatConditionNumberInt = strtol(thatConditionNumber, NULL, 10);
    //thatConditionNumberInt = String(thatConditionNumber).toInt();

//    bySignalPWM[thatConditionInt][thatConditionNumberInt] = newValue;
    saveConditiontoJson(thatConditionInt);
  }
  for (char i = 0; i < nWidgets; i++) {
    //Serial.println("i:" + String(i, DEC) + " topic:" + topic + " " + sTopic[i] + "/control");
    char compare[50];

    if (type_mqtt == 0) {
      //compare = sTopic[i] + "/control";
      sprintf(compare, "%s/control", sTopic_ch[i]);
    } else if (type_mqtt == 1) {//MajorDomo
      //compare = sTopic[i] + "/status";
      sprintf(compare, "%s/status", sTopic_ch[i]);
    }

    //topic_char
    if (strcmp (topic_char, compare) == 0) {
      // if (topic == compare) {
      stat[i] = (newValue);
      callback_scoket(i, newValue);
      /*
        if ((pinmode[i] == 2) || (pinmode[i] == 1)) {//out, in
        stat[i] = newValue ^ defaultVal[i];
        send_IR(i);
        digitalWrite(pin[i], stat[i]);
        }
        else if (pinmode[i] == 3) {//pwm
        analogWrite(pin[i], newValue);
        }
        else if (pinmode[i] == 5) {//low_pwm
        low_pwm[i] = newValue;
        }
        else if (pinmode[i] == 4) { //adc
        }
      */
      pubStatus(sTopic_ch[i], setStatus(stat[i]));
      break;
    }
  }
  //const char* __prefix = prefix.c_str();
  if (strcmp (topic_char, prefix) == 0) {
    //if (topic == prefix) {
    if ((char)Byte[0] == 'H') {//значит HELLO
      if (type_mqtt == 0) {
        pubConfig();
      }
    }
  }
}

void setup_IOTManager() {
  client.setServer(mqttServerName, mqttport);
  client.setCallback(callback);
  client.setBufferSizes(512, 512);  // Set the minimum buffer sizes
  else {
    initVar();
  }
}

void loop_IOTMAnager() {

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.print("Connecting to MQTT server: ");
      Serial.print(mqttServerName);
      Serial.print(" port ");
      Serial.println(mqttport);
      bool success = false;
      //if (internet) {
      //const char *__deviceID = deviceID.c_str();

      if (strlen(mqttuser) != 0) {
        //if (mqttuser.length() > 0) {


        //const char *__mqttuser = mqttuser.c_str();
        //const char *__mqttpass = mqttpass.c_str();

        Serial.println("MQTT device id:" + String(deviceID) + ", user:" + String(mqttuser) + ",password:" + String(mqttpass));

        //success = client.connect( MQTT::Connect( deviceID ).set_auth(mqttuser, mqttpass) );
        success = client.connect(deviceID , mqttuser, mqttpass );
      } else {
        success = client.connect(deviceID);
      }
      // }
      if (success) {
        server.send(200, "text/json", "{client:connected}");
        Serial.println("Connect to MQTT server: Success");

        pubConfig();

        subscribe_loop = 0;
        // const char *__prefix = prefix.c_str();

        if (!client.subscribe(prefix)) {
          Serial.println("FAIL! subscribe" + String(prefix));
        };                 // for receiving HELLO messages
        //client.subscribe(prefix + "/ids");        // for receiving IDS  messages
        //for (int i = 0; i < char(nWidgets); i++) {
        /*

          if (i < nWidgets) {
          String topic_subscr;
          if (type_mqtt == 0) {//Iotmanager
            topic_subscr = sTopic[i] + "/control";
          } else if (type_mqtt == 1) {//MajorDomo
            topic_subscr = sTopic[i] + "/status";
          }
          const char *__topic_subscr = topic_subscr.c_str();

          if (!client.subscribe(__topic_subscr)) {
            Serial.println("Client subscribe FAIL CYCLE!:" + String(__topic_subscr));
            subscribe_loop = i;
            break;
          }          else {

            Serial.println("Client subscribe SUCCSESS CYCLE!:" + String(__topic_subscr));
          };   // for receiving GPIO messages
          //delay(10000);
          }
        */
        //}
      } else {
        success = false;
        //long wait_for_sec = ((no_internet_timer + 30000) - millis()) / 1000;
        Serial.print("Connect to MQTT server: FAIL");
        try_MQTT_access = false;
        //////////////////////////
        /*
          trying_attempt_mqtt++;
          if (trying_attempt_mqtt > 5) { //может нет интернета
          if (internet_pin != 255) {
            internet_pin_inv = digitalRead(internet_pin); //подключено к Wifi сети, значит роутер включен
            digitalWrite(internet_pin, internet_pin_inv ^ 1);//отключаем роутер
            trying_attempt_mqtt = 0;
            internet_cycle = 1;
          }
          }
        */
        //////////////////////////
      }


    }

    if (client.connected()) {
      if (onesec > oldtime + subscr_sec) { // 5 sec
        //////////////subscribe///////////////////////////
        if (subscribe_loop < nWidgets) {
          //if (subscribe_loop > nWidgets)return;
          //String topic_subscr;
          char topic_subscr[50];
          if (type_mqtt == 0) {//Iotmanager
            //topic_subscr = sTopic_ch[subscribe_loop] + "/control";
            sprintf(topic_subscr, "%s/control", sTopic_ch[subscribe_loop]);
          }
          else if (type_mqtt == 1) {//MajorDomo
            //topic_subscr = sTopic_ch[subscribe_loop] + "/status";
            sprintf(topic_subscr, "%s/status", sTopic_ch[subscribe_loop]);
          }
          //const char *__topic_subscr = topic_subscr.c_str();
          if (!client.subscribe(topic_subscr)) {
            Serial.println("Client subscribe FAIL!:" + String(topic_subscr));
          }
          else {
            Serial.println("Client subscribe SUCCSESS!:" + String(topic_subscr));
            subscribe_loop++;
          };
        }
        /////////////////////////////////////////
        oldtime = onesec;
        mqttspacing_oldtime++;
      }
      if ((mqttspacing_oldtime >  (mqttspacing / subscr_sec)) && (!ESP_busy)) { // 20 sec

        float x = 0.0f;
        for (uint8_t i = 0; i < nWidgets; i++) {
          x = get_new_pin_value(i);
          stat[i] = (int)x;
          //pubStatus(sTopic_ch[i],  setStatus(String(x, 2)));
          pubStatus(sTopic_ch[i],  setStatus(x));
        }
        mqttspacing_oldtime = 0;
      }
      /*
        int key;
        key = digitalRead(pin[4]);
        if ( stat[4] != setStatus(key) ) {
        stat[4] = setStatus(key);
        pubStatus(sTopic[4], stat[4] );  // widget 4
        }
      */
      client.loop();
    }
  }
}
#endif

