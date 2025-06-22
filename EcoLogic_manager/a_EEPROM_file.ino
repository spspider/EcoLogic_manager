

void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}
#if defined(EEPROM)
void saveEEPROM(int adress, char value[5]) {
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
}
void saveEEPROM_char(int adress, char value) {
  char sep = 8;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Saved EEPROM:");
  Serial.println(String(getEEPROM_char(adress), DEC));
}
char *getEEPROM(int adress) {
  char buffer[5];
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.get(adress * sizeof(buffer) + sep, buffer);
  EEPROM.end();
  Serial.println("Recovered credentials:");
  Serial.println(buffer);
  return buffer;
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
}

void save_stat_void() {
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  JsonArray stat_json = jsonDocument.createNestedArray("stat");

  for (uint8_t i = 0; i < nWidgets; i++) {
    stat_json.add(stat[i]);
  }

  String buffer;
  serializeJson(jsonDocument, buffer);
  saveCommonFiletoJson("stat", buffer, 1);
}
#endif