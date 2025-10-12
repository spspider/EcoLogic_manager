unsigned char lastCheck = 0;
const unsigned char checkInterval = 5; // 5 seconds


void loop_ecologicclient() {
  if ((unsigned char)(onesec_255 - lastCheck) >= checkInterval) {
    syncWithServer();
    lastCheck = onesec_255;
  }
}
// Буферы для экономии памяти
char device_id[32];  // Увеличенный размер для MAC (глобальная)
static char device_token[] = "tk01";  // Короткий токен



void uploadConfig_ecologicclient() {
  
  if (!LittleFS.begin()) {
    Serial.println("LittleFS fail");
    return;
  }

  File file = LittleFS.open("/pin_setup.txt", "r");
  if (!file) {
    Serial.println("pin_setup.txt not found");
    return;
  }

  // Читаем файл порциями для экономии памяти
  String config = file.readString();
  file.close();
  
  if (config.length() > 800) {  // Ограничиваем размер
    Serial.println("Config too large");
    return;
  }

  // Короткий URL
  String url = String(server_url) + "/api/cfg?id=" + device_id + "&tk=" + device_token;
  
  if (url.length() > 200) {  // Проверяем длину URL
    Serial.println("URL too long");
    return;
  }

  if (!http.begin(wclient, url)) {
    Serial.println("HTTP begin fail");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);  // 5 сек таймаут

  int httpCode = http.POST(config);
  
  if (httpCode == 200) {
    Serial.println("Config uploaded OK");
  } else {
    Serial.printf("Upload failed: %d\n", httpCode);
  }

  http.end();
}

void syncWithServer() {
  DynamicJsonDocument doc(512);
  JsonArray statArray = doc.createNestedArray("stat");
  for (int i = 0; i < nWidgets; i++) {
    float value = get_new_widjet_value(i);
    statArray.add(String(value, 2));
  }
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  String url = String(server_url) + "/api/sync?id=" + device_id + "&tk=" + device_token;
  
  if (!http.begin(wclient, url)) {
    Serial.println("HTTP begin fail");
    return;
  }
  
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument responseDoc(512);
    deserializeJson(responseDoc, payload);
    
    if (responseDoc.containsKey("stat") && responseDoc.containsKey("upd")) {
      unsigned char has_updates = responseDoc["upd"].as<int>();
      
      // Применяем изменения только если есть обновления
      if (has_updates == 1) {
        JsonArray states = responseDoc["stat"];
        for (int i = 0; i < states.size() && i < nWidgets; i++) {
          int pinState = states[i].as<int>();
          write_new_widjet_value(i, pinState);
        }
        Serial.println("Updates applied");
      }
    }
    Serial.println("Sync OK");
  } else {
    Serial.printf("Sync failed: %d\n", httpCode);
  }
  
  http.end();
}
