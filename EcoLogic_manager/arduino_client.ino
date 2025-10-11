unsigned long lastCheck = 0;
const unsigned long checkInterval = 5000; // 5 seconds


// void loop_ecologicclient() {
//   if (millis() - lastCheck > checkInterval) {
//     checkForUpdates();
//     lastCheck = millis();
//   }
//   delay(100);
// }
// Буферы для экономии памяти
char device_id[32];  // Увеличенный размер для MAC (глобальная)
static char device_token[] = "tk01";  // Короткий токен
static bool device_id_generated = false;

void generate_device_id() {
  if (!device_id_generated) {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    uint32_t chip = ESP.getChipId();
    snprintf(device_id, sizeof(device_id), "esp%06X_%s", chip & 0xFFFFFF, mac.c_str());
    device_id_generated = true;
  }
}

void uploadConfig_ecologicclient() {
  generate_device_id();  // Генерируем ID однажды
  
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

// void checkForUpdates() {
//   http.begin(wclient, String(serverURL) + "/api/get_state");
//   int httpCode = http.GET();
  
//   if (httpCode == 200) {
//     String payload = http.getString();
//     DynamicJsonDocument doc(1024);
//     deserializeJson(doc, payload);
    
//     if (doc.containsKey("stat")) {
//       JsonArray states = doc["stat"];
//       for (int i = 0; i < states.size(); i++) {
//         int pinState = states[i].as<int>();
//         // Apply pin state to actual hardware pins
//         // digitalWrite(pinMap[i], pinState);
//         Serial.printf("Pin %d: %d\n", i, pinState);
//       }
//     }
//   }
//   http.end();
// }