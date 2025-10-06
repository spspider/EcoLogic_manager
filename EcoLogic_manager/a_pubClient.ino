//This variables left in another file, so they are out of scope

uint8_t oldtime = 0;
uint8_t nWidgets = N_WIDGETS;
const uint8_t nWidgetsArray = N_WIDGETS;
int16_t stat[nWidgetsArray];  // use int16_t for clarity, same as short int
// uint8_t widget[nWidgetsArray]; // inputWidjet[0] = 'unknown';1 = 'toggle';2 = 'simple-btn';4 = 'range';4 = 'small-badge';5 = 'uint8_tt';
char descr[nWidgetsArray][10];
uint8_t id[nWidgetsArray];
uint8_t pin[nWidgetsArray];
int16_t defaultVal[nWidgetsArray];
uint8_t IrButtonID[nWidgetsArray];
float analogDivider = 1.0F;
int16_t analogSubtracter = 0;
uint16_t low_pwm[nWidgetsArray];
bool low_pwm_off = false;        // low_pwm
uint8_t pinmode[nWidgetsArray];  // inputPinmode[0] = "no pin";inputPinmode[1] = "in";inputPinmode[2] = "out";inputPinmode[3] = "pwm";inputPinmode[4] = "adc";inputPinmode[5] = "low_pwm";inputPinmode[6] = "IR";inputPinmode[7] = "датчик газа MQ7";

uint8_t subscribe_loop = 0;
const uint8_t subscr_sec = 5;
uint8_t mqttspacing_oldtime;



#if defined(USE_PUBSUBCLIENT)

// Use static buffer only once, not per function call
template<typename T>
char *setStatus(T s) {
  static char stat[16];
  if constexpr (std::is_same<T, float>::value) {
    dtostrf(s, 1, 2, stat);
  } else {
    snprintf(stat, sizeof(stat), "%d", s);
  }
  return stat;
}

char *setStatus(const char *s) {
  static char stat[40];
  strncpy(stat, s, sizeof(stat) - 1);
  stat[sizeof(stat) - 1] = '\0';
  return stat;
}

void pubStatus(const char *t, const char *payload) {
  if (!client.connected())
    return;
  if (client.publish(t, payload)) {
#if defined(PUB_DEBUG)
    Serial.print(F("publish:"));
    Serial.print(t);
    Serial.print(F(", value: "));
    Serial.println(payload);
#endif
  }
}

void pubConfig()  // that is how I publish config, you dont need to adhere same structure
{
  char sTopic_ch[20];
  for (uint8_t i = 0; i < nWidgets; i++) {
    snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d", deviceID, i);
    pubStatus(sTopic_ch, setStatus(stat[i]));
  }
}

void callback(char *topic, byte *payload, uint8_t length)  // callback for recieving messages from subsriptions
{
  char *lastSlash = strrchr(topic, '/');
  uint8_t i = 0;
  if (lastSlash != NULL && *(lastSlash + 1) != '\0') {
    i = (uint8_t)atoi(lastSlash + 1);  // parse the number after the last '/'
  }
  char payloadBuffer[8];
  uint8_t copyLen = (length < sizeof(payloadBuffer) - 1) ? length : sizeof(payloadBuffer) - 1;
  strncpy(payloadBuffer, (char *)payload, copyLen);
  payloadBuffer[copyLen] = '\0';
  char *end;
  int newValue = strtol(payloadBuffer, &end, 10);
  callback_socket(i, newValue);
  //pub status back

  Serial.print(F("callback:"));
  Serial.print(i);  // print as int for clarity
  Serial.print(F(" Payload: "));
  Serial.println(newValue);

  char sTopic_ch[24];
  snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d/status", deviceID, i);
  pubStatus(sTopic_ch, setStatus(newValue));
}

// MQTT reconnect function - non-blocking version
void reconnect() {
  static unsigned long lastAttempt = 0;
  static char attemptCount = 0;
  
  if (!enable_mqtt_reconnect) return;
  
  unsigned long now = millis();
  if (now - lastAttempt > 30000) { // Попытка каждые 30 сек
    if (attemptCount < 3 && !client.connected()) {
      Serial.print(F("Attempting MQTT connection..."));
      if (client.connect(deviceID, mqttuser, mqttpass)) {
        Serial.println(F("connected"));
        for (uint8_t i = 0; i < nWidgets; i++) {
          char topic_subscr[30];
          if ((pinmode[i] == 2) || (pinmode[i] == 3) || (pinmode[i] == 5)) {
            snprintf(topic_subscr, sizeof(topic_subscr), "%s/%d", deviceID, i);
            client.subscribe(topic_subscr);
          }
        }
        pubConfig();
        subscribe_loop = nWidgets;
        attemptCount = 0;
      } else {
        Serial.print(F("failed, rc="));
        Serial.println(client.state());
        attemptCount++;
      }
    }
    lastAttempt = now;
  }
}

void setup_IOTManager()  // that void is in setup() section in a main ino file
{
  client.setServer(mqttServerName, mqttport);
  client.setCallback(callback);
  client.setBufferSize(256);  // default, change if needed
  client.setKeepAlive(15);    // default, change if needed
}

void loop_IOTManager()  // that void is in loop section in a main ino file
{
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}

void pubClientOneSecEvent() {  //this event call each second, for reduce load on esp8266
  if (client.connected()) {
    if (onesec_255 > mqttspacing_oldtime + mqttspacing) {
      char sTopic_ch[20];
      for (uint8_t i = 0; i < nWidgets; i++) {
        float x = get_new_pin_value(i);
        stat[i] = (int)x;
        snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d/status", deviceID, i);
        pubStatus(sTopic_ch, setStatus(x));
      }
      mqttspacing_oldtime = onesec_255;
    }
  }
  client.loop();
}
#endif