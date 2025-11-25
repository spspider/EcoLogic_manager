bool loadConfig(File jsonConfig) {
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonConfig);

  if (error) {
    Serial.println("Failed to parse JSON! loadConfig");
    return false;
  }

  if (jsonDocument.containsKey("softAP_ssid")) {
    // Do something with softAP_ssid if needed
  }

  if (jsonDocument.containsKey("ssid")) {
    strcpy(ssid, jsonDocument["ssid"]);
    strcpy(password, jsonDocument["password"]);
  }

  // device_id теперь генерируется автоматически и не читается из конфигурации
  String defName = "ecologic_" + String(ESP.getChipId());
  strncpy(softAP_ssid, defName.c_str(), sizeof(softAP_ssid) - 1);
  softAP_ssid[sizeof(softAP_ssid) - 1] = '\0';

#if defined(USE_PUBSUBCLIENT)
  IOT_Manager_loop = jsonDocument["iot_enable"];
  if (IOT_Manager_loop) {
    client.disconnect();
  }

  strcpy(mqttServerName, jsonDocument["mqttServerName"]);
  jsonDocument.containsKey("mqttport") ? mqttport = jsonDocument["mqttport"] : mqttport = 1883;

  strcpy(mqttuser, jsonDocument["mqttuser"]);
  strcpy(mqttpass, jsonDocument["mqttpass"]);
  jsonDocument.containsKey("mqttspacing") ? mqttspacing = jsonDocument["mqttspacing"] : mqttspacing = 60;
#endif

  jsonDocument.containsKey("geo_enable") ? geo_enable = jsonDocument["geo_enable"] : geo_enable = 0;
  jsonDocument.containsKey("wifi_scan") ? wifi_scan = jsonDocument["wifi_scan"] : wifi_scan = 1;

#if defined(ws433)
  jsonDocument.containsKey("loop_433") ? loop_433 = jsonDocument["loop_433"] : loop_433 = 0;
#endif

  jsonDocument.containsKey("ws8211_loop") ? ws8211_loop = jsonDocument["ws8211_loop"] : ws8211_loop = 0;
  jsonDocument.containsKey("save_stat") ? save_stat = jsonDocument["save_stat"] : save_stat = 0;
  unsigned int freq = 1000;
  jsonDocument.containsKey("PWM_frequency") ? freq = jsonDocument["PWM_frequency"] : freq = 1000;
  analogWriteFreq(freq);  // frequency for PWM
  // analogWriteRange(1023);
  jsonDocument.containsKey("send_to_nodeRed") ? send_to_nodeRed = jsonDocument["send_to_nodeRed"] : send_to_nodeRed = 0;
  // telegram
#ifdef use_telegram
  if (jsonDocument.containsKey("BOTtoken")) {
    BOTtoken = jsonDocument["BOTtoken"].as<String>();
  }

#endif
  if (jsonDocument.containsKey("nodered_address")) {
    strncpy(nodered_address, jsonDocument["nodered_address"], sizeof(nodered_address) - 1);
    nodered_address[sizeof(nodered_address) - 1] = '\0';
  }
  if (jsonDocument.containsKey("server_url")) {
    strncpy(server_url, jsonDocument["server_url"], sizeof(server_url) - 1);
    server_url[sizeof(server_url) - 1] = '\0';
  }
  
  use_static_ip = jsonDocument.containsKey("use_static_ip") ? jsonDocument["use_static_ip"] : false;
  if (jsonDocument.containsKey("static_ip")) {
    strncpy(static_ip, jsonDocument["static_ip"], sizeof(static_ip) - 1);
    static_ip[sizeof(static_ip) - 1] = '\0';
  }
  if (jsonDocument.containsKey("gateway")) {
    strncpy(gateway, jsonDocument["gateway"], sizeof(gateway) - 1);
    gateway[sizeof(gateway) - 1] = '\0';
  }
  if (jsonDocument.containsKey("subnet")) {
    strncpy(subnet, jsonDocument["subnet"], sizeof(subnet) - 1);
    subnet[sizeof(subnet) - 1] = '\0';
  }
  if (jsonDocument.containsKey("dns1")) {
    strncpy(dns1, jsonDocument["dns1"], sizeof(dns1) - 1);
    dns1[sizeof(dns1) - 1] = '\0';
  }
  if (jsonDocument.containsKey("dns2")) {
    strncpy(dns2, jsonDocument["dns2"], sizeof(dns2) - 1);
    dns2[sizeof(dns2) - 1] = '\0';
  }
  //  String jsonConfig_string = readCommonFiletoJson("pin_setup");
  if (updatepinsetup(fileSystem->open("/pin_setup.txt", "r"))) {
    Serial.println("Widgets Loaded");
  }
#if defined(USE_PUBSUBCLIENT)
  setup_IOTManager();
#endif
  return true;
}

void Setup_pinmode(bool stat_loaded) {
  for (uint8_t i = 0; i < nWidgets; i++) {
    #if defined(USE_IRUTILS)
    if (pin[i] ==  RECV_PIN) return;      // IR recieve d1
    if (pin[i] == SEND_PIN) return;     // IR send d2
    #endif
    stat[i] = stat_loaded ? stat[i] : defaultVal[i];
    if (pin[i] != 255) {
      if (pinmode[i] == 1) {  // in
        defaultVal[i] == 0 ? pinMode(pin[i], INPUT_PULLUP) : pinMode(pin[i], INPUT);
        stat[i] = (digitalRead(pin[i] ^ defaultVal[i]));
      }
      if ((pinmode[i] == 2)) {  // out
        pinMode(pin[i], OUTPUT);
        digitalWrite(pin[i], stat[i]);
      }
      if (pinmode[i] == 3) {  // pwm
        pinMode(pin[i], OUTPUT);
        analogWrite(pin[i], stat[i]);
      }
      if (pinmode[i] == 4) { //ADC
        stat[i] = (analogRead(17) * 1.0F - analogSubtracter) / analogDivider;  // adc pin:A0//

      }
      if ((pinmode[i] == 5) || (pinmode[i] == 6)) {  // DHT
#if defined(USE_DHT)
        dht.setup(pin[i], DHTesp::DHT11);  // data pin
        delay(2000); // Даем время DHT11 для инициализации
        Serial.println("DHT11 initialized on pin: " + String(pin[i], DEC));
        // Проверяем статус датчика
        float testTemp = dht.getTemperature();
        float testHum = dht.getHumidity();
        Serial.println("DHT11 test - Temp: " + String(testTemp) + ", Humidity: " + String(testHum));
#endif
      }
      if (pinmode[i] == 8) {  // powerMeter
#if defined(USE_EMON)
        emon1.current(A0, PowerCorrection); // PowerCorrection=111.1
#endif
      }
      if (pinmode[i] == 14) {  // ads1115
#if defined(ads1115)
        ads.begin();
#endif
      }
    }
  }
}

String readCommonFiletoJson(String file) {
  File configFile = fileSystem->open("/" + file + ".txt", "r");
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
  String jsonConfig = configFile.readString();
  Serial.print("file:");
  Serial.print(file);
  Serial.print(" ");
  Serial.println(jsonConfig);
  configFile.close();
  return jsonConfig;
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, "a");
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}
bool saveCommonFiletoJson(String pagename, String json, boolean write_add) {
#if defined(USE_LITTLEFS)
  String filePath = "/" + pagename + ".txt";
  if (write_add == 1) {
    writeFile(LittleFS, filePath.c_str(), json.c_str());
    return true;
  } else {
    appendFile(LittleFS, filePath.c_str(), json.c_str());
    return true;
  }
  Serial.println("#############################SAVE: " + String(write_add, DEC));
#endif
  // File configFile;
  // if (write_add == 1)
  // {
  //   configFile = fileSystem->open("/" + pagename + ".txt", "w");
  // }
  // else if (write_add == 0)
  // {
  //   configFile = fileSystem->open("/" + pagename + ".txt", "a");
  // }
  // if (!configFile)
  // {
  //   Serial.println("Failed to open " + pagename + ".txt for writing");
  //   return false;
  // }
  // Serial.println("#############################SAVE: " + String(write_add, DEC));
  // Serial.println(json);
  // configFile.print(json);
  // configFile.close();
  // return true;
}

/////////////////////CONDITION////////////////////////////////////////////////////
bool SaveCondition(String json) {
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, json);

  if (error) {
    Serial.println("Failed to parse JSON!");
    return false;
  }

  unsigned int NumberID = jsonDocument["ID"];
  Serial.println("NimberId:" + String(NumberID));

  String NameFile = "Condition" + String(NumberID);
  saveCommonFiletoJson(NameFile, json, 1);
  // load_Current_condition(NumberID); // сразу же загружаем в переменные это условие
  return true;
}

//////////////////////////////////////////////////////////////////////////////

bool updatepinsetup(File jsonrecieve) {
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonrecieve);

  if (error) {
    Serial.println("Failed to parse JSON!");
    loadDefaultPinSetup();
    Setup_pinmode(false);  // false: use defaultVal
    return false;
  }

  uint8_t numberChosed = jsonDocument["numberChosed"];
  if (numberChosed == 0) {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FAIL!! numberChosed = 0");
    return false;
  }

  if (numberChosed > nWidgetsArray) {
    numberChosed = nWidgetsArray;
  }

  nWidgets = numberChosed;

#if defined(ws433)
  w433rcv = jsonDocument.containsKey("w433") ? jsonDocument["w433"] : 255;
  w433send = jsonDocument.containsKey("w433send") ? jsonDocument["w433send"] : 255;
#endif

  for (uint8_t i = 0; i < numberChosed; i++) {
    pinmode[i] = jsonDocument["pinmode"][i];
    pin[i] = jsonDocument["pin"][i];
    defaultVal[i] = jsonDocument["defaultVal"][i];
    IrButtonID[i] = jsonDocument["IrBtnId"][i];
    id[i] = i;

    strncpy(descr[i], jsonDocument["descr"][i], sizeof(descr[i]) - 1);
  }

  analogDivider = jsonDocument.containsKey("aDiv") ? jsonDocument["aDiv"] : 1.0F;
  analogSubtracter = jsonDocument.containsKey("aSusbt") ? jsonDocument["aSusbt"] : 0;
  pwm_delay_long = jsonDocument.containsKey("PWM_interval") ? jsonDocument["PWM_interval"] : 60;

#if defined(USE_EMON)
  PowerCorrection = jsonDocument.containsKey("PCorr") ? jsonDocument["PCorr"] : 111.1;
#endif

  router = jsonDocument.containsKey("router") ? jsonDocument["router"] : 255;

  Setup_pinmode(load_stat());

  return true;
}

bool load_stat() {
  if (save_stat == false) {
    return false;
  }

  DynamicJsonDocument jsonDocument_stat(2048);  // Adjust the capacity as needed
  File stat1 = fileSystem->open("/stat.txt", "r");

  DeserializationError error = deserializeJson(jsonDocument_stat, stat1);
  if (error) {
    Serial.println("PARSE FAIL!!");
    //     for (uint8_t i = 0; i < nWidgets; i++) {
    //       stat[i] = 0;
    //     }
    return false;
  }

  for (uint8_t i = 0; i < nWidgets; i++) {
    short int stat_js = jsonDocument_stat["stat"][i];
    if (stat_js) {
      stat[i] = stat_js;
      // write_new_widjet_value(i, stat_js);
    }
    // Serial.println(stat_js);
  }

  return true;
}

void callback_from_stat() {
  for (uint8_t i = 0; i < nWidgets; i++) {
    write_new_widjet_value(i, stat[i]);
    // Serial.println(stat_js);
  }
}

void loadDefaultPinSetup() {
  nWidgets = 10;
  // pinmode: [2, 2, 2, 2, 3, 3, 3, 6, 6, 6]
  pinmode[0] = 2;
  pinmode[1] = 2;
  pinmode[2] = 2;
  pinmode[3] = 2;
  pinmode[4] = 3;
  pinmode[5] = 3;
  pinmode[6] = 3;
  pinmode[7] = 6;
  pinmode[8] = 6;
  pinmode[9] = 6;
  // pin: [0, 16, 16, 16, 14, 12, 13, 0, 5, 17]
  pin[0] = 0;
  pin[1] = 16;
  pin[2] = 16;
  pin[3] = 16;
  pin[4] = 14;
  pin[5] = 12;
  pin[6] = 13;
  pin[7] = 0;
  pin[8] = 5;
  pin[9] = 17;
  // defaultVal: [1, 1, 1, 1, 0, 0, 0, 0, 0, 0]
  defaultVal[0] = 1;
  defaultVal[1] = 1;
  defaultVal[2] = 1;
  defaultVal[3] = 1;
  defaultVal[4] = 0;
  defaultVal[5] = 0;
  defaultVal[6] = 0;
  defaultVal[7] = 0;
  defaultVal[8] = 0;
  defaultVal[9] = 0;
  // IrButtonID: [255, 255, 255, 255, 255, 255, 255, 255, 255, 0]
  IrButtonID[0] = 255;
  IrButtonID[1] = 255;
  IrButtonID[2] = 255;
  IrButtonID[3] = 255;
  IrButtonID[4] = 255;
  IrButtonID[5] = 255;
  IrButtonID[6] = 255;
  IrButtonID[7] = 255;
  IrButtonID[8] = 255;
  IrButtonID[9] = 0;
  // descr: ["d3 switch", "tx switch", "d0 switch", "rx switch", "d5 PWM", "d6 PWM", "d7 PWM", "ds18b20 temp", "d1 in", "ADC"]
  strncpy(descr[0], "d3 switch", sizeof(descr[0]) - 1);
  strncpy(descr[1], "tx switch", sizeof(descr[1]) - 1);
  strncpy(descr[2], "d0 switch", sizeof(descr[2]) - 1);
  strncpy(descr[3], "rx switch", sizeof(descr[3]) - 1);
  strncpy(descr[4], "d5 PWM", sizeof(descr[4]) - 1);
  strncpy(descr[5], "d6 PWM", sizeof(descr[5]) - 1);
  strncpy(descr[6], "d7 PWM", sizeof(descr[6]) - 1);
  strncpy(descr[7], "ds18b20 temp", sizeof(descr[7]) - 1);
  strncpy(descr[8], "d1 in", sizeof(descr[8]) - 1);
  strncpy(descr[9], "ADC", sizeof(descr[9]) - 1);
  // analogDivider, analogSubtracter, pwm_delay_long, router
  analogDivider = 0.0185F;
  analogSubtracter = 0;
  pwm_delay_long = 60;
  router = 255;
#if defined(USE_EMON)
  PowerCorrection = 111.1;
#endif
}

void checkAndRestoreDefaults() {
  pinMode(RESET_PIN, INPUT_PULLUP);
  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("RESET_PIN LOW at boot: waiting 3 seconds for restore trigger...");
    delay(3000);  // freeze for 3 seconds
    if (digitalRead(RESET_PIN) == LOW) {
      Serial.println("RESET_PIN still LOW: Restoring default config files...");
      struct {
        const char *defName;
        const char *targetName;
      } files[] = {
        { "pin_setup-def.txt", "pin_setup.txt" },
        { "other_setup-def.txt", "other_setup.txt" },
        { "wifilist-def.txt", "wifilist.txt" }
      };
      for (auto &f : files) {
        File src = LittleFS.open(f.defName, "r");
        if (!src) {
          Serial.print("Default file not found: ");
          Serial.println(f.defName);
          continue;
        }
        File dst = LittleFS.open(f.targetName, "w");
        if (!dst) {
          Serial.print("Failed to open for writing: ");
          Serial.println(f.targetName);
          src.close();
          continue;
        }
        while (src.available()) dst.write(src.read());
        src.close();
        dst.close();
        Serial.print("Restored: ");
        Serial.print(f.defName);
        Serial.print(" -> ");
        Serial.println(f.targetName);
      }
      Serial.println("Rebooting...");
      delay(1000);
      ESP.restart();
    }
  }
}
