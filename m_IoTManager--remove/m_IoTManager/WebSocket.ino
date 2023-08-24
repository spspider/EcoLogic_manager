
/*
   WebSocketServer.ino

    Created on: 22.05.2015

*/
/*
  #include <ESP8266WiFi.h>
  #include <WebSocketsServer.h>
  #include <Hash.h>
  WebSocketsServer webSocket = WebSocketsServer(81);

  //unsigned char NumberWebSocket;
  #define USE_SERIAL Serial1
  uint8_t NumberWebSocket;
  bool WebSocketConnected = false;
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  NumberWebSocket = num;
  switch (type) {
    case WStype_DISCONNECTED:
      WebSocketConnected = false;
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        WebSocketConnected = true;
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");

      }
      break;
    case WStype_TEXT:

      String text2 = String((char *) &payload[0]);

      if  (text2 == "HELLO") {
        Serial.println("HELLO recieved");
        pubConfigWS();
      }
      else if (text2 == "STATUS") {
        Serial.println("STATUS recieved");
        retriveNewStatus();
      }
      else {
        //char __text[text2.length() + 1];
        //text2.toCharArray(__text, text2.length() + 1);
        //char* text = ((char *) &payload[0]);
        //char* text = ((char *) &payload[0]);
        WebScoketCallback(text2);
      }
      break;
      /*
        case WStype_BIN:
        USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
        hexdump(payload, lenght);

        // send message to client
        // webSocket.sendBIN(num, payload, lenght);
        break;
*/
/*
  }

  }

  void setup_websoket() {

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  }
*/
void loop_websocket() {

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
  static unsigned long l = 0;                     // only initialized once
  //unsigned long t;
  /*
    webSocket.loop();
  */


  if ((millis() - l) > 120000) {                           // update temp every 5 seconds
    save_stat();
    l = millis();                                      // typical runtime this IF{} == 300uS - 776uS measured
    /*
      if (WebSocketConnected) {
      for (int i = 0; i < nWidgets; i = i + 1) {
        if (pinmode[i] == "in") {
          stat[i] = setStatus(digitalRead(pin[i]));
          //Serial.println("save:" + String(pin[i]) + "i:" + i);
          pubStatusWS(sTopic[i], stat[i], false);
        }
        if (pinmode[i] == "adc") {
          stat[i] = setStatus(analogRead(pin[i]));//adc pin:A0
          //Serial.println("save read adc:" + String(pin[i]) + "i:" + String(i) + "stat:" + String( stat[i]));
          pubStatusWS(sTopic[i], stat[i], false);
        }
      }
      //Serial.println("websocket sended");
      yield();
      }
    */
  }
  //  dim_check();
}
/*
  void zero_cross_detect() {
  //zero_cross = true;
  for (char i = 0; i < nWidgets; i++) {
    if (pinmode[i] == 10) {
      digitalWrite(pin[i], LOW);
    }
  }
  Serial.println(next);
  next = 0;
  }
*/
/*
  void dim_check() {
  if (zero_cross == true) {
    //unsigned long pass_microsec = micros() - zerro_cross_milisec;
    for (char i = 0; i < nWidgets; i++) {
      if (pinmode[i] == 10) {
        if (pass_microsec >= stat[i]) {
          digitalWrite(pin[i], HIGH); // turn on light
        }
      }
    }
    if (pass_microsec > 20) {
      zerro_cross_milisec = micros();
      zero_cross = false; //reset zero cross detection
    }

  }
  else {
    next++;; // increment time step counter
  }
  }
*/

/*
  void dim_check2(void *pArg) {
  next++;

    if (zero_cross == true) {
    unsigned long pass_millisec = millis() - zerro_cross_milisec;
    for (char i = 0; i < nWidgets; i++) {
      if (pinmode[i] == 10) {
        if (pass_millisec >= stat[i]) {
          digitalWrite(pin[i], HIGH); // turn on light
        }
      }
    }
    if (pass_millisec > 20) {
      zerro_cross_milisec = millis();
      zero_cross = false; //reset zero cross detection
    }
    }
  }
*/
/*
  void retriveNewStatus() {
  for (int i = 0; i < nWidgets; i = i + 1) {
    if ((pinmode[i] == 1) || (pinmode[i] == 2)) {//int,out
      int new_stat = (digitalRead(pin[i]));
      if (new_stat != stat[i]) {
        stat[i] = new_stat;
        //pubStatusWS(sTopic[i],setStatus(stat[i]), false);
      }
      //stat[i] = setStatus(digitalRead(pin[i]));
      //Serial.println("save:" + String(pin[i]) + "i:" + i);
      //pubStatusWS(sTopic[i], stat[i], false);
    }
    if (pinmode[i] == 3) { //pwm не знаю как сделать,статус не обновляется, как прочитать analogWrite
     // String new_stat = setStatus(digitalRead(pin[i]));
    }
    if (pinmode[i] == 4) {//adc
      stat[i] = (analogRead(pin[i]));//adc pin:A0
      //Serial.println("save read adc:" + String(pin[i]) + "i:" + String(i) + "stat:" + String( stat[i]));
      // pubStatusWS(sTopic[i], stat[i], false);
      //pubStatusWS(sTopic[i],setStatus( stat[i]), false);
    }
  }
  }
*/
/*
  void pubStatusWs() {
  for (int i = 0; i < nWidgets; i = i + 1) {
    if ((pinmode[i] == "in") || (pinmode[i] == "adc")|| (pinmode[i] == "adc")) {
      pubStatusWS(sTopic[i], stat[i], false);
    }
  }
  }
*/

