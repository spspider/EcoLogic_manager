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
  if (root.containsKey("iot_enable")) {
    IOT_Manager_loop = root["iot_enable"];
    if (IOT_Manager_loop) {
      client.disconnect();
    }
  }
  if (root.containsKey("ssid")) {
    //const char* ssid_string = root["ssid"];
    strcpy(ssid, root["ssid"]);
    //const char* password_string = root["password"];
    strcpy(password, root["password"]);
  }
  if (root.containsKey("prefix")) {
    prefix = root["prefix"].as<String>();
  }
  if (root.containsKey("deviceID")) {
    // String softAP_ssid_string = root["deviceID"].as<String>();
    //const char* softAP_ssid_constChar = root["deviceID"];
    strcpy(softAP_ssid, root["deviceID"]);
    //Serial.print("strcpy(softAP_ssid, softAP_ssid_constChar)");
    // Serial.print(softAP_ssid);
    deviceID = root["deviceID"].as<String>();
  }

  mqttServerName = root["mqttServerName"].as<String>();
  root.containsKey("mqttport") ? mqttport = root["mqttport"] : mqttport = 1883;
  //mqttuser = root["mqttuser"].as<String>();
  /////////////////////
  //const char* mqttuser_ch_con_ch = root["mqttuser"];
  //strcpy (mqttuser_ch, mqttuser_ch_con_ch);
  /////////////////////
  //mqttpass = root["mqttpass"].as<String>();
  strcpy(mqttuser, root["mqttuser"]);
  strcpy(mqttpass, root["mqttpass"]);
  
  const char *buff_smtp_arr = root["smtp_arr"]; snprintf(smtp_arr, sizeof smtp_arr, "%s", buff_smtp_arr); // strncpy(smtp_arr, buff_smtp_arr, strlen(buff_smtp_arr));
  smtp_arr[sizeof(smtp_arr) - 1] = '\0';


  smtp_port = root["smtp_port"];
  to_email_addr = root["to_email_addr"].as<String>();
  from_email_addr = root["from_email_addr"].as<String>();
  emaillogin = (root["emaillogin"].as<String>());
  password_email =  (root["password_email"].as<String>());

  //root.containsKey("ipport") ? ipport = root["ipport"] : ipport = 80;
  root.containsKey("mqttspacing") ? mqttspacing = root["mqttspacing"] : mqttspacing = 60;
  root.containsKey("timezone") ? timezone = root["timezone"] : timezone = 2;
  root.containsKey("geo_enable") ? geo_enable = root["geo_enable"] : geo_enable = 0;
  root.containsKey("wifi_scan") ? wifi_scan = root["wifi_scan"] : wifi_scan = 1;
  root.containsKey("ir_loop") ? ir_loop = root["ir_loop"] : ir_loop = 0;
  root.containsKey("loop_433") ? loop_433 = root["loop_433"] : loop_433 = 0;
  root.containsKey("ws8211_loop") ? ws8211_loop = root["ws8211_loop"] : loop_433 = 0;

  setup_IOTManager();

  return true;
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
  Serial.println("SAVE: " + String(write_add, DEC));
  Serial.println(json);
  configFile.print(json);
  configFile.close();
  return true;
}


bool loadWidgets() {
  String  jsonConfig = readCommonFiletoJson("pin_setup");
  updatepinsetup(jsonConfig);
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
    Serial.println("FAIL!! numberChosed = 0");
    return false;
  }
  if (numberChosed > nWidgetsArray) {
    numberChosed = nWidgetsArray;
  }
  nWidgets = numberChosed;
  rootjs.containsKey("w433") ? w433rcv = rootjs["w433"] : w433rcv = 255;
  rootjs.containsKey("w433send") ? w433send = rootjs["w433send"] : w433send = 255;
  for (uint8_t i = 0; i < numberChosed; i++) {
    uint8_t pinmodeJS = rootjs["pinmode"][i];
    pinmode[i] = rootjs["pinmode"][i];
    char idJS = rootjs["id"][i];
    char pinJS = rootjs["pin"][i];
    String pageJS = rootjs["page"][i];
    String descrJS = rootjs["descr"][i];
    char widgetJS = rootjs["widget"][i];
    unsigned int defaultValJS = rootjs["defaultVal"][i];
    //int delimValJS = rootjs["aDiv"];
    char IrBtnId  = rootjs["IrBtnId"][i];
    if (pinmodeJS) {
      pinmode[i] = pinmodeJS;
    }
    id[i] = i;
    pin[i] = pinJS;
    if (descrJS) {
      descr[i] = descrJS;
    }
    if (widgetJS) {
      widget[i] = widgetJS;
    }
    if (IrBtnId) {
      IrButtonID[i] = IrBtnId;
      //Serial.println("IrButtonID[i]"+String(IrBtnId));
    }
  }
  analogDivider = rootjs["aDiv"];
  analogSubtracter = rootjs["aSusbt"];
  pwm_delay_long = rootjs["PWM_interval"];
  //  PowerCorrection = rootjs["PCorr"];
  rootjs.containsKey("PCorr") ? PowerCorrection = rootjs["PCorr"] : PowerCorrection = 111.1;

  rootjs.containsKey("router") ? router = rootjs["router"] : router = 255;
  //Serial.println("ROUTER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"+String(router,DEC));

  initThingConfig();
  /////////////////////////
  DynamicJsonBuffer jsonBuffer_stat;
  String stat1 = readCommonFiletoJson("stat");
  JsonObject& root_stat = jsonBuffer_stat.parseObject(stat1);

  for (char i = 0; i < numberChosed; i++) {
    int stat_js = root_stat["stat"][i];
    if (stat_js) {
      stat[i] = stat_js;
    }
    //Serial.println(stat_js);
  }
  ////////////////////////////
  Setup_pinmode();
  return true;
}

