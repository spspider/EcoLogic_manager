// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson
// If you like this project, please add a star!




/*
  void WebScoketCallback(String text) {
  DynamicJsonBuffer jsonBufferParse;
  JsonObject& jsonParse = jsonBufferParse.parseObject(text);

  // Test if parsing succeeds.
  if (!jsonParse.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  const char* Topic_is = jsonParse["topic"];
  const char* newValue = jsonParse["newValue"];
  //Serial.print(Topic_is);
  //Serial.print(newValue);
  callback_scoket(Topic_is, newValue);
  }
*/
void callback_scoket(char i, int payload_is) {
  bool that_Ajax = false;
  bool saveEEPROM = false;
  if (i == 127) { //выслать только статус
    pubStatusFULLAJAX_String(false);
    return;
  }
  stat[i] = (payload_is ^ defaultVal[i]);
  if ((pinmode[i] == 2) || (pinmode[i] == 1)) { //out,in - saveEEPROM=false;
    //Serial.println("SendIR");
    //send_IR(i);
    digitalWrite(pin[i], stat[i]);
    check_if_there_timer_once(i);
  } else if (pinmode[i] == 3) {//pwm
    analogWrite(pin[i], payload_is);
  }
  else if (pinmode[i] == 5) {//low_pwm
    //String x = payload_is;
    low_pwm[i] = payload_is;

  }
  else if (pinmode[i] == 4) { //adc
  }
  //}

  //}

  //check_for_changes();
  pubStatusFULLAJAX_String(false);

}
//void loop_websocket() {
  //loop_pwm();
//}
bool loop_pwm() {
  int pwm_long = pwm_delay_long * 240;
  for (unsigned char i = 0; i < nWidgets; i++) {
    if (pinmode[i] == 5) {//low_pwm
      newtimePWM = millis();//таймер для 1 сек
      if (!low_pwm_off) {
        if (newtimePWM - oldtimePWM > (low_pwm[i] * (pwm_long / 1024))) { // если миллисекунды с старта больше записанного значения
          digitalWrite(pin[i], 0);//далее - выключаем
          low_pwm_off = true;
        }
      }
      if (newtimePWM - oldtimePWM > pwm_long) { // 1 sec//
        digitalWrite(pin[i], 1);//включаем снова
        oldtimePWM = newtimePWM;
        low_pwm_off = false;
      }
    }
    if (pinmode[i] == 7) { //MQ7
      newtimePWM = millis();//таймер для 1 сек
      if (low_pwm_off) {
        if (newtimePWM - oldtimePWM > (90 * 1000)) { //90 секунд
          analogWrite(pin[i], 1024);//5V
          low_pwm_off = false;
          oldtimePWM = newtimePWM;
        }
      } else {
        if (newtimePWM - oldtimePWM > (60 * 1000)) { // 60 секунд
          analogWrite(pin[i], 286);//1.4V
          //analogWrite(pin[i], 0);//1.4V
          low_pwm_off = true;
          oldtimePWM = newtimePWM;
        }

      }
    }
  }
}

void pubStatusFULLAJAX_String(bool save_eeprom) { //отправка на сервер _nobuffer
  String stat1 = "{\"stat\":[";
  for (char i = 0; i < nWidgets; i++) {
    float that_stat = 0.0F;
    that_stat = stat[i];
    that_stat = get_new_pin_value(i);
    //that_stat = that_stat ^ defaultVal(i);
    //get_new_pin_value()
    stat[i] = (int)that_stat;
    stat1 += String(that_stat, 2);
    stat1 += (i < nWidgets - 1) ? "," : "]";
  }
  stat1 += "}";
  String buffer = stat1;
  //Serial.println(buffer);
  server.send(200, "text / json", buffer);
}

/*
  void pubStatusFULLAJAX_buffer(bool save_eeprom) { //отправка на сервер
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonArray& stat_array = json.createNestedArray("stat");
  for (int i = 0; i < nWidgets; i = i + 1) {
    //JsonObject& stat_array = networks.createNestedObject();
    int that_stat = -123;
    that_stat = stat[i];
    if ((pinmode[i] == 1) || (pinmode[i] == 2)) {//int,out
      that_stat = digitalRead(pin[i]);
      stat[i] = that_stat;
    }
    else if ((pinmode[i] == 3) || (pinmode[i] == 5) ) { //pwm не знаю как сделать,статус не обновляется, как прочитать analogWrite

    }
    else if (pinmode[i] == 4) {//adc
      int analog = (analogRead(pin[i])) ; //adc pin:A0
      that_stat = (analog / analogDivider) + analogSubtracter;
      stat[i] = that_stat;
    }
    else {

    }

    stat_array.add(String(that_stat));
  }
  Serial.print("nWidjets:" + String(nWidgets));
  json.printTo(Serial);
  String buffer;
  json.printTo(buffer);
  server.send(200, "text/json", buffer);
  }
*/
