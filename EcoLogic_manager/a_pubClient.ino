//This variables left in another file, so they are out of scope

uint8_t oldtime = 0;
uint8_t nWidgets = N_WIDGECTS;
const uint8_t nWidgetsArray = N_WIDGECTS;
short int stat[nWidgetsArray];
// uint8_t widget[nWidgetsArray]; // inputWidjet[0] = 'unknown';1 = 'toggle';2 = 'simple-btn';4 = 'range';4 = 'small-badge';5 = 'uint8_tt';
char descr[nWidgetsArray][10];
uint8_t id[nWidgetsArray];
uint8_t pin[nWidgetsArray];
short int defaultVal[nWidgetsArray];
uint8_t IrButtonID[nWidgetsArray];
float analogDivider = 1.0F;
short int analogSubtracter = 0;
unsigned int low_pwm[nWidgetsArray];
bool low_pwm_off = false;             // low_pwm
uint8_t pinmode[nWidgetsArray];       // inputPinmode[0] = "no pin";inputPinmode[1] = "in";inputPinmode[2] = "out";inputPinmode[3] = "pwm";inputPinmode[4] = "adc";inputPinmode[5] = "low_pwm";inputPinmode[6] = "IR";inputPinmode[7] = "датчик газа MQ7";

uint8_t subscribe_loop = 0;
uint8_t subscr_sec = 5; 
uint8_t mqttspacing_oldtime;



#if defined(USE_PUBSUBCLIENT)

char *setStatus(char *s)
{
  static char stat[40];
  sprintf(stat, "%s", s);
  return stat;
}

char *setStatus(int s)
{
  static char stat[40];
  sprintf(stat, "%d", s);
  return stat;
}

char *setStatus(float s)
{
  static char stat[40];
  sprintf(stat, "%.2f", s);
  return stat;
}

void pubStatus(char t[], char *payload)
{
  if (!client.connected())
    return;
  if (client.publish(t, payload))
  {
    Serial.println("publish:" + String(t) + ", value: " + String(payload));
  }
}

void pubConfig()// that is how I publish config, you dont need to adhere same structure
{
  for (uint8_t i = 0; i < nWidgets; i++)
  {
    char sTopic_ch[20];
    snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d", deviceID, i);
    pubStatus(sTopic_ch, setStatus(stat[i]));
  }
}

void callback(char *topic, byte *payload, uint8_t length) // callback for recieving messages from subsriptions
{
  char *lastSlash = strrchr(topic, '/');
  char i = 0;
  if (lastSlash != NULL && *(lastSlash + 1) != '\0') {
    int temp = atoi(lastSlash + 1); // parse the number after the last '/'
    i = (char)temp; // store as char to save memory
  }
  char payloadBuffer[length + 1];
  strncpy(payloadBuffer, (char *)payload, length);
  payloadBuffer[length] = '\0';
  char *end;
  int newValue = strtol(payloadBuffer, &end, 10);
  callback_socket(i, newValue);
  //pub status back

  Serial.print("callback:");
  Serial.print((int)i); // print as int for clarity
  Serial.print(" Payload: ");
  Serial.println(newValue);

  char sTopic_ch[24];
  snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d/status", deviceID, (int)i);
  pubStatus(sTopic_ch, setStatus(newValue));
}

void setup_IOTManager() // that void is in setup() section in a main ino file
{
  client.setServer(mqttServerName, mqttport);
  client.setCallback(callback);
}

void loop_IOTManager() // that void is in loop section in a main ino file
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      Serial.print("MQTT server: ");
      Serial.print(mqttServerName);
      Serial.print(" port ");
      Serial.println(mqttport);
      bool success = client.connect(deviceID, mqttuser, mqttpass);
      if (success)
      {
        server.send(200, "text/json", "{client:connected}");
        Serial.println("Connect to MQTT server: Success");
        pubConfig();
        subscribe_loop = 0;
      }
      else
      {
        Serial.print("Connect to MQTT server: FAIL");
      }
    }
  }
}

void pubClientOneSecEvent() { //this event call each second, for reduce load on esp8266
  if (client.connected())
  {
    if (onesec > oldtime + subscr_sec && subscribe_loop < nWidgets)// here I subscribe for a one message in a 1 period of subscribtion
    {
      char topic_subscr[30];
      if ((pinmode[subscribe_loop] == 2) || (pinmode[subscribe_loop] == 3) || (pinmode[subscribe_loop] == 5)) {
        snprintf(topic_subscr, sizeof(topic_subscr), "%s/%d", deviceID, subscribe_loop);
        if (client.subscribe(topic_subscr))
        {
          Serial.println("subscribe:" + String(topic_subscr));
          subscribe_loop++;
        }
        oldtime = onesec;
      } else {
        subscribe_loop++;
      }
    }

    if (onesec_255 > mqttspacing_oldtime + mqttspacing )
    {
      for (uint8_t i = 0; i < nWidgets; i++)
      {
        float x = get_new_pin_value(i);
        stat[i] = (int)x;
        char sTopic_ch[20];
        snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d/status", deviceID, i);
        pubStatus(sTopic_ch, setStatus(x));
      }
      mqttspacing_oldtime = onesec_255;
    }
  }
  client.loop();
}
#endif

