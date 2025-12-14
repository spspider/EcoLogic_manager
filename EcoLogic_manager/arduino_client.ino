unsigned char lastCheck = 0;
extern unsigned char sync_interval;

void loop_ecologicclient() {
  if ((unsigned char)(onesec_255 - lastCheck) >= sync_interval) {
      syncWithServer();
      lastCheck = onesec_255;
  }
}
// Буферы для экономии памяти
char device_id[32];  // Увеличенный размер для MAC (глобальная)
static char device_token[] = "tk01";  // Короткий токен
static bool updates_applied = false;  // Флаг применения обновлений



void uploadConfig_ecologicclient() {
  if (WiFi.status() != WL_CONNECTED) {return;}
  if (!LittleFS.begin()) {
    Serial.println("LittleFS fail");
    return;
  }

  // Отправляем other_setup.txt
  File otherFile = LittleFS.open("/other_setup.txt", "r");
  if (otherFile) {
    String otherConfig = otherFile.readString();
    otherFile.close();
    
    String otherUrl = String(server_url) + "/api/other?id=" + device_id + "&tk=" + device_token;
    if (http.begin(wclient, otherUrl)) {
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(5000);
      int otherCode = http.POST(otherConfig);
      if (otherCode == 200) {
        Serial.println("Other setup uploaded OK");
      }
      http.end();
    }
  }

  // Отправляем pin_setup.txt
  File file = LittleFS.open("/pin_setup.txt", "r");
  if (!file) {
    Serial.println("pin_setup.txt not found");
    return;
  }

  String config = file.readString();
  file.close();
  
  if (config.length() > 800) {
    Serial.println("Config too large");
    return;
  }

  String ip_address = WiFi.localIP().toString();
  String url = String(server_url) + "/api/cfg?id=" + device_id + "&tk=" + device_token + "&ip=" + ip_address;
  
  if (!http.begin(wclient, url)) {
    Serial.println("HTTP begin fail");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);

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
  
  // Добавляем флаг синхронизации если были применены обновления
  if (updates_applied) {
    doc["synced"] = 1;
    updates_applied = false;
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
        updates_applied = true;  // Устанавливаем флаг что обновления применены
        Serial.println("Updates applied");
        syncWithServer();  // Немедленно синхронизируем снова для отправки подтверждения и нового статуса
      }
    }
    // Serial.println("Sync OK");
  } else {
    Serial.printf("Sync failed: %d\n", httpCode);
  }
  
  http.end();
}
