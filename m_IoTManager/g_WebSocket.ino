void loop_websocket()
{

  unsigned long pwm_long = pwm_delay_long * 240;
  for (uint8_t i = 0; i < nWidgets; i++)
  {
    if (pinmode[i] == 5)
    {                        // low_pwm
      newtimePWM = millis(); // таймер для 1 сек
      if (!low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (low_pwm[i] * (pwm_long / 1024)))
        {                          // если миллисекунды с старта больше записанного значения
          digitalWrite(pin[i], 0); // далее - выключаем
          low_pwm_off = true;
        }
      }
      if (newtimePWM - oldtimePWM > pwm_long)
      {                          // 1 sec//
        digitalWrite(pin[i], 1); // включаем снова
        oldtimePWM = newtimePWM;
        low_pwm_off = false;
      }
    }
    if (pinmode[i] == 7)
    {                        // MQ7
      newtimePWM = millis(); // таймер для 1 сек
      if (low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (90 * 1000))
        {                            // 90 секунд
          analogWrite(pin[i], 1024); // 5V
          low_pwm_off = false;
          oldtimePWM = newtimePWM;
        }
      }
      else
      {
        if (newtimePWM - oldtimePWM > (60 * 1000))
        {                           // 60 секунд
          analogWrite(pin[i], 286); // 1.4V
          // analogWrite(pin[i], 0);//1.4V
          low_pwm_off = true;
          oldtimePWM = newtimePWM;
        }
      }
    }
  }
}
