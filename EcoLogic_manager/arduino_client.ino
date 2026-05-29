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

  char uploadUrl[128];

  // Відправляємо other_setup.txt
  File otherFile = LittleFS.open("/other_setup.txt", "r");
  if (otherFile) {
    char otherConfig[512];  // Use char buffer instead of String (saves ~500 bytes)
    size_t otherSize = otherFile.readBytes(otherConfig, sizeof(otherConfig) - 1);
    otherConfig[otherSize] = '\0';
    otherFile.close();

    snprintf(uploadUrl, sizeof(uploadUrl), "%s/api/other?id=%s&tk=%s", server_url, device_id, device_token);
    if (http.begin(wclient, uploadUrl)) {
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(5000);
      http.setReuse(false);  // Don't keep connection alive (reduces memory)
      int otherCode = http.POST(otherConfig);
      if (otherCode == 200) {
        Serial.println("Other setup uploaded OK");
      }
      http.end();  // Always free buffers
    }
  }

  // Відправляємо pin_setup.txt
  File file = LittleFS.open("/pin_setup.txt", "r");
  if (!file) {
    Serial.println("pin_setup.txt not found");
    return;
  }

  char config[768];  // Use char buffer instead of String (saves ~500 bytes)
  size_t configSize = file.readBytes(config, sizeof(config) - 1);
  config[configSize] = '\0';
  file.close();

  if (configSize > 800) {
    Serial.println("Config too large");
    return;
  }

  IPAddress localIp = WiFi.localIP();
  snprintf(uploadUrl, sizeof(uploadUrl), "%s/api/cfg?id=%s&tk=%s&ip=%d.%d.%d.%d",
           server_url, device_id, device_token, localIp[0], localIp[1], localIp[2], localIp[3]);

  if (!http.begin(wclient, uploadUrl)) {
    Serial.println("HTTP begin fail");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  http.setReuse(false);  // Don't keep connection alive (reduces memory)

  int httpCode = http.POST(config);
  
  if (httpCode == 200) {
    Serial.println("Config uploaded OK");
  } else {
    Serial.printf("Upload failed: %d\n", httpCode);
  }

  http.end();  // Always free buffers
}

void syncWithServer() {
  StaticJsonDocument<512> doc;
  JsonArray statArray = doc.createNestedArray("stat");
  char valueBuf[12];
  for (int i = 0; i < nWidgets; i++) {
    float value = get_new_widjet_value(i);
    dtostrf(value, 1, 2, valueBuf);
    statArray.add((const char*)valueBuf);
  }

  if (updates_applied) {
    doc["synced"] = 1;
    updates_applied = false;
  }

  char jsonBuf[200];
  serializeJson(doc, jsonBuf, sizeof(jsonBuf));

  char url[128];
  snprintf(url, sizeof(url), "%s/api/sync?id=%s&tk=%s", server_url, device_id, device_token);

  if (!http.begin(wclient, url)) {
    Serial.println("HTTP begin fail");
    return;
  }

  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  http.setReuse(false);  // Don't keep connection alive (reduces memory)

  int httpCode = http.POST(jsonBuf);

  if (httpCode == 200) {
    StaticJsonDocument<256> responseDoc;
    deserializeJson(responseDoc, http.getStream());

    if (responseDoc.containsKey("stat") && responseDoc.containsKey("upd")) {
      unsigned char has_updates = responseDoc["upd"].as<int>();

      if (has_updates == 1) {
        JsonArray states = responseDoc["stat"];
        for (int i = 0; i < states.size() && i < nWidgets; i++) {
          int pinState = states[i].as<int>();
          write_new_widjet_value(i, pinState);
        }
        updates_applied = true;
        Serial.println("Updates applied");
        lastCheck = 0;  // Schedule immediate resync on next loop — recursive call removed (was causing stack overflow)
      }
    }
  } else {
    Serial.printf("Sync failed: %d\n", httpCode);
  }

  http.end();
}
