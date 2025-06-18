bool loadConfig(const String &jsonString)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonString);

  if (error)
  {
    Serial.println("Failed to parse JSON! loadConfig");
    return false;
  }

  if (jsonDocument.containsKey("softAP_ssid"))
  {
    // Do something with softAP_ssid if needed
  }

  if (jsonDocument.containsKey("ssid"))
  {
    strcpy(ssid, jsonDocument["ssid"]);
    strcpy(password, jsonDocument["password"]);
  }

  if (jsonDocument.containsKey("deviceID"))
  {
    strcpy(softAP_ssid, jsonDocument["deviceID"]);
    strcpy(deviceID, jsonDocument["deviceID"]);
  }

#if defined(pubClient)
  IOT_Manager_loop = jsonDocument["iot_enable"];
  if (IOT_Manager_loop)
  {
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
  analogWriteFreq(freq); // frequency for PWM
  // analogWriteRange(1023);
  jsonDocument.containsKey("IR_recieve") ? IR_recieve = jsonDocument["IR_recieve"] : IR_recieve = 0;

  // telegram
#ifdef use_telegram
  if (jsonDocument.containsKey("BOTtoken"))
  {
    BOTtoken = jsonDocument["BOTtoken"].as<String>();
  }

#endif
#ifdef USE_IRUTILS
  if (jsonDocument.containsKey("nodered_address"))
  {
    strncpy(nodered_address, jsonDocument["nodered_address"], sizeof(nodered_address) - 1);
    nodered_address[sizeof(nodered_address) - 1] = '\0';
  }
#endif
  //  String jsonConfig_string = readCommonFiletoJson("pin_setup");
  if (updatepinsetup(fileSystem->open("/pin_setup.txt", "r")))
  {
    Serial.println("Widgets Loaded");
  }
#if defined(pubClient)
  setup_IOTManager();
#endif
  return true;
}

void Setup_pinmode(bool stat_loaded)
{
  for (uint8_t i = 0; i < nWidgets; i++)
  {

    stat[i] = stat_loaded ? stat[i] : defaultVal[i];
    if (pin[i] != 255)
    {
      if (pinmode[i] == 1)
      { // in
        defaultVal[i] == 0 ? pinMode(pin[i], INPUT_PULLUP) : pinMode(pin[i], INPUT);
        stat[i] = (digitalRead(pin[i] ^ defaultVal[i]));
      }
      if ((pinmode[i] == 2))
      { // out
        pinMode(pin[i], OUTPUT);
        digitalWrite(pin[i], stat[i]);
      }
      if ((pinmode[i] == 3) || (pinmode[i] == 7))
      { // pwm,MQ7
        pinMode(pin[i], OUTPUT);
        analogWrite(pin[i], stat[i]); // PWM
      }
      // if (pinmode[i] == 5)
      // { // low_pwm
      //   pinMode(pin[i], OUTPUT);
      //   low_pwm[i] = stat[i];
      //   digitalWrite(pin[i], 1); // далее - выключаем
      //   Serial.println("set low_pwm:" + String(pin[i], DEC) + "i:" + String(i, DEC) + "stat:" + String(stat[i], DEC));
      // }
      if (pinmode[i] == 4)
      {
        stat[i] = (analogRead(17) * 1.0F - analogSubtracter) / analogDivider; // adc pin:A0//
      }
      if ((pinmode[i] == 6) || (pinmode[i] == 8))
      { // dht temp
#if defined(USE_DHT)
        dht.setup(pin[i], DHTesp::DHT11); // data pin
        Serial.println("DHT:" + String(pin[i], DEC));
#endif
      }
      if (pinmode[i] == 10)
      { // powerMeter
        // pinMode(pin[i], OUTPUT);
#if defined(USE_EMON)
        emon1.current(17, PowerCorrection); // PowerCorrection=111.1
#endif
      }
      if (pinmode[i] == 15)
      { // ads
#if defined(ads1115)
        ads.begin();
#endif
      }
    }
  }
}

String readCommonFiletoJson(String file)
{
  File configFile = fileSystem->open("/" + file + ".txt", "r");
  if (!configFile)
  {
    // если файл не найден
    Serial.println("Failed to open " + file + ".txt");
    configFile.close();
    return "";
  }
  // Проверяем размер файла, будем использовать файл размером меньше 1024 байта
  size_t size = configFile.size();
  if (size > 1024)
  {
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

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
  file.close();
}
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, "a");
  if (!file)
  {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- message appended");
  }
  else
  {
    Serial.println("- append failed");
  }
  file.close();
}
bool saveCommonFiletoJson(String pagename, String json, boolean write_add)
{
#if defined(USE_LITTLEFS)
  String filePath = "/" + pagename + ".txt";
  if (write_add == 1)
  {
    writeFile(LittleFS, filePath.c_str(), json.c_str());
    return true;
  }
  else
  {
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
bool SaveCondition(String json)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, json);

  if (error)
  {
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

bool updatepinsetup(File jsonrecieve)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonrecieve);

  if (error)
  {
    Serial.println("Failed to parse JSON!");
    return false;
  }

  uint8_t numberChosed = jsonDocument["numberChosed"];
  if (numberChosed == 0)
  {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FAIL!! numberChosed = 0");
    return false;
  }

  if (numberChosed > nWidgetsArray)
  {
    numberChosed = nWidgetsArray;
  }

  nWidgets = numberChosed;

#if defined(ws433)
  w433rcv = jsonDocument.containsKey("w433") ? jsonDocument["w433"] : 255;
  w433send = jsonDocument.containsKey("w433send") ? jsonDocument["w433send"] : 255;
#endif

  for (uint8_t i = 0; i < numberChosed; i++)
  {
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

bool load_stat()
{
  if (save_stat == false)
  {
    return false;
  }

  DynamicJsonDocument jsonDocument_stat(2048); // Adjust the capacity as needed
  File stat1 = fileSystem->open("/stat.txt", "r");

  DeserializationError error = deserializeJson(jsonDocument_stat, stat1);
  if (error)
  {
    Serial.println("PARSE FAIL!!");
    //     for (uint8_t i = 0; i < nWidgets; i++) {
    //       stat[i] = 0;
    //     }
    return false;
  }

  for (uint8_t i = 0; i < nWidgets; i++)
  {
    short int stat_js = jsonDocument_stat["stat"][i];
    if (stat_js)
    {
      stat[i] = stat_js;
      // callback_socket(i, stat_js);
    }
    // Serial.println(stat_js);
  }

  return true;
}

void callback_from_stat()
{
  for (uint8_t i = 0; i < nWidgets; i++)
  {
    callback_socket(i, stat[i]);
    // Serial.println(stat_js);
  }
}
