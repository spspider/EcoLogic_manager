void callback_socket(uint8_t i, int payload_is)
{
  bool that_Ajax = false;
  bool saveEEPROM = false;
  if (i == 127)
  { // выслать только статус
    pubStatusFULLAJAX_String(false);
    return;
  }

  if ((pinmode[i] == 2) || (pinmode[i] == 1))
  {                         // out,in - saveEEPROM=false;
    stat[i] = (payload_is); // ^ defaultVal[i]
    digitalWrite(pin[i], payload_is);
#if defined(timerAlarm)
    check_if_there_timer_once(i);
#endif
  }
  else if (pinmode[i] == 3)
  { // pwm
    if (!license)
      return;
    payload_is = defaultVal[i] != 0 ? defaultVal[i] - payload_is : payload_is;
    analogWrite(pin[i], payload_is);
    stat[i] = payload_is;
  }
  else if (pinmode[i] == 5)
  { // low_pwm
    if (!license)
      return;
    // String x = payload_is;
    low_pwm[i] = payload_is;
    stat[i] = (payload_is); // ^ defaultVal[i]
  }
  else if (pinmode[i] == 4)
  { // adc
  }
  else if (pinmode[i] == 12)
  { // mac adress
    if (!license)
      return;
#if defined(wakeOnLan)
    const char *mac_adress = (const char *)descr;
    wakeMyPC(mac_adress);
#endif
  }
  else if (pinmode[i] == 11)
  { // Dimmer
    // DimmerVal = payload_is;
    // dimmer.setPower(DimmerVal);
  }
  pubStatusShortAJAX_String(i);
  // pubStatusFULLAJAX_String(false);
}

void loop_pwm() {
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
          analogWrite(pin[i], 286);                  // 1.4V
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
    float that_stat = get_new_pin_value(i);
    stat1 += "\"";
    stat1 +=  String(that_stat, 2);
    stat1 += "\"";
    stat1 += (i < nWidgets - 1) ? "," : "]";
  }
  stat1 += "}";
  String buffer = stat1;
  server.send(200, "text / json", buffer);
}
void pubStatusShortAJAX_String(uint8_t i)
{
  server.send(200, "text / plain", String(get_new_pin_value(i)));
}

// void pubStatusShortAJAX_String(uint8_t i)
// {
//   char buffer[6]; // Adjust the size based on the maximum length of the short int
//   snprintf(buffer, sizeof(buffer), "%d", get_new_pin_value(i));
//   server.send(200, "text/plain", buffer);
// }