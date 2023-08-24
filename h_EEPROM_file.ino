/** Load WLAN credentials from EEPROM */




void loadCredentials() {
  /*
    EEPROM.begin(512);
    EEPROM.get(0, ssid);
    EEPROM.get(0 + sizeof(ssid), password);
    char ok[2 + 1];
    EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);

    EEPROM.end();
    if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    }
    Serial.println("Recovered credentials:");
    Serial.println(ssid);
    //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
    Serial.println(password);
  */
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}
void saveEEPROM(int adress, char value[5] ) {
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
}
void saveEEPROM_char(int adress, char value ) {
  char sep = 8;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Saved EEPROM:");
  Serial.println(String(getEEPROM_char(adress), DEC));
}
char* getEEPROM(int adress) {
  char buffer[5];
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.get(adress * sizeof(buffer) + sep, buffer);
  EEPROM.end();
  Serial.println("Recovered credentials:");
  Serial.println(buffer);
  return buffer;
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}
char getEEPROM_char(int adress) {
  char buffer;
  char sep = 8;
  EEPROM.begin(512);
  EEPROM.get(adress * sizeof(buffer) + sep, buffer);
  EEPROM.end();
  Serial.println("Recovered EEPROM:");
  Serial.println(String(buffer, DEC));
  return buffer;
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}
/*
  void saveCredentialsAP() {
  EEPROM.begin(512);
  EEPROM.put(70, softAP_ssid);
  EEPROM.put(70 + sizeof(softAP_ssid), softAP_password);
  char ok[2 + 1] = "OK";
  EEPROM.put(70 + sizeof(softAP_ssid) + sizeof(softAP_password), ok);
  EEPROM.commit();
  EEPROM.end();
  }
  void  loadCredentialsAP() {
  EEPROM.begin(512);
  EEPROM.get(70, softAP_ssid);
  EEPROM.get(70 + sizeof(softAP_ssid), softAP_password);
  char ok[2 + 1];
  EEPROM.get(70 + sizeof(softAP_ssid) + sizeof(softAP_password), ok);

  EEPROM.end();
  if (String(ok) != String("OK")) {
  //    *softAP_ssid = "ESP_ap_dev_001";
  //    *softAP_password= "12345678";
  }
  Serial.println("Recovered credentials:");
  Serial.println(softAP_ssid);
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
  Serial.println(softAP_password);
  }
*/
/*
  void saveEEPROM(String topic_pub, int new_stat) {
  int that_widget = -1;
  for (int i = 0; i < nWidgets; i++) {
    //Serial.println("topic_pub:"+topic_pub+" sTopic:"+sTopic[i]);
    if (String(topic_pub) == String(sTopic[i])) {

      that_widget = i;
      //if (defaultVal[i]==new_stat){that_widget=-1;}
      break;
    }
  }
  if (that_widget != -1) {
    //if (defaultVal[that_widget]!=new_stat){
    int adress = that_widget * 5 + addr_widgets_begins;
    //Serial.println("saveEEPROM" + sTopic[that_widget] + " status:" + new_stat + " addr:" + adress);
    //String value
    EEPROM.begin(512);
    EEPROM.put(adress, new_stat);
    EEPROM.commit();
    EEPROM.end();
    //delay(100);
    /// }

  }
  }
*/
/////////////////////////////////////////////////////////////////////////////////////////
/*
  unsigned int *getSPIFFS_JSON_VALUE(int nWidgets) {
  unsigned int value_back[nWidgets];
  for (int i = 0; i < nWidgets; i++) {
    value_back[i] = defaultVal[i];
  }
  String buffer = readCommonFiletoJson("config");
  /*
    File configFile = SPIFFS.open("/config.txt", "r");
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
*/
/*
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buffer);
  if (!json.success()) {
  Serial.println("Failed to parse config file");
  return value_back;
  }

  //unsigned int value_back[nWidgets];
  for (int i = 0; i < nWidgets; i++) {

  const char* value_char = json[sTopic[i]];
  StaticJsonBuffer<200> jsonBufferParseStatus;
  JsonObject& jsonParseSt = jsonBufferParseStatus.parseObject(value_char);//сперва раскодируем

  const char* status_json = jsonParseSt["status"];
  String status_str = String((char*)status_json);
  unsigned int status_int = status_str.toInt();
  if (jsonParseSt.success()) {
    value_back[i] = status_int;
  } else {
    Serial.println("failed restore value:" + String(i));
    value_back[i] = defaultVal[i];
  }

  Serial.print("stored_value for topic:" + sTopic[i] + " is:");
  Serial.println(value_back[i]);
  //Serial.println(status_int);
  }

  return   value_back;

  }
*/
/////////////////////////////////////////////////////////////////////////////////////
/*
  bool saveSPIFFS() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  for (int i = 0; i < nWidgets; i++) {
    String id_t = sTopic[i];
    json[id_t] = stat[i];
  }

  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  //Serial.println("Saved:"+id_t+"value"+defaultVal[i]);
  json.printTo(configFile);
  json.printTo(Serial);
  return true;
  }
*/
void save_stat_void() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonArray& stat_json = json.createNestedArray("stat");
  for (uint8_t i = 0; i < nWidgets; i++) {
    stat_json.add(stat[i]);
  }
  /*
    String stat1 = "{\"stat\":[";
    for (char i = 0; i < nWidgets; i++) {
    stat1 += String(stat[i], DEC);
    stat1 += (i < nWidgets - 1) ? "," : "]";
    }
    stat1 += "}";
  */
  String buffer;
  json.printTo(buffer);
  //Serial.println(buffer);
  //Serial.print("nWidgets:" + String(nWidgets, DEC));
  saveCommonFiletoJson("stat", buffer, 1);
}
bool saveSPIFFS_jsonArray(int *stat_arr) {
  String buffer_read = readCommonFiletoJson("pin_setup");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buffer_read);
  JsonArray& defaultVal_json = json.createNestedArray("stat");

  for (int i = 0; i < nWidgets; i++) { //i < (sizeof(myInts)/sizeof(int));
    //Serial.println(stat_arr[i]);
    defaultVal_json.add(stat_arr[i]);
  }

  String buffer;
  json.printTo(buffer);
  //Serial.println(buffer);
  saveCommonFiletoJson("pin_setup", buffer, 1);

}
////////////////////////////////////////////////////////////////////////////////
/*
  unsigned int getEEPROM(int id) {
  int value_back = 0;
  int adress = id * 5 + addr_widgets_begins;
  EEPROM.begin(512);
  delay(100);
  EEPROM.get(adress, value_back);
  delay(500);
  Serial.print("adress:");
  Serial.print(adress);
  Serial.print(" id:");
  Serial.print(id);
  Serial.println(" RestoredValue:" + value_back);
  delay(500);
  EEPROM.end();

  return value_back;
  }
*/
