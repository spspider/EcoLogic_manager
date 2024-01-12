bool loadConfig(String  jsonConfig) {
  // Открываем файл для чтения
  /*
    File configFile = SPIFFS.open("/settings.txt", "r");
    if (!configFile) {
    // если файл не найден
    Serial.println("Failed to open settings.txt");
    //  Создаем файл запиав в него аные по умолчанию
    saveConfig();
    configFile.close();
    }
    // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
    size_t size = configFile.size();
    if (size > 1024) {
    Serial.println("Config file size is too large");
    configFile.close();
    }
  */



  // Serial.println(jsonConfig);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonConfig);
  if (!root.success()) {
    //Serial.println("Parsing fail");
    return false;
  }
  /////////////////////////////

  if (root.containsKey("softAP_ssid")) {
    /*
      String softAP_password_string = root["softAP_password"].as<String>();
      softAP_password_string.toCharArray(softAP_password, 32);
    */
    //const char *buff_softAP_ssid = root["softAP_ssid"]; softAP_ssid = (char[32])buff_softAP_ssid;
    //const char *buff_softAP_password = root["softAP_password"]; softAP_password = (char[32])buff_softAP_password;
    //Serial.print("softAP_ssid"); Serial.println(softAP_ssid);
    //Serial.print("softAP_password"); Serial.println(softAP_password);
  }

  if (root.containsKey("ssid")) {
    //const char* ssid_string = root["ssid"];
    strcpy(ssid, root["ssid"]);
    //const char* password_string = root["password"];
    strcpy(password, root["password"]);
  }

  if (root.containsKey("deviceID")) {
    // String softAP_ssid_string = root["deviceID"].as<String>();
    //const char* softAP_ssid_constChar = root["deviceID"];
    strcpy(softAP_ssid, root["deviceID"]);
    //Serial.print("strcpy(softAP_ssid, softAP_ssid_constChar)");
    // Serial.print(softAP_ssid);
    //deviceID = root["deviceID"].as<String>();
    strcpy(deviceID, root["deviceID"]);
  }

  //mqttServerName = root["mqttServerName"].as<String>();
#if defined(pubClient)

  IOT_Manager_loop = root["iot_enable"];
  if (IOT_Manager_loop) {
    client.disconnect();
  }

  strcpy(prefix, root["prefix"]);
  strcpy(mqttServerName, root["mqttServerName"]);
  root.containsKey("mqttport") ? mqttport = root["mqttport"] : mqttport = 1883;
  //mqttuser = root["mqttuser"].as<String>();
  /////////////////////
  //const char* mqttuser_ch_con_ch = root["mqttuser"];
  //strcpy (mqttuser_ch, mqttuser_ch_con_ch);
  /////////////////////
  //mqttpass = root["mqttpass"].as<String>();
  strcpy(mqttuser, root["mqttuser"]);
  strcpy(mqttpass, root["mqttpass"]);
  root.containsKey("mqttspacing") ? mqttspacing = root["mqttspacing"] : mqttspacing = 60;
#endif
  const char *buff_smtp_arr = root["smtp_arr"]; snprintf(smtp_arr, sizeof smtp_arr, "%s", buff_smtp_arr); // strncpy(smtp_arr, buff_smtp_arr, strlen(buff_smtp_arr));
  smtp_arr[sizeof(smtp_arr) - 1] = '\0';


  smtp_port = root["smtp_port"];
  to_email_addr = root["to_email_addr"].as<String>();
  from_email_addr = root["from_email_addr"].as<String>();
  emaillogin = (root["emaillogin"].as<String>());
  password_email =  (root["password_email"].as<String>());

  //root.containsKey("ipport") ? ipport = root["ipport"] : ipport = 80;

  root.containsKey("timezone") ? timezone = root["timezone"] : timezone = 2;
  root.containsKey("geo_enable") ? geo_enable = root["geo_enable"] : geo_enable = 0;
  root.containsKey("wifi_scan") ? wifi_scan = root["wifi_scan"] : wifi_scan = 1;
  // root.containsKey("ir_loop") ? ir_loop = root["ir_loop"] : ir_loop = 0;
#if defined(ws433)
  root.containsKey("loop_433") ? loop_433 = root["loop_433"] : loop_433 = 0;
#endif
  root.containsKey("ws8211_loop") ? ws8211_loop = root["ws8211_loop"] : ws8211_loop = 0;
  root.containsKey("save_stat") ? save_stat = root["save_stat"] : save_stat = 0;
  root.containsKey("PWM_frequency") ? PWM_frequency = root["PWM_frequency"] : PWM_frequency = 1;
  root.containsKey("IR_recieve") ? IR_recieve = root["IR_recieve"] : IR_recieve = 0;
  unsigned int freq = PWM_frequency * 100;
  analogWriteFreq(freq);
#if defined(pubClient)
  setup_IOTManager();
#endif

  String jsonConfig_string = readCommonFiletoJson("pin_setup");
  if (updatepinsetup(jsonConfig_string)) {
    Serial.println("widgets Loaded");
  }

  return true;
}

void Setup_pinmode(bool stat_loaded) {
  for (char i = 0; i < char(nWidgets); i++) {

    stat[i] = stat_loaded ? stat[i] : defaultVal[i];
    if (pin[i] != 255) {
      callback_scoket(i, stat[i]);
      //callback_scoket(char i, int payload_is);
      if (pinmode[i] == 1) {//in
        defaultVal[i] == 0 ? pinMode(pin[i], INPUT_PULLUP) : pinMode(pin[i], INPUT);
        stat[i] = (digitalRead(pin[i] ^ defaultVal[i]));
        //Serial.println("set input:" + String(pin[i], DEC) + "i:" + i);
      }
      if ((pinmode[i] == 2)) { //out
        pinMode(pin[i], OUTPUT);
        //stat[i] =  (defaultVal[i]);
        digitalWrite(pin[i],  stat[i] ); //^defaultVal[i]
        //stat[i] = (defaultVal[i]);
        //Serial.println("set output:" + String(pin[i], DEC) + "i:" + i + "stat:" + stat[i] + "def:" + String(defaultVal[i], DEC));
      }
      if ((pinmode[i] == 3) || (pinmode[i] == 7)) { //pwm,MQ7
        pinMode(pin[i], OUTPUT);

        analogWrite(pin[i], stat[i]); // PWM
        //              setPwmFrequency(pin[i], 1024);

        //analogWrite(pin[i], 286);//1.4V
        //Serial.println("set pwm:" + String(pin[i], DEC) + "i:" + String(i, DEC) + "stat:" + String(stat[i], DEC));
      }
      if (pinmode[i] == 5) {//low_pwm
        pinMode(pin[i], OUTPUT);
        low_pwm[i] = stat[i];
        digitalWrite(pin[i], 1);//далее - выключаем
        //Serial.println("set low_pwm:" + String(pin[i], DEC) + "i:" + String(i, DEC) + "stat:" + String(stat[i], DEC));
      }
      if (pinmode[i] == 4) {//adc// analogDivider analogSubtracter
        //stat[i] = (analogRead(17) * 1.0F / analogDivider) + analogSubtracter; //adc pin:A0//
        stat[i] = (analogRead(17) * 1.0F - analogSubtracter) / analogDivider; //adc pin:A0//
        //Serial.println("read adc:" + String(pin[i], DEC) + "i:" + String(i, DEC) + "stat:" + String( stat[i], DEC));
      }

      if ((pinmode[i] == 6) || (pinmode[i] == 8)) { //dht temp
        dht.setup(pin[i]); // data pin
        Serial.println("DHT:" + String(pin[i], DEC) );
      }
      if (pinmode[i] == 9) {//Dimmer
        //attachInterrupt(pin[i], zero_crosss_int, RISING);//When arduino Pin 2 is FALLING from HIGH to LOW, run light procedure!
        //InitInterrupt(do_on_delay, freqStep);

      }

      if (pinmode[i] == 10) { //powerMeter
        //pinMode(pin[i], OUTPUT);
#if defined(emon)
        emon1.current(17, PowerCorrection);//PowerCorrection=111.1
#endif
      }
      if (pinmode[i] == 11) { //compass

      }
      if (pinmode[i] == 13) { //EncoderA
        pinMode(pin[i], INPUT);
        //attachInterrupt(digitalPinToInterrupt(pin[i]), doEncoderA, RISING);
      }
      if (pinmode[i] == 14) { //EncoderB
        pinMode(pin[i], INPUT);
        //attachInterrupt(digitalPinToInterrupt(pin[i]), doEncoderB, CHANGE);
      }
      if (pinmode[i] == 15) { //ads
#if defined(ads1115)
        ads.begin();
#endif
      }

    }
  }

}




String readCommonFiletoJson(String file) {
  File configFile = SPIFFS.open("/" + file + ".txt", "r");
  if (!configFile) {
    // если файл не найден
    Serial.println("Failed to open " + file + ".txt");
    configFile.close();
    return "";
  }
  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
  }
  String  jsonConfig = configFile.readString();
  Serial.print("file:");
  Serial.print(file);
  Serial.print(" ");
  Serial.println(jsonConfig);
  configFile.close();
  return jsonConfig;
}
bool saveCommonFiletoJson(String pagename, String json, boolean write_add) {
  //w-перезапись, а  - добавление
  //char* pagename_ch;
  //pagename.toCharArray(pagename_ch,sizeof(pagename_ch));
  //  char *write_add_char = (write_add == true) ? 'w' : 'a';
  File configFile;
  if (write_add == 1) {
    //char* openString;
    //strcat(openString,"/");
    configFile = SPIFFS.open("/" + pagename + ".txt", "w");
  } else if (write_add == 0) {
    configFile = SPIFFS.open("/" + pagename + ".txt", "a");
  }
  if (!configFile) {
    Serial.println("Failed to open " + pagename + ".txt for writing");
    return false;
  }
  Serial.println("#############################SAVE: " + String(write_add, DEC));
  Serial.println(json);
  configFile.print(json);
  configFile.close();
  return true;
}




/////////////////////CONDITION////////////////////////////////////////////////////
bool SaveCondition(String json) {

  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(json);

  unsigned int NumberID = rootjs["ID"];

  Serial.println("NimberId:" + NumberID);
  String NameFile = "Condition" + String(NumberID);
  saveCommonFiletoJson(NameFile, json, 1);
  //load_Current_condition(NumberID);//сразу же загружаем в перменные это условие
  return true;
}
//////////////////////////////////////////////////////////////////////////////

bool updatepinsetup(String jsonrecieve) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(jsonrecieve);
  unsigned char numberChosed = rootjs["numberChosed"];
  if (numberChosed == 0) {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FAIL!! numberChosed = 0");
    return false;
  }
  if (numberChosed > nWidgetsArray) {
    numberChosed = nWidgetsArray;
  }
  nWidgets = numberChosed;
#if defined(ws433)
  rootjs.containsKey("w433") ? w433rcv = rootjs["w433"] : w433rcv = 255;
  rootjs.containsKey("w433send") ? w433send = rootjs["w433send"] : w433send = 255;
#endif
  for (uint8_t i = 0; i < numberChosed; i++) {
    uint8_t pinmodeJS = rootjs["pinmode"][i];                 pinmode[i] = pinmodeJS;
    unsigned char pinJS = rootjs["pin"][i];                            pin[i] = pinJS;
    char widgetJS = rootjs["widget"][i];                     widget[i] = widgetJS;
    unsigned int defaultValJS  = rootjs["defaultVal"][i];    defaultVal[i] = defaultValJS;
    char IrButtonIDJS  = rootjs["IrBtnId"][i];               IrButtonID[i] = IrButtonIDJS;
    id[i] = i;

    strncpy( descr[i], rootjs["descr"][i], sizeof(descr[i]) - 1);
    //sprintf(descr[i], "%19s", rootjs["descr"][i]);//instead of strcpy( descr[i], rootjs["descr"][i]);
    //snprintf( a, sizeof( a ), "%d", 132 );  // when a is array, not pointer

  }

  analogDivider = rootjs["aDiv"];
  analogSubtracter = rootjs["aSusbt"];
  pwm_delay_long = rootjs["PWM_interval"];
  //  PowerCorrection = rootjs["PCorr"];
#if defined(emon)
  rootjs.containsKey("PCorr") ? PowerCorrection = rootjs["PCorr"] : PowerCorrection = 111.1;
#endif
  rootjs.containsKey("router") ? router = rootjs["router"] : router = 255;
  //Serial.println("ROUTER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"+String(router,DEC));
#if defined(pubClient)
  initThingConfig();
#endif
  Setup_pinmode(load_stat());

  return true;
}
bool load_stat() {
  if (save_stat == false) {
    return false;
  }
  /////////////////////////
  DynamicJsonBuffer jsonBuffer_stat;
  String stat1 = readCommonFiletoJson("stat");
  JsonObject& root_stat = jsonBuffer_stat.parseObject(stat1);
  if (!root_stat.success()) {
    Serial.println("PARSE FAIL!!");
    //    for (char i = 0; i < nWidgets; i++) {
    //      stat[i] = 0;
    //    }
    return false;
  }
  for (char i = 0; i < nWidgets; i++) {
    short int stat_js = root_stat["stat"][i];
    if (stat_js) {
      stat[i] = stat_js;
      //callback_scoket(i, stat_js);
    }
    //Serial.println(stat_js);
  }
  ////////////////////////////
  return true;
}
void callback_from_stat() {
  for (char i = 0; i < nWidgets; i++) {
    callback_scoket(i, stat[i]);
    //Serial.println(stat_js);
  }
}
