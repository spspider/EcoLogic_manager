#if defined(pubClient)

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

void pubConfig()
{
  for (char i = 0; i < nWidgets; i++)
  {
    char sTopic_ch[20];
    snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d", deviceID, i);
    pubStatus(sTopic_ch, setStatus(stat[i] ^ defaultVal[i]));
  }
}

void callback(char *topic, byte *payload, unsigned char length)
{
  char *lastSlash = strrchr(topic, '/');
  char i = lastSlash != NULL ? *(lastSlash + 1) : '0';
  char payloadBuffer[length + 1];
  strncpy(payloadBuffer, (char *)payload, length);
  payloadBuffer[length] = '\0';
  char *end;
  int newValue = strtol(payloadBuffer, &end, 10);
  callback_socket(i, newValue);
  //pub status back

  Serial.print("callback:");
  Serial.print(i);
  Serial.print(" Payload: ");
  Serial.println(newValue);

  char sTopic_ch[20];
  snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%c/status", deviceID, i);
  pubStatus(sTopic_ch, setStatus(newValue));
}

void setup_IOTManager()
{
  client.setServer(mqttServerName, mqttport);
  client.setCallback(callback);
}

void loop_IOTManager()
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

void pubClientOneSecEvent() {
  if (client.connected())
  {
    if (onesec > oldtime + subscr_sec && subscribe_loop < nWidgets)
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

