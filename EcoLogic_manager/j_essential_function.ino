void generate_device_id() {
    String mac = WiFi.macAddress();  // Get MAC address
    mac.replace(":", "");            // Remove colons
    strncpy(device_id, mac.c_str(), sizeof(device_id));  // Copy to char array
    device_id[sizeof(device_id) - 1] = '\0';  // Ensure null-termination
}

float get_new_widjet_value(uint8_t i) {

  float that_stat = (float)stat[i];

  if (pin[i] == 255) {
    // return that_stat;
  }
  if (pinmode[i] == 1) {  // in
    that_stat = digitalRead(pin[i]) ^ defaultVal[i];
    stat[i] = that_stat;
    return that_stat;
  }
  if ((pinmode[i] == 2)) {  // out
    that_stat = digitalRead(pin[i]);
    return that_stat;
  }
  if (pinmode[i] == 3)
  { // pwm
    stat[i] = (int)that_stat;
    return that_stat;
  }
#if !defined(USE_EMON)
  if ((pinmode[i] == 4))
  { //adc
    that_stat = (float)analogRead(A0);
    stat[i] = (int)that_stat;
    return that_stat;
  }
#endif
  if (pinmode[i] == 5) {  // dht Temp
#if defined(USE_DHT)
    float temperature = dht.getTemperature();
    // Serial.println("DHT11 Temperature reading: " + String(temperature));
    if (!isnan(temperature) && temperature != 0) {
      that_stat = temperature;
      stat[i] = that_stat;
      // Serial.println("DHT11 Temperature OK: " + String(that_stat));
    } else {
      // Serial.println("DHT11 Temperature ERROR - using previous value: " + String(stat[i]));
      that_stat = stat[i];  // Use previous value
    }
#endif
    return that_stat;
  }
  if (pinmode[i] == 6) {  // dht Humidity
#if defined(USE_DHT)
    float humidity = dht.getHumidity();
    // Serial.println("DHT11 Humidity reading: " + String(humidity));
    if (!isnan(humidity) && humidity != 0) {
      that_stat = humidity;
      stat[i] = that_stat;
      // Serial.println("DHT11 Humidity OK: " + String(that_stat));
    } else {
      // Serial.println("DHT11 Humidity ERROR - using previous value: " + String(stat[i]));
      that_stat = stat[i];  // Use previous value
    }
#endif
    return that_stat;
  }

  if (pinmode[i] == 7)
  { // remote
    that_stat = getHttp(String(descr[i])).toFloat();
    return that_stat;
  }
  if (pinmode[i] == 11)
  {
#if defined(USE_AS5600)
    that_stat = (encoder.getAngle() - analogSubtracter) / analogDivider * 1.0F;
#endif
    return that_stat;
  }
  if (pinmode[i] == 9) {  // MAC ADRESS
    return that_stat;
  }
  if (pinmode[i] == 12) {  // EncA
    that_stat = no_internet_timer;
    return that_stat;
  }
  if (pinmode[i] == 13) {  // EncB
    // that_stat = stat[i] ^ 1;
    return that_stat;
  }
  if (pinmode[i] == 14) {  // ads1115
#if defined(ads1115)
    that_stat = (ads.readADC_SingleEnded(defaultVal[i]));
    return that_stat;
#endif
  }
  if (pinmode[i] == 10) {
#if defined(USE_DS18B20)
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(defaultVal[i]);
    if ((tempC != DEVICE_DISCONNECTED_C) && (tempC != 0)) {
      that_stat = tempC;
      stat[i] = that_stat;
      return that_stat;
    }
#endif
#if !defined(USE_DS18B20)
    that_stat = -1111;
    return that_stat;
#endif
    return stat[i];
  }

  if (pinmode[i] == 8)
  { // PowerMeter должен быть последним, иначе ошибка jump to case label
    // double Irms ;
#if defined(USE_EMON)
    that_stat = (float)emon1.calcIrms(1480); // Calculate Irms only
    return that_stat;                        // that_stat = (that_stat * 1.0F / analogDivider) + analogSubtracter;
#endif
    return that_stat;
  }
  if ((isnan(that_stat)) || (isinf(that_stat))) {
    that_stat = stat[i];  // 0
    return that_stat;
  }

  return -123.12;
}

void makeAres_sim(String json) {
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, json);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Failed to parse JSON");
    return;
  }

  JsonObject root = jsonDocument.as<JsonObject>();
  uint8_t that_pin;
  float that_val = 0.0F;
  uint8_t control = 255;
  uint8_t that_stat = 255;
  String String_value = "";

  root.containsKey("pin") ? that_pin = root["pin"] : that_pin = 255;
  root.containsKey("stat") ? that_stat = root["stat"] : that_stat = 255;
  root.containsKey("val") ? that_val = root["val"] : that_val = -1;
  root.containsKey("C") ? control = root["C"] : control = 255;
  root.containsKey("st") ? String_value = root["st"].as<String>() : String_value = "";

  switch (control) {
    case 255:
      {
        uint8_t i = 255;
        for (uint8_t i1 = 0; i1 < nWidgets; i1++) {
          if (that_pin == pin[i1]) {
            i = i1;
            break;  // Fix for the loop termination condition
          }
        }

        if (that_stat != 255) {
          if (root.containsKey("val")) {
            stat[that_stat] = that_val;
          } else {
            that_val = get_new_widjet_value(that_stat);  // только чтение
          }
        }

        if (i != 255) {
          if ((pinmode[i] == 2) || (pinmode[i] == 1)) {  // out, in
            stat[i] = static_cast<int>(that_val) ^ defaultVal[i];
            // send_IR(i);
            digitalWrite(that_pin, stat[i]);
          } else if (pinmode[i] == 3) {  // pwm
            analogWrite(that_pin, that_val);
          }
        }

        // pubStatusFULLAJAX_String(false);
        that_val = round(that_val * 200) / 200;
        server.send(200, "text / json", String(that_val, DEC));
        break;
      }
    case 1:  // PLUS Control
      {
        //        bySignalPWM[that_pin][that_stat] = that_val;
        //        server.send(200, "text / json",  saveConditiontoJson(that_pin));
        break;
      }
    case 2:  // IR
      {
#if defined(USE_IRUTILS)
        send_IR(that_stat);
#endif
        break;
      }
    case 3:
      {
        // irsend.sendNEC(StrToHex(String_value.c_str()), 32);
        break;
      }
    case 4:
      {
        // irsend.sendRaw(String_value, String_value.length(), 38);
        break;
      }
  }
}
