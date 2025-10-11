void callback_socket(uint8_t i, int payload_is) {
  bool that_Ajax = false;
  bool saveEEPROM = false;
  if (i == 127) {  // выслать только статус
    pubStatusFULLAJAX_String(false);
    return;
  }

  if (pinmode[i] == 3)
  { // pwm
    payload_is = defaultVal[i] != 0 ? defaultVal[i] - payload_is : payload_is;
    analogWrite(pin[i], payload_is);
    stat[i] = payload_is;
  }
  else if (pinmode[i] == 9)
  { // mac adress
#if defined(wakeOnLan)
    const char *mac_adress = (const char *)descr;
    wakeMyPC(mac_adress);
#endif
  }
  else if (pinmode[i] == 2)
  { // out
    stat[i] = (uint8_t)payload_is;
    digitalWrite(pin[i], payload_is);
#if defined(timerAlarm)
    check_if_there_timer_once(i);
#endif
  }
  else if (pinmode[i] == 1)
  { // in
    stat[i] = (uint8_t)payload_is;
    digitalWrite(pin[i], payload_is);
#if defined(timerAlarm)
    check_if_there_timer_once(i);
#endif
  }
  pubStatusShortAJAX_String(i);
  // pubStatusFULLAJAX_String(false);
}

void pubStatusFULLAJAX_String(bool save_eeprom) {  // отправка на сервер _nobuffer
  String stat1 = "{\"stat\":[";
  for (uint8_t i = 0; i < nWidgets; i++) {
    float that_stat = get_new_pin_value(i);
    stat1 += "\"";
    stat1 += String(that_stat, 2);
    stat1 += "\"";
    stat1 += (i < nWidgets - 1) ? "," : "]";
  }
  stat1 += "}";
  String buffer = stat1;
  server.send(200, "text / json", buffer);
}
void pubStatusShortAJAX_String(uint8_t i) {
  server.send(200, "text / plain", String(get_new_pin_value(i)));
}
#if defined(USE_IRUTILS)
void sendIRCode_toServer(uint32_t code) {
  if ((WiFi.status() == WL_CONNECTED) && (IR_recieve)) {
    if (!nodered_address || nodered_address[0] == '\0' || strcmp(nodered_address, "0.0.0.0") == 0) return;
    HTTPClient http_client_code;
    WiFiClient client_code;
    char url[64];
    snprintf(url, sizeof(url), "http://%s/ir", nodered_address);
    http_client_code.begin(client_code, url);
    http_client_code.addHeader("Content-Type", "application/json");
    char body[32];
    snprintf(body, sizeof(body), "{\"code\":\"%lX\"}", code);
    int status = http_client_code.POST(body);
    http_client_code.end();
  }
}
#endif

void sendPinStatus_toNodeRed(int pin, int value) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http_client_pin;
    WiFiClient client_pin;
    char url[64];
    snprintf(url, sizeof(url), "http://%s/pin", nodered_address);
    http_client_pin.begin(client_pin, url);
    http_client_pin.addHeader("Content-Type", "application/json");
    char body[64];
    snprintf(body, sizeof(body), "{\"pin\":%d,\"value\":%d}", pin, value);
    int status = http_client_pin.POST(body);
    http_client_pin.end();
  }
}
void check_new_status_and_send_nodeRed() {
  if (!send_to_nodeRed || !enable_http_requests) return;
  if (!nodered_address || nodered_address[0] == '\0' || strcmp(nodered_address, "0.0.0.0") == 0) return;
  
  static unsigned long lastNodeRedSend = 0;
  if (millis() - lastNodeRedSend < 10000) return; // Ограничиваем частоту
  
  for (int i = 0; i < N_WIDGETS; i++) {
    int new_value = get_new_pin_value(i);
    if (new_value != stat[i]) {
      sendPinStatus_toNodeRed(i, new_value);
      stat[i] = new_value;
    }
  }
  lastNodeRedSend = millis();
}
// void pubStatusShortAJAX_String(uint8_t i)
// {
//   char buffer[6]; // Adjust the size based on the maximum length of the short int
//   snprintf(buffer, sizeof(buffer), "%d", get_new_pin_value(i));
//   server.send(200, "text/plain", buffer);
// }