


float get_new_pin_value(unsigned char i) {


  float that_stat = (float)stat[i];

  if (pin[i] == 255) {
    //return that_stat;
  }
  if (pinmode[i] == 1) { //in
    that_stat = digitalRead(pin[i])^defaultVal[i];
    stat[i] = that_stat;
    return that_stat;
  }
  if ((pinmode[i] == 2) || (pinmode[i] == 3) || (pinmode[i] == 5)) { //out
    // that_stat = digitalRead(pin[i])^defaultVal[i];
    //stat[i] = that_stat;
    return that_stat;
  }
  if (pinmode[i] == 4) {//adc
    if (!license)return 127;
    //that_stat = (analogRead(pin[i]) / analogDivider) + analogSubtracter; //adc pin:A0
    //that_stat = (analogRead(17) - analogSubtracter) / analogDivider * 1.0F;
    stat[i] = (int)that_stat;
    return that_stat;
  }
  if (pinmode[i] == 6) { // dht Temp
    if (!license) return 127;
    float temperature = dht.getTemperature();
    if (!isnan(temperature)) {
      that_stat = temperature;
      stat[i] = that_stat;
    } else {
      that_stat = stat[i]; // Use a default value or handle the NaN case accordingly
    }

    return that_stat;
  }
  if (pinmode[i] == 7) {
    if (!license)return 127;
    that_stat = (float)low_pwm_off;
    return that_stat;
  }
  if (pinmode[i] == 8) { // dht Humidity
    if (!license) return 127;
    float humidity = dht.getHumidity();
    if (!isnan(humidity)) {
      that_stat = humidity;
      stat[i] = that_stat;
    } else {
      that_stat = stat[i]; // Use a default value or handle the NaN case accordingly
    }
    return that_stat;
  }

  if (pinmode[i] == 9) {//remote
    if (!license)return 127;
    that_stat = getHttp(String(descr[i])).toFloat();
    return that_stat;
  }
  if (pinmode[i] == 11) {
    if (!license)return 127;
    //compass
    //      that_stat = dimmer.getPower();

    //that_stat = (encoder.getAngle() - analogSubtracter) / analogDivider * 1.0F;
    return that_stat;

  }
  if (pinmode[i] == 12) {//MAC ADRESS
    //that_stat = stat[i] ^ 1;
    return that_stat;
  }
  if (pinmode[i] == 13) {//EncA
    that_stat = no_internet_timer;
    return that_stat;
  }
  if (pinmode[i] == 14) {//EncB
    //that_stat = stat[i] ^ 1;
    return that_stat;
  }
  if (pinmode[i] == 15) {//ads
#if defined(ads1115)
    that_stat = (ads.readADC_SingleEnded(defaultVal[i]));
    return that_stat;
#endif
  }
  if (pinmode[i] == 16) {//ds18b20
    //Serial.print ("stat[i]", stat[i]);
#if defined(ds18b20)
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(defaultVal[i]);
    if ((tempC != DEVICE_DISCONNECTED_C) && (tempC != 0))
    {
      that_stat = tempC;
      stat[i] = that_stat;
      return that_stat;
    }
#endif
#if !defined(ds18b20)
    that_stat = -1111;
    return that_stat;
#endif
    return stat[i];
  }


  if (pinmode[i] == 10) {//PowerMeter должен быть последним, иначе ошибка jump to case label
    if (!license)return 127;
    // double Irms ;
#if defined(emon)
    that_stat = (float) emon1.calcIrms(1480); // Calculate Irms only
    that_stat = (that_stat * 1.0F / analogDivider) + analogSubtracter;
#endif
    return that_stat;
  }

  /*
    case 11:



    break;
  */

//  that_stat = (isnan(that_stat) || isnanf (that_stat)) ? stat[i] : that_stat;
  if ((isnan(that_stat)) || ( isinf (that_stat))) {
    that_stat =  stat[i];//0
    return that_stat;
  }

  return -123.12;
}


void makeAres_sim(String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  char that_pin;
  float that_val = 0.0F;
  char control = 255;
  char that_stat = 255;
  //char that_nID;
  String String_value = "";
  root.containsKey("pin") ? that_pin = root["pin"] : that_pin = 255;
  root.containsKey("stat") ? that_stat = root["stat"] : that_stat = 255;
  //root.containsKey("read") ? that_stat = root["stat"] : that_stat = 255;
  root.containsKey("val") ? that_val = root["val"] : that_val = -1;
  //root.containsKey("nID") ? that_nID = root["nID"] : that_nID = 255;
  root.containsKey("C") ? control = root["C"] : control = 255;
  root.containsKey("st") ? String_value = root["st"].as<String>() : String_value = "";
  switch (control) {
    case 255: {
        char i = 255;
        for (char i1 = 0; i1 < nWidgets; i1++) {
          if (that_pin == pin[i1])
            i = i1;
          break;
        }

        if (that_stat != 255) {
          if  (root.containsKey("val")) {
            stat[that_stat] = that_val;
          } else {
            that_val = get_new_pin_value(that_stat);//только чтение
          }
        }
        if (i != 255) {
          if ((pinmode[i] == 2) || (pinmode[i] == 1)) {//out, in
            stat[i] = (int)that_val ^ defaultVal[i];
            //send_IR(i);
            digitalWrite(that_pin, stat[i]);
          }
          else if (pinmode[i] == 3) {//pwm
            //unsigned int freq = PWM_frequency * 100;
            //analogWriteFreq(freq);
            analogWrite(that_pin, that_val);
          }
        }

        //pubStatusFULLAJAX_String(false);
        that_val = round(that_val * 200) / 200;
        server.send(200, "text / json", String(that_val, DEC));
        break;
      }
    case 1://PLUS Control

      {
        bySignalPWM[that_pin][that_stat] = that_val;


        //PWM_json.add(bySignalPWM[that_pin][that_nID]);
        //that_pin-это условие
        /*
          for (char i1 = 0; i1 < Condition; i1++) {
          //JsonArray& C = PWM_json.createNestedArray();
          for (char i = 0; i < NumberIDs[i1]; i++) {
            if (bySignalPWM[i1][i] != -1) {
              PWM_json.add(bySignalPWM[i1][i]);
            }
          }
          //PWM_json.add(Condition_json);
          }
        */
        //String buffer;
        //json.printTo(buffer);
        //Serial.println(buffer);
        server.send(200, "text / json",  saveConditiontoJson(that_pin));
        break;
      }
    case 2: { //IR
        send_IR(that_stat);
        break;
      }
    case 3: {
        //irsend.sendNEC(StrToHex(String_value.c_str()), 32);
        break;
      }
    case 4: {
        //        irsend.sendRaw(String_value, String_value.length(), 38);
        break;
      }
  }
}

