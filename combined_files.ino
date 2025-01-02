//This variables left in another file, so they are out of scope

unsigned long newtimePWM, oldtimePWM;
uint8_t oldtime = 0;
char nWidgets = 10;
const char nWidgetsArray = 10;
short int stat[nWidgetsArray];
//char widget[nWidgetsArray]; // inputWidjet[0] = 'unknown';1 = 'toggle';2 = 'simple-btn';4 = 'range';4 = 'small-badge';5 = 'chart';
char descr[nWidgetsArray][10];
char id[nWidgetsArray];
unsigned char pin[nWidgetsArray];
short int defaultVal[nWidgetsArray];
char IrButtonID[nWidgetsArray];
float analogDivider = 1.0F;
short int analogSubtracter = 0;
unsigned int low_pwm[nWidgetsArray];
bool low_pwm_off = false;             // low_pwm
unsigned char pinmode[nWidgetsArray]; // inputPinmode[0] = "no pin";inputPinmode[1] = "in";inputPinmode[2] = "out";inputPinmode[3] = "pwm";inputPinmode[4] = "adc";inputPinmode[5] = "low_pwm";inputPinmode[6] = "IR";inputPinmode[7] = "?????? ???? MQ7";

unsigned char subscribe_loop = 0;
uint8_t subscr_sec = 5; 
uint8_t mqttspacing_oldtime;

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

void pubConfig()// that is how I publish config, you dont need to adhere same structure
{
  for (char i = 0; i < nWidgets; i++)
  {
    char sTopic_ch[20];
    snprintf(sTopic_ch, sizeof(sTopic_ch), "%s/%d", deviceID, i);
    pubStatus(sTopic_ch, setStatus(stat[i] ^ defaultVal[i]));
  }
}

void callback(char *topic, byte *payload, unsigned char length)// callback for recieving messages from subsriptions
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




#if defined(USE_DNS_SERVER)
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif

#define NAPT 1000
#define NAPT_PORT 10

#define CONSOLE Serial
#define _PRINTF(a, ...) printf_P(PSTR(a), ##__VA_ARGS__)
#define CONSOLE_PRINTF CONSOLE._PRINTF
#define DEBUG_PRINTF CONSOLE_PRINTF

struct ip_addr *IPaddress;
uint32 uintaddress;

// ESP8266WebServer server(80); - defined in previous ino file

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

/** Should I connect to WLAN asap? */
bool connect = false;

bool staReady = false; // Don't connect right away

uint8_t lastConnectTry = 0;
uint8_t status = WL_IDLE_STATUS;
uint8_t FreeWIFIid[5];
bool freeWIFIConnected = false;

void connect_as_AccessPoint()
{
  delay(1000);
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.disconnect();
  WiFi.persistent(false); // w/o this a flash write occurs at every boot
  WiFi.mode(WIFI_OFF);    // Prevent use of SDK stored credentials
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid + WiFi.macAddress(), softAP_password);
#if defined(USE_DNS_SERVER)
  dnsServer.setTTL(0);
  dnsServer.start(IANA_DNS_PORT, "*", apIP);
#endif
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}
void captive_setup()
{ // starting void

  Captive_server_init();
  if (load_ssid_pass())
  {
    connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
    if (connect)
    {
      try_MQTT_access = true;
    }
    else
    {
      try_MQTT_access = false;
      connect_as_AccessPoint();
    }
  }
  else
  {
    try_MQTT_access = false;
    connect_as_AccessPoint();
  }

  server_init();
  server.begin();
}

void connectWifi(char ssid_that[32], char password_that[32])
{
  WiFi.disconnect();
  status = WL_IDLE_STATUS; // ???????? ??????, ??? ???????????
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println("Connecting as wifi client...");
  Serial.println(ssid_that);
  Serial.println(password_that);
  Serial.print("Wifi status:");
  Serial.println(WiFi.status());
  delay(100);
  WiFi.begin(ssid_that, strlen(password_that) > 0 ? password_that : nullptr);
  uint8_t attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 50)
  {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
    connect_as_AccessPoint();
}
// void try_connect_free() {
//   Serial.println("!!!scan for FREE WIFI!!!");
//   uint8_t n = WiFi.scanNetworks();//????????? Wifi ????
//   if (n > 0) {//???? ????? ?????? ?????
//     uint8_t number = 0;
//     for (uint8_t i = 0; i < n; i++) {
//       if (WiFi.encryptionType(i) == ENC_TYPE_NONE)  {//???? ??? ???? ????????
//         FreeWIFIid[number] = i;
//         number++;
//       }
//     }
//     if (number > 0) {
//       uint8_t ssid_number = random(number - 1);
//       //Serial.println("random:" + String(ssid_number));
//       WiFi.SSID(FreeWIFIid[ssid_number]).toCharArray(ssid, sizeof(ssid) - 1);
//       strncpy( password, "\0", sizeof(password) - 1);
//       lastConnectTry = onesec;
//       connect = false;
//       connectWifi(ssid, password);
//       freeWIFIConnected = true;
//     }
//     else {
//       Serial.println("no Free Wifi");
//     }
//   }
// }
void relayRouter()
{
  if (router != 255)
  {
    pinMode(router, OUTPUT);
    digitalWrite(router, digitalRead(router) ^ 1);
    Serial.println("Router relay state:" + String(digitalRead(router), DEC));
  }
}
void captive_loop()
{
  if (connect)
  {
    status = WL_IDLE_STATUS;
    connect = false;
    connectWifi(ssid, password);
    lastConnectTry = onesec;
  }
  if (try_MQTT_access)
  {
#if defined(pubClient)
    if (IOT_Manager_loop)
    {
      loop_IOTManager();
    }
#endif
  }
  // if ((!try_MQTT_access) && (WiFi.status() == WL_CONNECTED) && (WiFi.getMode() == WIFI_STA)) {
  //   if (onesec > no_internet_timer + 300 ) {//????????? ??????????? ??????30 ???
  //     try_MQTT_access = true; //??????? ?????????? MQTT
  //     Serial.println("Connect again");
  //     no_internet_timer = onesec;
  //   }
  // }
  {

    uint8_t s = WiFi.status();

    if (onesec > lastConnectTry + 60)
    {
      if ((WiFi.status() == WL_DISCONNECTED) && ((wifi_softap_get_station_num() != 0)))
      {
        WiFi.disconnect();
        Serial.println("Wrong connection");
        lastConnectTry = onesec;
        connect_as_AccessPoint();
        relayRouter();
        return;
      }
      else if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() == 0) && wifi_scan && staReady)
      {
        Serial.println("Connect Credentials");
        if (load_ssid_pass())
        {
          WiFi.disconnect();
          connect = true;
        }
#if defined(USE_DNS_SERVER)
        dnsServer.setTTL(600); // 10 minutes
        dnsServer.enableForwarder(myHostname, WiFi.dnsIP(0));
        if (dnsServer.isDNSSet())
        {
          CONSOLE_PRINTF("  Forward other lookups to DNS: %s\r\n", dnsServer.getDNS().toString().c_str());
        }
#endif
        return;
      }
      else if (!internet && geo_enable)
      {
        Serial.println("Status WIFI: " + String(WiFi.status()));
      }
      else if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() != 0))
      {
        Serial.print("stations connected as AP:");
        Serial.println(wifi_softap_get_station_num());
        return;
      }

      lastConnectTry = onesec;
    }
    if (status != s)
    { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED)
      {

        try_MQTT_access = true; // ????? ?????????? ???????????? ? ?????????
        /* Just connected to WLAN */
        Serial.println("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        server.send(200, "text/plain", toStringIp(WiFi.localIP()));
        save_wifiList(ssid, password);
        setup_ota();
        MDNS.addService("http", "tcp", ipport);
        if (geo_enable)
          sendLocationData();
#if defined(timerAlarm)
        setup_alarm();
#endif
#ifdef use_telegram
        setup_telegram();
#endif
      }
      else if (s == WL_NO_SSID_AVAIL)
      {
        internet = false;
        try_MQTT_access = false;
        WiFi.disconnect();
      }
      else if (s == WL_CONNECTED)
        MDNS.update();
#if defined(timerAlarm)
      setup_alarm();
#endif
    }
  }
#if defined(USE_DNS_SERVER)
  dnsServer.processNextRequest();
#endif
  server.handleClient();
}
void sendLocationData()
{
  String pos = getHttp("api.2ip.ua/geo.json?ip=");
  if (pos != "fail")
  {
    internet = true;
  }
  else
  {
    internet = false;
    Serial.println("Internet connection failed");
  }
  saveCommonFiletoJson("ip_gps", pos, 1);
  sendEmail(pos);
}



#if defined(as5600)
void setup_compass()
{
  ///////////////compass/////////////

  Wire.begin();
  compass.init();
  compass.setSamplingRate(50);
  ///////////////////////////////
  //pinMode(pin[i], OUTPUT);
  compass.reset();
  Serial.println("Compass Ready");
}

  float get_fuel_value() {

  float compass_read = 0.0F;
  compass_read = compass.readHeading();
  if (compass_read == 0) {
  compass.reset();
  } else {
   //compass_read = (compass_read * 1.0F / analogDivider * 1.0F ) - analogSubtracter; //adc pin:A0;
  // compass_read = ((compass_read  - analogSubtracter) / analogDivider);

  }
  return compass_read ;
  }
#endif
  float setFUllFuel(uint8_t full_fuel)
  {
    // high_compass_fuel = compass.readHeading();
#if defined(as5600)
    analogDivider = (encoder.getAngle() - analogSubtracter) / (full_fuel * 1.00F);
#endif
    savePinSetup();

    return analogDivider;
  }
  float setZeroFuel()
  {
#if defined(as5600)
    analogSubtracter = encoder.getAngle();
#endif
    savePinSetup();

    return analogSubtracter;
  }

  bool savePinSetup()
  {
    File buffer_read = SPIFFS.open("/pin_setup.txt", "r");
    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, buffer_read);

    if (error)
    {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return false;
    }

    JsonObject json = jsonDocument.as<JsonObject>();
    json["aDiv"] = analogDivider;
    json["aSusbt"] = analogSubtracter;

    String buffer;
    serializeJson(json, buffer);
    // Serial.println(buffer);
    saveCommonFiletoJson("pin_setup", buffer, 1);

    return true;
  }





float get_new_pin_value(unsigned char i)
{

  float that_stat = (float)stat[i];

  if (pin[i] == 255)
  {
    // return that_stat;
  }
  if (pinmode[i] == 1)
  { // in
    that_stat = digitalRead(pin[i]) ^ defaultVal[i];
    stat[i] = that_stat;
    return that_stat;
  }
  if ((pinmode[i] == 2))
  { // out
    that_stat = digitalRead(pin[i]);
    return that_stat;
  }
  if ((pinmode[i] == 4) || (pinmode[i] == 3))
  { // pwm, adc
    stat[i] = (int)that_stat;
    return that_stat;
  }
  if (pinmode[i] == 6)
  { // dht Temp
#if defined(dht)
    if (!license)
      return 127;
    float temperature = dht.getTemperature();
    if (!isnan(temperature))
    {
      that_stat = temperature;
      stat[i] = that_stat;
    }
    else
    {
      that_stat = stat[i]; // Use a default value or handle the NaN case accordingly
    }
#endif
    return that_stat;
  }
  if (pinmode[i] == 7)
  {
    if (!license)
      return 127;
    that_stat = (float)low_pwm_off;
    return that_stat;
  }
  if (pinmode[i] == 8)
  { // dht Humidity
#if defined(dht)
    if (!license)
      return 127;
    float humidity = dht.getHumidity();
    if (!isnan(humidity))
    {
      that_stat = humidity;
      stat[i] = that_stat;
    }
    else
    {
      that_stat = stat[i]; // Use a default value or handle the NaN case accordingly
    }
#endif
    return that_stat;
  }

  if (pinmode[i] == 9)
  { // remote
    if (!license)
      return 127;
    that_stat = getHttp(String(descr[i])).toFloat();
    return that_stat;
  }
  if (pinmode[i] == 11)
  {
    if (!license)
      return 127;
#if defined(as5600)
    that_stat = (encoder.getAngle() - analogSubtracter) / analogDivider * 1.0F;
#endif
    return that_stat;
  }
  if (pinmode[i] == 12)
  { // MAC ADRESS
    // that_stat = stat[i] ^ 1;
    return that_stat;
  }
  if (pinmode[i] == 13)
  { // EncA
    that_stat = no_internet_timer;
    return that_stat;
  }
  if (pinmode[i] == 14)
  { // EncB
    // that_stat = stat[i] ^ 1;
    return that_stat;
  }
  if (pinmode[i] == 15)
  { // ads
#if defined(ads1115)
    that_stat = (ads.readADC_SingleEnded(defaultVal[i]));
    return that_stat;
#endif
  }
  if (pinmode[i] == 16)
  { // ds18b20
    // Serial.print ("stat[i]", stat[i]);
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

  if (pinmode[i] == 10)
  { // PowerMeter ?????? ???? ?????????, ????? ?????? jump to case label
    if (!license)
      return 127;
      // double Irms ;
#if defined(emon)
    that_stat = (float)emon1.calcIrms(1480); // Calculate Irms only
    that_stat = (that_stat * 1.0F / analogDivider) + analogSubtracter;
#endif
    return that_stat;
  }
  if ((isnan(that_stat)) || (isinf(that_stat)))
  {
    that_stat = stat[i]; // 0
    return that_stat;
  }

  return -123.12;
}

void makeAres_sim(String json)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, json);

  if (error)
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Failed to parse JSON");
    return;
  }

  JsonObject root = jsonDocument.as<JsonObject>();
  char that_pin;
  float that_val = 0.0F;
  char control = 255;
  char that_stat = 255;
  String String_value = "";

  root.containsKey("pin") ? that_pin = root["pin"] : that_pin = 255;
  root.containsKey("stat") ? that_stat = root["stat"] : that_stat = 255;
  root.containsKey("val") ? that_val = root["val"] : that_val = -1;
  root.containsKey("C") ? control = root["C"] : control = 255;
  root.containsKey("st") ? String_value = root["st"].as<String>() : String_value = "";

  switch (control)
  {
  case 255:
  {
    char i = 255;
    for (char i1 = 0; i1 < nWidgets; i1++)
    {
      if (that_pin == pin[i1])
      {
        i = i1;
        break; // Fix for the loop termination condition
      }
    }

    if (that_stat != 255)
    {
      if (root.containsKey("val"))
      {
        stat[that_stat] = that_val;
      }
      else
      {
        that_val = get_new_pin_value(that_stat); // ?????? ??????
      }
    }

    if (i != 255)
    {
      if ((pinmode[i] == 2) || (pinmode[i] == 1))
      { // out, in
        stat[i] = static_cast<int>(that_val) ^ defaultVal[i];
        // send_IR(i);
        digitalWrite(that_pin, stat[i]);
      }
      else if (pinmode[i] == 3)
      { // pwm
        analogWrite(that_pin, that_val);
      }
    }

    // pubStatusFULLAJAX_String(false);
    that_val = round(that_val * 200) / 200;
    server.send(200, "text / json", String(that_val, DEC));
    break;
  }
  case 1: // PLUS Control
  {
    //        bySignalPWM[that_pin][that_stat] = that_val;
    //        server.send(200, "text / json",  saveConditiontoJson(that_pin));
    break;
  }
  case 2: // IR
  {
#if defined(ir_code)
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



/** Handle root or redirect to captive portal */
void sendMyheader() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
}
String sendHead() {
  String Page_head;
  Page_head += F(
                 "<html><head>"
                 "<meta http-equiv='Content-type' content='text/html; charset=utf-8'>"
                 "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                 "</head><body>"
               );
  return Page_head;
}
void handleRoot() {
  //  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
  //    return;
  //  }
  sendMyheader();

  String Page = sendHead();
  Page += F("<h1>DEV</h1>");
  /*
    if (server.client().localIP() == apIP) {
    Page += String(F("<p>")) + softAP_ssid + F("</p>");
    } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
    }
  */
  Page += String(F(
                   "<style>"
                   "button {"
                   "   padding: 10px 20px; "
                   "   font-size: 16px;"
                   "   width: 200px;"
                   "   margin: 5px;"
                   "   text-align: center;"
                   "   display: inline-block;"
                   "   border: none;"
                   "   cursor: pointer;"
                   "   background-color: #008CBA; "
                   "   color: white;"
                   "   border-radius: 5px;"
                   "}"
                   "a {"
                   "   text-decoration: none;"
                   "}"
                   "</style>"
                   "<p><br><a href='/home.htm'><button>home page</button></a></p>"
                   "<p><br><a href='/wifi'><button>Wifi</button></a></p>"
                   "<p><br><a href='/other_setup'><button>other setup</button></a></p>"
                   "<p><br><a href='/pin_setup'><button>pin setup</button></a></p>"
                   "<p><br><a href='/IR_setup'><button>IR</button></a></p>"
                   "<p><br><a href='/condition'><button>condition</button></a></p>"
                   "<p><br><a href='/function?json={reboot:1}'><button>reboot</button></a></p>"
                 ));



  server.send(200, "text / html", Page);
}
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    //server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}
void handleWifilist() {
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  JsonObject json = jsonDocument.to<JsonObject>();

  json["ssid"] = ssid;
  json["WiFilocalIP"] = toStringIp(WiFi.localIP());
  json["softAP"] = softAP_ssid;
  json["WiFisoftAPIP"] = toStringIp(WiFi.softAPIP());

  JsonArray scan_array = json.createNestedArray("scan");
  JsonArray enc_array = json.createNestedArray("enc");
  JsonArray RSSI_array = json.createNestedArray("RSSI");

  int n = WiFi.scanNetworks();
  if (n > 0) {
    json["n"] = n;
    for (int i = 0; i < n; i++) {
      scan_array.add(WiFi.SSID(i));
      enc_array.add((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "Free" : "");
      RSSI_array.add(WiFi.RSSI(i));
    }
  }

  String buffer;
  serializeJson(json, buffer);
  server.send(200, "text/json", buffer);
}

void handleWifi() {
  String Page = sendHead();
  Page += F(
            "<h1>Wifi config</h1>"
            "<script>function populateSSID(ssid) {document.getElementById(\"network\").value = ssid;}</script>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      //      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
      Page += String(F("\r\n<tr><td><a href='#' onclick='populateSSID(\"")) + WiFi.SSID(i) + String(F("\")'>")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</a></td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' id='network' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);

  saveCredentials();

  connectWifi(ssid, password);
}

void save_wifiList(const char *s, const char *p)
{
  File WifiList = SPIFFS.open("/wifilist.txt", "r");

  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, WifiList);

  if (error) {
    Serial.print(F("deserializeJson() save_wifiList failed with code "));
    Serial.println(error.c_str());
    //    return;
  }

  JsonArray name_array = jsonDocument.createNestedArray("name");
  JsonArray pass_array = jsonDocument.createNestedArray("pass");

  char num = name_array.size();
  bool ssid_not_found = true;
  bool write_array = false;

  if (num == 0) {
    jsonDocument["num"] = 1;
  }
  for (unsigned char i = 0; i < num; i++) {
    char nameWifi[20];
    char passWifi[20];

    strlcpy(nameWifi, name_array[i], sizeof(nameWifi));
    strlcpy(passWifi, pass_array[i], sizeof(passWifi));

    if ((strcmp(nameWifi, ssid) == 0) && (strcmp(passWifi, password) != 0)) {
      Serial.println("update ssid password");
      // Need update
      name_array.add(name_array[i]);
      pass_array.add(password);
      write_array = true;
    } else {
      Serial.println("write ssid as previous");
      name_array.add(name_array[i]);
      pass_array.add(pass_array[i]);
    }

    if (strcmp(nameWifi, ssid) != 0) {
      ssid_not_found = false;
    }
  }

  if (ssid_not_found) {
    Serial.println("add new ssid");
    name_array.add(ssid);
    pass_array.add(password);
    write_array = true;
  }

  if (write_array) {
    String buffer;
    serializeJson(jsonDocument, buffer);
    saveCommonFiletoJson("wifilist", buffer, 1);
  }

  saveCredentials();
}

/*
  String *read_wifiList(unsigned char index) {
  String WifiList = readCommonFiletoJson("wifilist");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(WifiList);
  if (!rootjs.success()) {
    Serial.println("parseObject() save_wifiList failed");
    //return;
  }
  //char num = rootjs.containsKey("num") ? rootjs["num"] : 10;
  static String wifi[2];
  wifi[0] = rootjs["name"][index].as<String>();
  wifi[1] = rootjs["pass"][index].as<String>();
  return wifi;
  }
*/
bool load_ssid_pass() {
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");

  if (n > 0) {
    //        String WifiList = readCommonFiletoJson("wifilist");

    File WifiList = SPIFFS.open("/wifilist.txt", "r");

    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, WifiList);
    if (error) {
      Serial.print(F("load_ssid_pass deserializeJson() failed load_ssid_pass with code "));
      Serial.println(error.c_str());
      return false;
    }

    for (int i = 0; i < 10; i++) {
      String nameWifi = jsonDocument["name"][i];
      String passWifi = jsonDocument["pass"][i];

      for (int in = 0; in < n; in++) {
        if (WiFi.SSID(in) == nameWifi) {
          strlcpy(ssid, nameWifi.c_str(), sizeof(ssid));
          strlcpy(password, passWifi.c_str(), sizeof(password));
          Serial.println(ssid);
          Serial.println(password);
          return true; // Found and set, exit the function
        }
      }
    }
  }

  return false; // No match found
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI : ");
  message += server.uri();
  message += F("\nMethod : ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments : ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(" : ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache - Control", "no - cache, no - store, must - revalidate");
  server.sendHeader("Pragma", "no - cache");
  server.sendHeader("Expires", " - 1");
  server.send(404, "text / plain", message);
}

/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}





/** Load WLAN credentials from EEPROM */




void loadCredentials() {
  /*
    EEPROM.begin(512);
    EEPROM.get(0, ssid);
    EEPROM.get(0 + sizeof(ssid), password);
    char ok[2 + 1];
    EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);

    EEPROM.end();
    if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    }
    Serial.println("Recovered credentials:");
    Serial.println(ssid);
    //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
    Serial.println(password);
  */
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}
void saveEEPROM(int adress, char value[5] ) {
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
}
void saveEEPROM_char(int adress, char value ) {
  char sep = 8;
  EEPROM.begin(512);
  EEPROM.put(adress * sizeof(value) + sep, value);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Saved EEPROM:");
  Serial.println(String(getEEPROM_char(adress), DEC));
}
char* getEEPROM(int adress) {
  char buffer[5];
  char sep = 100;
  EEPROM.begin(512);
  EEPROM.get(adress * sizeof(buffer) + sep, buffer);
  EEPROM.end();
  Serial.println("Recovered credentials:");
  Serial.println(buffer);
  return buffer;
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}
char getEEPROM_char(int adress) {
  char buffer;
  char sep = 8;
  EEPROM.begin(512);
  EEPROM.get(adress * sizeof(buffer) + sep, buffer);
  EEPROM.end();
  Serial.println("Recovered EEPROM:");
  Serial.println(String(buffer, DEC));
  return buffer;
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}
/*
  void saveCredentialsAP() {
  EEPROM.begin(512);
  EEPROM.put(70, softAP_ssid);
  EEPROM.put(70 + sizeof(softAP_ssid), softAP_password);
  char ok[2 + 1] = "OK";
  EEPROM.put(70 + sizeof(softAP_ssid) + sizeof(softAP_password), ok);
  EEPROM.commit();
  EEPROM.end();
  }
  void  loadCredentialsAP() {
  EEPROM.begin(512);
  EEPROM.get(70, softAP_ssid);
  EEPROM.get(70 + sizeof(softAP_ssid), softAP_password);
  char ok[2 + 1];
  EEPROM.get(70 + sizeof(softAP_ssid) + sizeof(softAP_password), ok);

  EEPROM.end();
  if (String(ok) != String("OK")) {
  //    *softAP_ssid = "ESP_ap_dev_001";
  //    *softAP_password= "12345678";
  }
  Serial.println("Recovered credentials:");
  Serial.println(softAP_ssid);
  //Serial.println(strlen(password) > 0 ? "********" : "<no password>");
  Serial.println(softAP_password);
  }
*/
/*
  void saveEEPROM(String topic_pub, int new_stat) {
  int that_widget = -1;
  for (int i = 0; i < nWidgets; i++) {
    //Serial.println("topic_pub:"+topic_pub+" sTopic:"+sTopic[i]);
    if (String(topic_pub) == String(sTopic[i])) {

      that_widget = i;
      //if (defaultVal[i]==new_stat){that_widget=-1;}
      break;
    }
  }
  if (that_widget != -1) {
    //if (defaultVal[that_widget]!=new_stat){
    int adress = that_widget * 5 + addr_widgets_begins;
    //Serial.println("saveEEPROM" + sTopic[that_widget] + " status:" + new_stat + " addr:" + adress);
    //String value
    EEPROM.begin(512);
    EEPROM.put(adress, new_stat);
    EEPROM.commit();
    EEPROM.end();
    //delay(100);
    /// }

  }
  }
*/
/////////////////////////////////////////////////////////////////////////////////////////
/*
  unsigned int *getSPIFFS_JSON_VALUE(int nWidgets) {
  unsigned int value_back[nWidgets];
  for (int i = 0; i < nWidgets; i++) {
    value_back[i] = defaultVal[i];
  }
  String buffer = readCommonFiletoJson("config");
  /*
    File configFile = SPIFFS.open("/config.txt", "r");
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
*/
/*
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buffer);
  if (!json.success()) {
  Serial.println("Failed to parse config file");
  return value_back;
  }

  //unsigned int value_back[nWidgets];
  for (int i = 0; i < nWidgets; i++) {

  const char* value_char = json[sTopic[i]];
  StaticJsonBuffer<200> jsonBufferParseStatus;
  JsonObject& jsonParseSt = jsonBufferParseStatus.parseObject(value_char);//?????? ???????????

  const char* status_json = jsonParseSt["status"];
  String status_str = String((char*)status_json);
  unsigned int status_int = status_str.toInt();
  if (jsonParseSt.success()) {
    value_back[i] = status_int;
  } else {
    Serial.println("failed restore value:" + String(i));
    value_back[i] = defaultVal[i];
  }

  Serial.print("stored_value for topic:" + sTopic[i] + " is:");
  Serial.println(value_back[i]);
  //Serial.println(status_int);
  }

  return   value_back;

  }
*/
/////////////////////////////////////////////////////////////////////////////////////
/*
  bool saveSPIFFS() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  for (int i = 0; i < nWidgets; i++) {
    String id_t = sTopic[i];
    json[id_t] = stat[i];
  }

  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  //Serial.println("Saved:"+id_t+"value"+defaultVal[i]);
  json.printTo(configFile);
  json.printTo(Serial);
  return true;
  }
*/
void save_stat_void() {
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  JsonArray stat_json = jsonDocument.createNestedArray("stat");

  for (uint8_t i = 0; i < nWidgets; i++) {
    stat_json.add(stat[i]);
  }

  String buffer;
  serializeJson(jsonDocument, buffer);
  saveCommonFiletoJson("stat", buffer, 1);
}

bool saveSPIFFS_jsonArray(int *stat_arr) {
  File buffer_read = SPIFFS.open("/pin_setup.txt", "r"); 
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, buffer_read);
  
  if (error) {
    Serial.print(F("deserializeJson() failed with pin_setup code "));
    Serial.println(error.c_str());
    return false;
  }

  JsonArray defaultVal_json = jsonDocument.createNestedArray("stat");
  
  for (int i = 0; i < nWidgets; i++) {
    defaultVal_json.add(stat_arr[i]);
  }

  String buffer;
  serializeJson(jsonDocument, buffer);
  saveCommonFiletoJson("pin_setup", buffer, 1);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/*
  unsigned int getEEPROM(int id) {
  int value_back = 0;
  int adress = id * 5 + addr_widgets_begins;
  EEPROM.begin(512);
  delay(100);
  EEPROM.get(adress, value_back);
  delay(500);
  Serial.print("adress:");
  Serial.print(adress);
  Serial.print(" id:");
  Serial.print(id);
  Serial.println(" RestoredValue:" + value_back);
  delay(500);
  EEPROM.end();

  return value_back;
  }
*/



bool loadConfig(File jsonConfig)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonConfig);

  if (error)
  {
    Serial.println("Failed to parse JSON! loadConfig");
    return false;
  }

  if (jsonDocument.containsKey("softAP_ssid"))
  {
    // Do something with softAP_ssid if needed
  }

  if (jsonDocument.containsKey("ssid"))
  {
    strcpy(ssid, jsonDocument["ssid"]);
    strcpy(password, jsonDocument["password"]);
  }

  if (jsonDocument.containsKey("deviceID"))
  {
    strcpy(softAP_ssid, jsonDocument["deviceID"]);
    strcpy(deviceID, jsonDocument["deviceID"]);
  }

#if defined(pubClient)
  IOT_Manager_loop = jsonDocument["iot_enable"];
  if (IOT_Manager_loop)
  {
    client.disconnect();
  }

  strcpy(mqttServerName, jsonDocument["mqttServerName"]);
  jsonDocument.containsKey("mqttport") ? mqttport = jsonDocument["mqttport"] : mqttport = 1883;

  strcpy(mqttuser, jsonDocument["mqttuser"]);
  strcpy(mqttpass, jsonDocument["mqttpass"]);
  jsonDocument.containsKey("mqttspacing") ? mqttspacing = jsonDocument["mqttspacing"] : mqttspacing = 60;
#endif

  jsonDocument.containsKey("geo_enable") ? geo_enable = jsonDocument["geo_enable"] : geo_enable = 0;
  jsonDocument.containsKey("wifi_scan") ? wifi_scan = jsonDocument["wifi_scan"] : wifi_scan = 1;

#if defined(ws433)
  jsonDocument.containsKey("loop_433") ? loop_433 = jsonDocument["loop_433"] : loop_433 = 0;
#endif

  jsonDocument.containsKey("ws8211_loop") ? ws8211_loop = jsonDocument["ws8211_loop"] : ws8211_loop = 0;
  jsonDocument.containsKey("save_stat") ? save_stat = jsonDocument["save_stat"] : save_stat = 0;
  // jsonDocument.containsKey("PWM_frequency") ? PWM_frequency = jsonDocument["PWM_frequency"] : PWM_frequency = 1;
  // unsigned int freq = PWM_frequency; // from 100Hz to 40000
  // analogWriteFreq(freq);             // frequency for PWM
  // analogWriteRange(1023);
  jsonDocument.containsKey("IR_recieve") ? IR_recieve = jsonDocument["IR_recieve"] : IR_recieve = 0;

  // telegram
#ifdef use_telegram
  if (jsonDocument.containsKey("BOTtoken"))
  {
    BOTtoken = jsonDocument["BOTtoken"].as<String>();
  }

#endif

  //  String jsonConfig_string = readCommonFiletoJson("pin_setup");
  if (updatepinsetup(SPIFFS.open("/pin_setup.txt", "r")))
  {
    Serial.println("Widgets Loaded");
  }
#if defined(pubClient)
  setup_IOTManager();
#endif
  return true;
}

void Setup_pinmode(bool stat_loaded)
{
  for (uint8_t i = 0; i < nWidgets; i++)
  {

    stat[i] = stat_loaded ? stat[i] : defaultVal[i];
    if (pin[i] != 255)
    {
      if (pinmode[i] == 1)
      { // in
        defaultVal[i] == 0 ? pinMode(pin[i], INPUT_PULLUP) : pinMode(pin[i], INPUT);
        stat[i] = (digitalRead(pin[i] ^ defaultVal[i]));
      }
      if ((pinmode[i] == 2))
      { // out
        pinMode(pin[i], OUTPUT);
        digitalWrite(pin[i], stat[i]);
      }
      if ((pinmode[i] == 3) || (pinmode[i] == 7))
      { // pwm,MQ7
        pinMode(pin[i], OUTPUT);
        analogWrite(pin[i], stat[i]); // PWM
      }
      // if (pinmode[i] == 5)
      // { // low_pwm
      //   pinMode(pin[i], OUTPUT);
      //   low_pwm[i] = stat[i];
      //   digitalWrite(pin[i], 1); // ????? - ?????????
      //   Serial.println("set low_pwm:" + String(pin[i], DEC) + "i:" + String(i, DEC) + "stat:" + String(stat[i], DEC));
      // }
      if (pinmode[i] == 4)
      {
        stat[i] = (analogRead(17) * 1.0F - analogSubtracter) / analogDivider; // adc pin:A0//
      }
      if ((pinmode[i] == 6) || (pinmode[i] == 8))
      { // dht temp
#if defined(dht)
        dht.setup(pin[i]); // data pin
        Serial.println("DHT:" + String(pin[i], DEC));
#endif
      }
      if (pinmode[i] == 10)
      { // powerMeter
        // pinMode(pin[i], OUTPUT);
#if defined(emon)
        emon1.current(17, PowerCorrection); // PowerCorrection=111.1
#endif
      }
      if (pinmode[i] == 15)
      { // ads
#if defined(ads1115)
        ads.begin();
#endif
      }
    }
  }
}

String readCommonFiletoJson(String file)
{
  File configFile = SPIFFS.open("/" + file + ".txt", "r");
  if (!configFile)
  {
    // ???? ???? ?? ??????
    Serial.println("Failed to open " + file + ".txt");
    configFile.close();
    return "";
  }
  // ????????? ?????? ?????, ????? ???????????? ???? ???????? ?????? 1024 ?????
  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
  }
  String jsonConfig = configFile.readString();
  Serial.print("file:");
  Serial.print(file);
  Serial.print(" ");
  Serial.println(jsonConfig);
  configFile.close();
  return jsonConfig;
}
bool saveCommonFiletoJson(String pagename, String json, boolean write_add)
{
  // w-??????????, ?  - ??????????
  // char* pagename_ch;
  // pagename.toCharArray(pagename_ch,sizeof(pagename_ch));
  //   char *write_add_char = (write_add == true) ? 'w' : 'a';
  File configFile;
  if (write_add == 1)
  {
    // char* openString;
    // strcat(openString,"/");
    configFile = SPIFFS.open("/" + pagename + ".txt", "w");
  }
  else if (write_add == 0)
  {
    configFile = SPIFFS.open("/" + pagename + ".txt", "a");
  }
  if (!configFile)
  {
    Serial.println("Failed to open " + pagename + ".txt for writing");
    return false;
  }
  Serial.println("#############################SAVE: " + String(write_add, DEC));
  Serial.println(json);
  configFile.print(json);
  configFile.close();
  return true;
}

/////////////////////CONDITION////////////////////////////////////////////////////
bool SaveCondition(String json)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, json);

  if (error)
  {
    Serial.println("Failed to parse JSON!");
    return false;
  }

  unsigned int NumberID = jsonDocument["ID"];
  Serial.println("NimberId:" + String(NumberID));

  String NameFile = "Condition" + String(NumberID);
  saveCommonFiletoJson(NameFile, json, 1);
  // load_Current_condition(NumberID); // ????? ?? ????????? ? ?????????? ??? ???????
  return true;
}

//////////////////////////////////////////////////////////////////////////////

bool updatepinsetup(File jsonrecieve)
{
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, jsonrecieve);

  if (error)
  {
    Serial.println("Failed to parse JSON!");
    return false;
  }

  unsigned char numberChosed = jsonDocument["numberChosed"];
  if (numberChosed == 0)
  {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!FAIL!! numberChosed = 0");
    return false;
  }

  if (numberChosed > nWidgetsArray)
  {
    numberChosed = nWidgetsArray;
  }

  nWidgets = numberChosed;

#if defined(ws433)
  w433rcv = jsonDocument.containsKey("w433") ? jsonDocument["w433"] : 255;
  w433send = jsonDocument.containsKey("w433send") ? jsonDocument["w433send"] : 255;
#endif

  for (uint8_t i = 0; i < numberChosed; i++)
  {
    pinmode[i] = jsonDocument["pinmode"][i];
    pin[i] = jsonDocument["pin"][i];
    defaultVal[i] = jsonDocument["defaultVal"][i];
    IrButtonID[i] = jsonDocument["IrBtnId"][i];
    id[i] = i;

    strncpy(descr[i], jsonDocument["descr"][i], sizeof(descr[i]) - 1);
  }

  analogDivider = jsonDocument.containsKey("aDiv") ? jsonDocument["aDiv"] : 1.0F;
  analogSubtracter = jsonDocument.containsKey("aSusbt") ? jsonDocument["aSusbt"] : 0;
  pwm_delay_long = jsonDocument.containsKey("PWM_interval") ? jsonDocument["PWM_interval"] : 60;

#if defined(emon)
  PowerCorrection = jsonDocument.containsKey("PCorr") ? jsonDocument["PCorr"] : 111.1;
#endif

  router = jsonDocument.containsKey("router") ? jsonDocument["router"] : 255;

  Setup_pinmode(load_stat());

  return true;
}

bool load_stat()
{
  if (save_stat == false)
  {
    return false;
  }

  DynamicJsonDocument jsonDocument_stat(2048); // Adjust the capacity as needed
  File stat1 = SPIFFS.open("/stat.txt", "r");

  DeserializationError error = deserializeJson(jsonDocument_stat, stat1);
  if (error)
  {
    Serial.println("PARSE FAIL!!");
    //     for (char i = 0; i < nWidgets; i++) {
    //       stat[i] = 0;
    //     }
    return false;
  }

  for (char i = 0; i < nWidgets; i++)
  {
    short int stat_js = jsonDocument_stat["stat"][i];
    if (stat_js)
    {
      stat[i] = stat_js;
      // callback_socket(i, stat_js);
    }
    // Serial.println(stat_js);
  }

  return true;
}

void callback_from_stat()
{
  for (char i = 0; i < nWidgets; i++)
  {
    callback_socket(i, stat[i]);
    // Serial.println(stat_js);
  }
}




//----------------------------------------defines-------------------------------//
//  #define ws2811_include// ???????????? ??? ws2811
// #define will_use_serial
// #define timerAlarm
//  #define use_telegram

#define ds18b20
#define USE_DNS_SERVER
// #define pubClient
// #define ir_code
// #define as
// #define wakeOnLan
// #define dht
//  #define ads1115
//  #define emon
//  #define ws433
//------------------------------------------------------------------------------//

// #include <Adafruit_GFX.h>
// #include <gfxfont.h>

// -----------------------DEFINING PINS----------------------------------
#define ONE_WIRE_BUS 0 // D3 pin
#define RECV_PIN 14 // IR recieve
#define SEND_PIN 15 // IR send

// #include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h> //iotmanager
#include <EEPROM.h>
#include <ESP8266WebServer.h> //Local WebServer used to
// ###############################
#if defined(ds18b20)
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
#endif
// ###############################
WiFiClientSecure wclient;
#if defined(pubClient)
#include <PubSubClient.h>
PubSubClient client(wclient); // for cloud broker - by hostname
#endif

//////////////////////////////////compass
#if defined(as5600)
#include <AS5600.h>
AS5600 encoder;
#endif
////////////////////////////////////////////
// serve the configuration portal
#include <FS.h>

#define DBG_OUTPUT_PORT Serial
#include <MD5.h>

#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TimeLib.h>
ESP8266WebServer server(80);
// ESP8266WebServer server = new ESP8266Webserver(80);

extern "C"
{
#include <user_interface.h>
}
#if defined(dht)
#include "DHTesp.h"
DHTesp dht;
#endif
////////////////////
/// emon//
#if defined(emon)
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
float PowerCorrection = 111.1;
/////////
#endif
#include <ESP8266SSDP.h>

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
bool try_MQTT_access = false;
bool IOT_Manager_loop = false;
int no_internet_timer = 0;
bool internet = false;
//////////////////CaptivePortalAdvanced
char softAP_ssid[32] = "dev_001";
char softAP_password[32] = "12345678";
char ssid[32] = "";
char password[32] = "";
uint8_t ipport = 80;
/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";
char deviceID[20] = "dev-"; // thing ID - unique device id in our project
#if defined(pubClient)
char mqttServerName[60] = "m20.cloudmqtt.com";
unsigned int mqttport = 16238;
char mqttuser[15] = "spspider";
char mqttpass[15] = "5506487";
unsigned char type_mqtt = 1;
#endif

/////////////IR
bool Page_IR_opened = false;
bool geo_enable = false;
#if defined(ws433)
bool loop_433 = true;
#endif
// bool ir_loop = false;
bool wifi_scan = true;
bool ws8211_loop = true;
bool save_stat = false;
bool IR_recieve = false;
bool loop_alarm_active = true;
bool check_internet = true;
short unsigned int mqttspacing = 60;
///////////////////433
#if defined(ws433)
char w433rcv = 255;
uint8_t w433send = 255;
#endif
//////////////////////////////
// String jsonConfig = "{}";
////////////TimeAlarmString/////////
const unsigned char Numbers = 1;   // ?????????? ??????? ? ?????? ??????
const unsigned char Condition = 1; // ?????????? ??????
unsigned char save_stat_long = 0;  // only initialized once
///////////////////////////////////////
uint8_t pwm_delay_long = 10;
///////////////////////////////////////////
// char trying_attempt_mqtt = 0;
uint8_t router = 255;
// ?????:
unsigned char countdown_lock = 0;
uint8_t onesec;
unsigned long millis_strart_one_sec;
uint8_t onesec_255;

bool license = 0;
bool test_action = false;

unsigned char PWM_frequency = 1;
// telegram global
#ifdef use_telegram
String BOTtoken = "";
#endif

/////////////////////////////ads
// ads1115
#include <Wire.h>
#if defined(ads1115)
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads(0x48);
#endif

///////////////////////////////
void setup()
{
  wclient.setInsecure(); // Disables certificate verification for testing purposes
#if defined(will_use_serial)
  Serial.begin(115200);
#endif
  delay(10);
  Serial.println();
  Serial.println();
  setup_FS();
  MD5Builder md5;
  md5.begin();
  md5.add(WiFi.macAddress() + "password");
  md5.calculate();
  if (readCommonFiletoJson("activation") == md5.toString())
  {
    license = 1;
  }

  if (loadConfig(SPIFFS.open("/other_setup.txt", "r")))
  {
  }
  captive_setup();
#if defined(ws2811_include)
  setup_ws2811(); // include ws2811.in
#endif
#if defined(wakeOnLan)
  setup_WOL();
#endif
#if defined(as5600)
  setup_compass();
#endif
  // ??????????? ? ????????? SSDP ?????????
  //   Serial.println("Start 3-SSDP");
  SSDP_init();

#if defined(ir_code)
  if (IR_recieve)
  {
    setup_IR();
  }
#endif
  //////////////////////////////////////
#if defined(ws433)
  setup_w433();
#endif
  // setup_wg();
  /////////////////////////////////////
  callback_from_stat();

#if defined(ds18b20)
  sensors.begin(); // Start up the library
#endif
}
void loop()
{
  captive_loop();
  test_loop();
  loop_websocket();
  if (IR_recieve)
  {
#if defined(ir_code)
    loop_IR();
#endif
  }
#if defined(ws433)
  if (w433rcv != 255)
  {
    loop_w433();
  }
#endif
  if (ws8211_loop == true)
  {
#if defined(ws2811_include)
    loop_ws2811(); // include ws2811.in
#endif
  }
#if defined(timerAlarm)
  if (loop_alarm_active)
  {

    loop_alarm();
  }
#endif
  if (millis() > 1000L + millis_strart_one_sec)
  {
    onesec++;
#ifdef use_telegram
    loop_telegram();
#endif
#if defined(timerAlarm)
    check_for_changes();
#endif
#if defined(pubClient)
    //    subscr_loop_PLUS();
    pubClientOneSecEvent();
#endif
    onesec_255++;
#if defined(timerAlarm)
    check_if_there_next_times();
#endif
#if defined(ws2811_include)
    one_sec();
#endif
    one_sec_lock();
    millis_strart_one_sec = millis();
  }
}



/*********
  ??? ?????? (Rui Santos)
  ????? ???????? ? ??????? ??: http://randomnerdtutorials.com
  ?????? ? IDE Arduino: File > Examples > Arduino OTA > BasicOTA.ino
                       (???? > ??????? > Arduino OTA > BasicOTA.ino)
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266HTTPUpdateServer httpUpdater;

void setup_ota() {
httpUpdater.setup(&server);
}




//#include <Ethernet.h>
//EthernetClient wclient;
//WiFiClient client_my;


//char smtp_arr[] = "mail.smtp2go.com";
//short unsigned int smtp_port = 2525;
//String to_email_addr = "spspider@mail.ru"; // destination email address
//String from_email_addr = "spspider95@smtp2go.com"; //source email address
////Use this site to encode: http://webnet77.com/cgi-bin/helpers/base-64.pl/
//String emaillogin = "c3BzcGlkZXI5NUBnbWFpbC5jb20="; //username
//String password_email = "NTUwNjQ4Nw=="; //password_email
//char timezone = 2;               // ??????? ???? GTM






byte sendEmail(String message)
{
  ///////////load email
  File load_other_setup = SPIFFS.open("/other_setup.txt", "r");
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, load_other_setup);

  if (error) {
    Serial.println("Failed to parse JSON! loadConfig for email");
    return false;
  }
  char smtp_arr[] = "mail.smtp2go.com";
  char timezone;

  const char *buff_smtp_arr = jsonDocument["smtp_arr"];
  snprintf(smtp_arr, sizeof smtp_arr, "%s", buff_smtp_arr);
  smtp_arr[sizeof(smtp_arr) - 1] = '\0';

  short unsigned int smtp_port = jsonDocument["smtp_port"];
  String to_email_addr = jsonDocument["to_email_addr"].as<String>();
  String from_email_addr = jsonDocument["from_email_addr"].as<String>();
  String emaillogin = jsonDocument["emaillogin"].as<String>();
  String password_email = jsonDocument["password_email"].as<String>();

  jsonDocument.containsKey("timezone") ? timezone = jsonDocument["timezone"] : timezone = 2;
  ///////////end load email
  byte thisByte = 0;
  byte respCode;

  if (wclient.connect(smtp_arr, smtp_port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }

  if (!eRcv()) return 0;

  Serial.println(F("Sending hello"));
  // replace 1.2.3.4 with your Arduino's ip
  wclient.println("EHLO");
  if (!eRcv()) return 0;

  Serial.println(F("Sending auth login"));
  wclient.println("auth login");
  if (!eRcv()) return 0;

  Serial.println(F("Sending User"));
  // Change to your base64 encoded user
  wclient.println(emaillogin);

  if (!eRcv()) return 0;

  Serial.println(F("Sending password_email"));
  // change to your base64 encoded password_email
  wclient.println(password_email);

  if (!eRcv()) return 0;

  // change to your email address (sender)
  Serial.println(F("Sending From"));
  wclient.print("MAIL FROM: <"); // identify sender
  wclient.print(from_email_addr);
  wclient.println(">");
  if (!eRcv()) return 0;

  // change to recipient address
  Serial.println(F("Sending To"));
  wclient.print("RCPT TO: <"); // identify recipient
  wclient.print(to_email_addr);
  wclient.println(">");
  if (!eRcv()) return 0;

  Serial.println(F("Sending DATA"));
  wclient.println("DATA");
  if (!eRcv()) return 0;

  Serial.println(F("Sending email"));

  // start of email
  wclient.print("To: ");
  wclient.println(to_email_addr);
  wclient.print("From: ");
  wclient.println(from_email_addr);
  wclient.print("Subject:");
  wclient.print(deviceID);
  wclient.println();

  unsigned char lenMessage = (message.length() / 50) + 1;
  unsigned int FromIndexComma = 0;
  unsigned int toIndexComma = 0;
  /*
    for (unsigned char i = 0; i < 10; i++) {
      FromIndexComma = message.indexOf(",", toIndexComma + 1);
      toIndexComma = message.substring(FromIndexComma + 1, message.length()).indexOf(",", FromIndexComma + 1);
      toIndexComma > 50 ? toIndexComma = 50 : true;
      wclient.println();
      wclient.println(message.substring(FromIndexComma, toIndexComma));
    }
  */
  //String SubstringMessage = message;

  for (unsigned char i = 0; i < lenMessage; i++) {
    wclient.println();
    wclient.println(message.substring((i * 50), (i * 50 + 50)));
    wclient.println();
  }

  //Serial.println(message);


  //    wclient.print(off_time);
  //    wclient.println(" ??????\r\n");

  wclient.println("End Email");

  wclient.println(".");

  if (!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  wclient.println("QUIT");
  if (!eRcv()) return 0;

  wclient.stop();

  Serial.println(F("disconnected"));

  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!wclient.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      wclient.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = wclient.peek();

  while (wclient.available())
  {
    thisByte = wclient.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}


void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  wclient.println(F("QUIT"));

  while (!wclient.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      wclient.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }

  while (wclient.available())
  {
    thisByte = wclient.read();
    Serial.write(thisByte);
  }

  wclient.stop();

  Serial.println(F("disconnected"));
}




void SSDP_init(void) {
  // SSDP ??????????
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });
  //???? ??????  2.0.0 ?????????????? ????????? ???????
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(ipport);
  SSDP.setName(deviceID);
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("LED");
  SSDP.setModelNumber("000000000001");
  //SSDP.setModelURL("http://esp8266-arduinoide.ru/step3-ssdp/");
  SSDP.setManufacturer("Sergey");
  //SSDP.setManufacturerURL("http://www.esp8266-arduinoide.ru");
  SSDP.begin();
}




#ifdef use_telegram
/*
  Name:        echoBot.ino
  Created:     12/21/2017
  Author:      Stefano Ledda <shurillu@tiscalinet.it>
  Description: a simple example that check for incoming messages
              and reply the sender with the received message
*/
#include "CTBot.h"
CTBot myBot;
CTBotInlineKeyboard myKbd;

void setup_telegram() {
  // initialize the Serial


  // set the telegram bot token
  myBot.setTelegramToken(BOTtoken);

  // check if all things are ok
  if (myBot.testConnection())
    Serial.println("\ntelegram Connection OK");
  else
    Serial.println("\ntelegram Connection Not OK");

  //  for (char i = 0; i < char(nWidgets); i++) {
  //    if (pinmode[i] == 2) {
  //      Serial.println("adding button");
  //
  //      char descr_on[20];
  //      sprintf(descr_on, "%s :on", descr[i], 20);
  //      char descr_off[20];
  //      sprintf(descr_off, "%s :off", descr[i], 20);
  //
  //      myKbd.addButton(descr_on, descr_on, CTBotKeyboardButtonQuery);
  //      myKbd.addButton(descr_off, descr_off, CTBotKeyboardButtonQuery);
  //      myKbd.addRow();
  //    }
  //  }
  for (char i = 0; i < char(nWidgets); i++) {
    if (pinmode[i] == 2) {
      char descr_on[10];
      snprintf(descr_on, sizeof(descr_on), "%s:on", descr[i]);  // Shortened length
      char descr_off[10];
      snprintf(descr_off, sizeof(descr_off), "%s:off", descr[i]);

      myKbd.addButton(descr_on, descr_on, CTBotKeyboardButtonQuery);
      myKbd.addButton(descr_off, descr_off, CTBotKeyboardButtonQuery);
      myKbd.addRow();
    }
  }
}

//void loop_telegram_char() { //char
//  // a variable to store telegram message data
//  TBMessage msg;
//  if (CTBotMessageText == myBot.getNewMessage(msg)) {
//    // ...forward it to the sender
//    char messageText[256];
//    strcpy(messageText, msg.text.c_str());
//
//    char* colonIndex = strchr(messageText, ':');
//
//    // Extract the substrings before and after the colon
//    if (colonIndex != NULL) {
//      // Extract the substrings before and after the colon
//      char part1[50];
//      strncpy(part1, messageText, colonIndex - messageText);
//      part1[colonIndex - messageText] = '\0';
//
//      char part2[50];
//      strcpy(part2, colonIndex + 1);
//
//      char Topic = atoi(part1);
//      Serial.print("Topic:");
//      Serial.println(Topic);
//      Serial.print("value:");
//      Serial.println(part2);
//
//      callback_socket(Topic, atoi(part2));
//
//      // Clear the contents of part1 and part2
//      memset(part1, 0, sizeof(part1));
//      memset(part2, 0, sizeof(part2));
//    }
//
//    char fullanswer[256] = "";
//    for (int i = 0; i < nWidgets; i++) {
//      char answer[50];
//      sprintf(answer, "%s: %.2f\n", descr[i], get_new_pin_value(i));
//      strcat(fullanswer, answer);
//    }
//    myBot.sendMessage(msg.sender.id, fullanswer);
//
//    // Clear the contents of fullanswer
//    memset(fullanswer, 0, sizeof(fullanswer));
//
//    delay(500);
//  }
//}
void loop_telegram() {
  TBMessage msg;
  char fullanswer[100];  // Adjust size based on expected response length
  fullanswer[0] = '\0';  // Initialize with empty string

  if (CTBotMessageText == myBot.getNewMessage(msg)) {
    char messageText[64];  // Define max message length
    msg.text.toCharArray(messageText, sizeof(messageText));

    int colonIndex = -1;
    for (int i = 0; i < sizeof(messageText); i++) {
      if (messageText[i] == ':') {
        colonIndex = i;
        break;
      }
    }

    if (colonIndex != -1) {
      messageText[colonIndex] = '\0';
      int Topic = atoi(messageText);
      int part2 = atoi(messageText + colonIndex + 1);
      callback_socket(Topic, part2);
    } else {
      snprintf(fullanswer, sizeof(fullanswer), "%s\nnot found,\nusing:\n0:1 - number:value\n", messageText);
    }

    for (unsigned char i = 0; i < nWidgets; i++) {
      char valueStr[10];
      dtostrf(get_new_pin_value(i), 2, 2, valueStr);  // Use `dtostrf` for float conversion
      snprintf(fullanswer + strlen(fullanswer), sizeof(fullanswer) - strlen(fullanswer), "%s: %s\n", descr[i], valueStr);
    }

    myBot.sendMessage(msg.sender.id, fullanswer);
//    delay(500);
  }
}
//void loop_telegram() { // String way
//  // a variable to store telegram message data
//  TBMessage msg;
//  //    if there is an incoming message...
//  if (CTBotMessageText == myBot.getNewMessage(msg)) {
//    String fullanswer = "";
//    // ...forward it to the sender
//    String messageText = msg.text;
//    char colonIndex = messageText.indexOf(':');
//    // Extract the substrings before and after the colon
//    if (colonIndex != -1) {
//      // Extract the substrings before and after the colon
//      String part1 = messageText.substring(0, colonIndex);
//      String part2 = messageText.substring(colonIndex + 1);
//      char Topic = (char) part1.toInt();
//      //      Serial.print("Topic:");
//      //      Serial.println(Topic, DEC);
//      //      Serial.print("value:");
//      //      Serial.println(part2);
//
//      callback_socket(Topic, part2.toInt());
//    } else {
//      fullanswer += messageText + "\n";
//      fullanswer += "\nnot found,\nusing:\n0:1 - number:value";
//    }
//
//    //////////////////STRING WORKING//////////////
//
//    // Assuming nWidgets is defined somewhere
//    for (unsigned char i = 0; i < nWidgets; i++) {
//      String valueStr = String(get_new_pin_value(i), 2);
//      fullanswer += String(descr[i]) + ": " + valueStr + "\n";
//    }
//    /////////////////////////////////////////
//
//    myBot.sendMessage(msg.sender.id, fullanswer);
//    delay(500);
//  }
//
//}
#endif



char incomingByte = 0;   // for incoming serial data
void test_setup() {
}
void test_loop() {
  if (Serial.available() > 0)
  {
    String readStringMy = Serial.readString();
  }
}




#include <TimeLib.h>
#if defined(timerAlarm)
bool En_a[Condition][Numbers];
uint8_t type_a[Condition][Numbers];
uint8_t act_a[Condition][Numbers];
unsigned int  type_value[Condition][Numbers];


char NumberIDs[Condition];
bool alarm_is_active[Condition][Numbers];

bool timer_alarm_action_switch = 0;
unsigned char timer_alarm_action = 0, timer_alarm_action_max = 20;

void setup_alarm() {

  for (char i1 = 0; i1 < Condition; i1++) {//??????????? ?? ???? ???????
    char thatCondition = i1;//idWidget ??????
    NumberIDs[thatCondition] = 0;
    String NameFile = "Condition" + String(thatCondition, DEC);
    String jsonCondition = readCommonFiletoJson(NameFile);
    if (jsonCondition != "") {
      load_Current_condition(jsonCondition);
    }
  }
  Serial.println("setup_alarm() - OK");
}


bool load_Current_condition(String jsonCondition) {
  if (jsonCondition != "") {
    DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, jsonCondition);
    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return false;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();
    if (error) {
      Serial.println("PARSE FAIL!!");
      return false;
    }

    char WidjetIds = rootjs["ID"]; // 1 //????? ??????
    int Numbers_that = rootjs["Numbers"]; // 1 //? ????? ????? ???-?? ????????
    char thatCondition = WidjetIds;
    Numbers_that > Numbers ? Numbers_that = Numbers : true;

    for (char i = 0; i < Numbers_that; i++) { //?? ????? ??????????? ????????
      char type_that = rootjs["type"][i];
      int timer_that = rootjs["timer"][i];
      char act_that = rootjs["act"][i];
      //      strncpy(actBtn_a_ch[thatCondition][i], rootjs["actBtn"][i], sizeof(actBtn_a_ch[thatCondition][i])); //error
      //      actBtn_a_ch[thatCondition][i] = rootjs["actBtn"][i];

      char actOn_that = rootjs["actOn"][i];



      bool En_that = rootjs["En"][i];

      type_a[thatCondition][i] = 0;
      type_a[thatCondition][i] = type_that;
      act_a[thatCondition][i] = act_that;

      type_value[thatCondition][i] = rootjs["type_value"][i].as<unsigned int>();
      En_a[thatCondition][i] = En_that;

      alarm_is_active[thatCondition][i] = alarm_is_active[thatCondition][i] ^ true;
    }

    NumberIDs[thatCondition] = Numbers_that; //?????????? ? ???? ??????? (?? ???? ??????);
    NumberIDs[thatCondition] > 10 ? NumberIDs[thatCondition] = 0 : true;
    //check_if_there_timer_times(thatCondition);
  }
  return true;
}

void check_if_there_timer_once(uint8_t idWidget) {//???????? ????????? ???????
  for (uint8_t i = 0; i < NumberIDs[idWidget]; i++) {
    if (type_a[idWidget][i] == 5) {//??????
      unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
      type_a[idWidget][i] = type_a[idWidget][i] + 10; //???????
      type_value[idWidget][i] = nowsec + type_value[idWidget][i];

      //idA[idWidget][i] = Alarm.timerOnce( timer_a[idWidget][i] * multiply, OnceOnly);
      Serial.println("????? ??????:" + String( nowsec / 3600, DEC) + ":" + String( nowsec % 3600 / 60, DEC) + ":" + String( nowsec % 60, DEC) );
      Serial.println("?????????? ??????:" + String( type_value[idWidget][i] / 3600, DEC) + ":" + String( type_value[idWidget][i] % 3600 / 60, DEC) + ":" + String( type_value[idWidget][i] % 60, DEC) );
    }
  }
}

void check_if_there_next_times() {//?????????? ?????? ???????

  for (uint8_t idWidget = 0; idWidget < Condition; idWidget++) {//??????????? ?? ???? ???????
    for (unsigned char i = 0; i < Numbers; i++) {
      if (En_a[idWidget][i]) {
        unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
        if (type_a[idWidget][i] == 15) {
          if ((nowsec == type_value[idWidget][i])) {
            test_action = false;
            Serial.println("??????? ?? ???????: " + String(type_value[idWidget][i], DEC) + " ??????:" + String(nowsec, DEC));
            make_action(idWidget, i, false);
            type_a[idWidget][i] = type_a[idWidget][i] - 10;
          }
        }
      }
    }
  }
}

static unsigned char l_minute;
void loop_alarm() {
  if (minute() != l_minute) {
    l_minute = minute();
  }
}
void CheckInternet(String request) {
  char timezone;
  String respond = getHttp(request);

  if (respond == "fail") { //????????? ???
    Serial.println("????????? ???");
    relayRouter();
  } else { //???????? ????
    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, respond);

    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();

    //"2018-08-23T07:43";
    String currentDateTime = rootjs["currentDateTime"];
    if (timeStatus() == timeNotSet) {
      setTime(
        currentDateTime.substring(11, 13).toInt() + timezone,
        currentDateTime.substring(14, 16).toInt(),
        0,
        currentDateTime.substring(8, 10).toInt(),
        currentDateTime.substring(5, 7).toInt(),
        currentDateTime.substring(0, 4).toInt()
      );
      //setup_alarm();
    } else {
      timezone = hour() - currentDateTime.substring(11, 13).toInt();
      Serial.println("timezone:" + String(timezone, DEC));
    }
    Serial.println("???????? ????");
  }
}

void check_for_changes() {
  if (timer_alarm_action_switch == 0) {
    for (uint8_t i1 = 0; i1 < Condition; i1++) {//??????????? ?? ???? ???????
      uint8_t idWidget = i1;
      for (uint8_t i = 0; i < Numbers; i++) {//?? ????? ??????????? ????????
        stat[idWidget] = (int) get_new_pin_value(idWidget);
        if (type_a[idWidget][i] == 2) {//????? (?? ??????)
          if (type_value[idWidget][i] == stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
        if (type_a[idWidget][i] == 3) {//??????
          if (type_value[idWidget][i] < stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
        if (type_a[idWidget][i] == 4) {//??????
          if (type_value[idWidget][i] > stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
      }

    }
  }
}



// functions to be called when an alarm triggers:

void MakeIfTrue(uint8_t idWidget, uint8_t i) {
  if (alarm_is_active[idWidget][i]) {
    make_action(idWidget, i, false);
    timer_alarm_action_switch = 1;
    alarm_is_active[idWidget][i] = false;
  }
}
void MakeIfFalse(char idWidget, char i) {
  if (!alarm_is_active[idWidget][i]) {
    if ((act_a[idWidget][i] != 5) ||  (act_a[idWidget][i] != 7) || (act_a[idWidget][i] != 8)) { // ??????????? ???????//act_a[i1][i] != 4)(act_a[idWidget][i] != 7)||(act_a[idWidget][i] != 8))
      make_action(idWidget, i, true);
    }
    alarm_is_active[idWidget][i] = true;
  }
}

void disable_En(uint8_t that_condtion_widget, uint8_t that_number_cond) {
  for (uint8_t i1 = 0; i1 < Condition; i1++) {
    for (uint8_t i = 0; i < NumberIDs[i1]; i++) {

      //if (actBtn_a[that_condtion_widget][that_number_cond] ==  actBtn_a[i1][i]) {
      //      if (strcmp (actBtn_a_ch[that_condtion_widget][that_number_cond], actBtn_a_ch[i1][i]) == 0) {
      //        Serial.println("??????:" + String(i1) + String(i) + String(actBtn_a_ch[i1][i]) + " " + String(actBtn_a_ch[that_condtion_widget][that_number_cond]));
      //        if ((that_number_cond != i) || (that_condtion_widget != i1)) {
      //          //????? ??????????, ? ???? ??? ?? ??, ??? ?????????, ?? ????????? ???
      //          Serial.println("!!!!!!!!!!!!!!????????? ???????, ??????? ???? ????????????? ?? ??? ??????" + String(i1, DEC) + String(i, DEC));
      //          En_a[i1][i] = false;
      //        }
      //      }

    }

  }

}

void parseStringToArray(String inputString, uint8_t values[], uint8_t& numValues) {
  numValues = 0; // Initialize the number of values
  while (inputString.length() > 0) {
    uint8_t spaceIndex = inputString.indexOf(' '); // Find the index of the next space
    if (spaceIndex == -1) {
      Serial.print("Parsed value number:");
      Serial.print(numValues);
      Serial.print("is:");
      values[numValues++] = inputString.toInt(); // Convert the remaining substring to an integer
      Serial.print(inputString.toInt());
      break;
    }
    String substring = inputString.substring(0, spaceIndex);
    values[numValues++] = substring.toInt();
    inputString = inputString.substring(spaceIndex + 1);
  }
}

String actBtn_a_ch_string(uint8_t that_condtion_widget, uint8_t that_number_cond) {
  File actBtn_a_ch_string_file = SPIFFS.open("/Condition" + String(that_condtion_widget, DEC) + ".txt", "r");
  String actBtn_a_ch = "";
  DynamicJsonDocument rootjs(1024); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(rootjs, actBtn_a_ch_string_file);
  if (error) {
    Serial.println("PARSE Condition" + String(that_condtion_widget, DEC) + "FAIL!!");
    return actBtn_a_ch;
  }
  char WidjetIds = rootjs["ID"];
  int Numbers_that = rootjs["Numbers"];

  actBtn_a_ch = rootjs["actOn"][that_number_cond].as<String>();

  return actBtn_a_ch;
}
void make_action(uint8_t that_condtion_widget, uint8_t that_number_cond, bool opposite) {
  if (En_a[that_condtion_widget][that_number_cond] == true) {//???? ?? ???????
    if (act_a[that_condtion_widget][that_number_cond] == 2) { ////////////////////////////"?????? ??????"////////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      callback_socket(values_back[0], values_back[1]);
      //values_back[0] - ?????? ?????? ? ?.?.
    }
    else if ((act_a[that_condtion_widget][that_number_cond] == 4) && (!opposite)) { //"????????? Email"//////////////////////////////////////////////////////////
      String buffer;
      buffer += actBtn_a_ch_string(that_condtion_widget, that_number_cond); //????????? ? ???????
      //      buffer = "????????? ??????? ?? ???????:" + String(descr[that_condtion_widget]) + " ?????:" + String(sTopic_ch[that_condtion_widget]) + " ?? ????:" + String(digitalRead(pin[that_condtion_widget]));
      buffer = "????????? ??????? ?? ???????:" + String(descr[that_condtion_widget]) + " ?? ????:" + String(digitalRead(pin[that_condtion_widget]));

      buffer += "\n";
      buffer += "????? ?? ??????????:" + String(hour()) + ":" + String(minute());
      buffer += "????????? ??????????????:";
      buffer += readCommonFiletoJson("ip_gps");
      buffer += "\n";
      Serial.print("Sending Email:");
      Serial.println(sendEmail(buffer));
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 1) { /////////////////////////////?????????? ???///////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      callback_socket(values_back[0], values_back[1]);
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 3) { ///////////////////////////????????? ??????///////////////////////////////////////////////////

      String host = "";//String(actBtn_a_ch[that_condtion_widget][that_number_cond]);//?????? ???? ? ??????
      int val_first = host.indexOf("val:");
      int val_last = host.indexOf("}");
      int value = host.substring(val_first + 4, val_last).toInt();


      if ((value > 1) && (opposite)) {
        value = 0;
      } else if (value < 2) {
        value ^= opposite;
      }
      host = host.substring(0, val_first + 4) + String(value, DEC) + "}";
      Serial.println("????????? ??????:" + host);
      //Serial.println("value:" + String(value^opposite));
      String respond = getHttp(host);
      //Serial.println(respond);
        }

    else if (act_a[that_condtion_widget][that_number_cond] == 7) { ///////////////////////////8211/////////////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      String DataLoad_8211 = readCommonFiletoJson("ws8211/" + String(values_back[0], DEC));
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
#if defined(ws2811_include)
      LoadData(buffer);
#endif
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 8) { /////////////////////////WakeOnLan///////////////////////////
      #if defined(wakeOnLan)
      char addresWakePC[20];
      strcpy(addresWakePC, actBtn_a_ch_string(that_condtion_widget, that_number_cond).c_str());
      wakeMyPC(addresWakePC);
      #endif
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 9) { /////////////////////////timer///////////////////////////
      //      switch_action(that_condtion_widget, that_number_cond, opposite);
      //      callback_socket(values_back[0], values_back[1]);
      uint8_t max_values = 5;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);


      unsigned char typePinsDuration_int = values_back[0];
      unsigned char  typePinsTimeChoise_int = values_back[1];
      unsigned char typePinsrepeats_int = values_back[2];
      unsigned char typeDelay_int = values_back[3];
      bool typeOpposite_int = values_back[4];

      Serial.print("?????????????????:"); Serial.println(typePinsDuration_int);
      Serial.print("???:"); Serial.println(typePinsTimeChoise_int);
      Serial.print("??????:"); Serial.println(typePinsrepeats_int);
      Serial.print("???????:"); Serial.println(typeDelay_int);
      Serial.print("typeOpposite_int:"); Serial.println(typeOpposite_int);
      //?????????? ????? ?????
      //?????? ?????? ?????:
      unsigned int typePinsTimeChoise_int_mult;
      switch (typePinsTimeChoise_int) {
        case 0:
          typePinsTimeChoise_int_mult = 1;//???????
          break;
        case 1:
          typePinsTimeChoise_int_mult = 60 * typePinsTimeChoise_int;//??????
          break;
        case 2://????
          typePinsTimeChoise_int_mult = 60 * 60 * typePinsTimeChoise_int;
          break;
      }

      if ((typePinsrepeats_int >= 0) && (typePinsrepeats_int != 255)) {
        if (!typeOpposite_int) {//????????? ?????? ??? ?? ?????
          type_value[that_condtion_widget][that_number_cond] = type_value[that_condtion_widget][that_number_cond] + typePinsDuration_int * typePinsTimeChoise_int_mult;
        } else {
          type_value[that_condtion_widget][that_number_cond] = type_value[that_condtion_widget][that_number_cond] + typeDelay_int * 60;
        }
        typeOpposite_int = typeOpposite_int ^ 1;
        typePinsrepeats_int--;
        //        sprintf(actBtn_a_ch[that_condtion_widget][that_number_cond], "%d %d %d %d %d", typePinsDuration_int, typePinsTimeChoise_int, typePinsrepeats_int, typeDelay_int, typeOpposite_int);
      }
      else if (typePinsrepeats_int == 255) {
        String NameFile = "Condition" + String(that_condtion_widget, DEC);
        String jsonCondition = readCommonFiletoJson(NameFile);
        jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
        //        switch_action(that_condtion_widget, that_number_cond, 1);
        Serial.println("!!!!!!!!!!!!!!!!!!!!???????? ??????? ?????????");
      }
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 10) { /////////////////////////////??????????? ??///////////////////////////////////////////


      float payload = get_new_pin_value(that_condtion_widget);//?????? ????? ??????? ?? ???? ??????? ??????????
      unsigned char minTemp, maxTemp, button_;
      if (payload != 0) {
        char * pEnd;
        String inputString = actBtn_a_ch_string(that_condtion_widget, that_number_cond);
        /////////////
        uint8_t max_values = 3;
        uint8_t values_back[max_values];
        parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
        minTemp = values_back[0];
        maxTemp = values_back[1];
        button_ = values_back[2];

        payload = ((payload - minTemp * 1.0) * (1024.0)) / (maxTemp  - minTemp) * 1.0;
        payload = payload < 0 ? 0 : payload;
        payload = payload > 1024 ? 1024 : payload;

        //uint8_t id_button = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], NULL, 10);
        callback_socket(button_, payload);
      }
    }
  }
}

#endif


#include <FS.h>
const char *fsName = "SPIFFS";
FS *fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();

static bool fsOK;
String unsupportedFiles = String();
#define USE_SPIFFS
File uploadFile;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK()
{
  server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg)
{
  server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg)
{
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg)
{
  DBG_OUTPUT_PORT.println(msg);
  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg)
{
  DBG_OUTPUT_PORT.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

#ifdef USE_SPIFFS
/*
   Checks filename for character combinations that are not supported by FSBrowser (alhtough valid on SPIFFS).
   Returns an empty String if supported, or detail of error(s) if unsupported
*/
String checkForUnsupportedPath(String filename)
{
  String error = String();
  if (!filename.startsWith("/"))
  {
    error += F("!NO_LEADING_SLASH! ");
  }
  if (filename.indexOf("//") != -1)
  {
    error += F("!DOUBLE_SLASH! ");
  }
  if (filename.endsWith("/"))
  {
    error += F("!TRAILING_SLASH! ");
  }
  return error;
}
#endif

bool handleFileRead(String path)
{
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if ((path == "/scripts/bootstrap.min.css") || (path == "/scripts/chart.min.js") || (path.endsWith(".js")) || (path.endsWith(".htm")) || (path.endsWith(".css")))
  {
    Serial.println("CASHE_CONTROL: " + path);
    server.sendHeader("Cache-Control", "public, max-age=604800, must-revalidate");
    server.sendHeader("Pragma", "public");
    server.sendHeader("Expires", "604800");
  }

  // if ((path.indexOf(".") == -1) && (!path.endsWith("/"))) path += ".htm";
  if (path.endsWith("/"))
    path += "home.htm";
  String contentType;
  if (server.hasArg("download"))
  {
    contentType = F("application/octet-stream");
  }
  else
  {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path))
  {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path))
  {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size())
    {
      DBG_OUTPUT_PORT.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  if (server.uri() != "/edit")
  {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/"))
    {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile)
    {
      return replyServerError(F("CREATE FAILED"));
    }
    DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (uploadFile)
    {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize)
      {
        return replyServerError(F("WRITE FAILED"));
      }
    }
    DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile)
    {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
    replyOKWithMsg("OK");
  }
}
void deleteRecursive(String path)
{
  File file = fileSystem->open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir)
  {
    fileSystem->remove(path);
    return;
  }

  // Otherwise delete its contents first
  Dir dir = fileSystem->openDir(path);

  while (dir.next())
  {
    deleteRecursive(path + '/' + dir.fileName());
  }

  // Then delete the folder itself
  fileSystem->rmdir(path);
}
void handleFileDelete()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg(0);
  if (path.isEmpty() || path == "/")
  {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!fileSystem->exists(path))
  {
    return replyNotFound(FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOKWithMsg(lastExistingParent(path));
}

bool FileDelete(String path)
{
  if (!SPIFFS.exists(path))
    return false;
  SPIFFS.remove(path);
  Serial.print("File deleted");
  Serial.println(path);
  path = String();
  return true;
}
String lastExistingParent(String path)
{
  while (!path.isEmpty() && !fileSystem->exists(path))
  {
    if (path.lastIndexOf('/') > 0)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    else
    {
      path = String(); // No slash => the top folder does not exist
    }
  }
  DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}
void handleFileCreate()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String path = server.arg("path");
  if (path.isEmpty())
  {
    return replyBadRequest(F("PATH ARG MISSING"));
  }
#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0)
  {
    return replyServerError(F("INVALID FILENAME"));
  }
#endif
  if (path == "/")
  {
    return replyBadRequest("BAD PATH");
  }
  if (fileSystem->exists(path))
  {
    return replyBadRequest(F("PATH FILE EXISTS"));
  }

  String src = server.arg("src");
  if (src.isEmpty())
  {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/"))
    {
      // Create a folder
      path.remove(path.length() - 1);
      if (!fileSystem->mkdir(path))
      {
        return replyServerError(F("MKDIR FAILED"));
      }
    }
    else
    {
      // Create a file
      File file = fileSystem->open(path, "w");
      if (file)
      {
        file.write((const char *)0);
        file.close();
      }
      else
      {
        return replyServerError(F("CREATE FAILED"));
      }
    }
    if (path.lastIndexOf('/') > -1)
    {
      path = path.substring(0, path.lastIndexOf('/'));
    }
    replyOKWithMsg(path);
  }
  else
  {
    // Source specified: rename
    if (src == "/")
    {
      return replyBadRequest("BAD SRC");
    }
    if (!fileSystem->exists(src))
    {
      return replyBadRequest(F("SRC FILE NOT FOUND"));
    }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/"))
    {
      path.remove(path.length() - 1);
    }
    if (src.endsWith("/"))
    {
      src.remove(src.length() - 1);
    }
    if (!fileSystem->rename(src, path))
    {
      return replyServerError(F("RENAME FAILED"));
    }
    replyOKWithMsg(lastExistingParent(src));
  }
}
void handle_sendEmail()
{
  // String email_txt = server.arg("Email"); // ???????? ???????? ssdp ?? ??????? ????????? ? ?????????? ??????????
  if (sendEmail(server.arg("Email")))
  {
    server.send(200, "text/plain", "send OK"); // ?????????? ????? ? ??????????
  }
  else
  {
    server.send(200, "text/plain", "Fail"); // ?????????? ????? ? ??????????
  }
}

void handle_setTime()
{
  char timezone;
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed

  DeserializationError error = deserializeJson(jsonDocument, server.arg("DateTime"));
  if (error)
  {
    Serial.print(F("deserializeJson() failed with handle_setTime code "));
    Serial.println(error.c_str());
    return;
  }

  short int hour_r = jsonDocument["h"];
  short int min_r = jsonDocument["m"];
  short int day_r = jsonDocument["d"];
  short int month_r = jsonDocument["n"];
  short int year_r = jsonDocument["y"];
  short int new_timezone;

  if (timeStatus() == timeNotSet)
  {
    Serial.println("Time not set");
  }

  if (timeStatus() == timeNeedsSync)
  {
    Serial.println("Time needs sync");
  }

  if (year_r != 0)
  {
    if (timeStatus() == timeSet)
    {
      if (hour_r > hour())
      {
        new_timezone = hour_r - (hour() - timezone);
      }
      else
      {
        new_timezone = hour() - (hour_r - timezone);
      }
      timezone = new_timezone;
    }

    short int timestat = timeStatus();

    setTime(hour_r, min_r, 0, day_r, month_r, year_r);

    if ((timestat == timeNotSet) && (timeStatus() == timeSet))
    {
      // setup_alarm();
    }
  }

  DynamicJsonDocument jsonDocument_back(1024); // Adjust the capacity as needed
  JsonObject json = jsonDocument_back.to<JsonObject>();
  json["h"] = hour();
  json["m"] = minute();
  json["s"] = second();
  json["d"] = day();
  json["n"] = month();
  json["y"] = year();
  json["t"] = timezone;

  String buffer;
  serializeJson(json, buffer);
  Serial.println(buffer);
  server.send(200, "text/json", buffer);
}

void handleFileList()
{
  if (!fsOK)
  {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  if (!server.hasArg("dir"))
  {
    return replyBadRequest(F("DIR ARG MISSING"));
  }

  String path = server.arg("dir");
  if (path != "/" && !fileSystem->exists(path))
  {
    return replyBadRequest("BAD PATH");
  }

  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  Dir dir = fileSystem->openDir(path);
  path.clear();

  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
  if (!server.chunkedResponseModeStart(200, "text/json"))
  {
    server.send(505, F("text/html"), F("HTTP1.1 required"));
    return;
  }

  // use the same string for every line
  String output;
  output.reserve(64);
  while (dir.next())
  {
#ifdef USE_SPIFFS
    String error = checkForUnsupportedPath(dir.fileName());
    if (error.length() > 0)
    {
      DBG_OUTPUT_PORT.println(String("Ignoring ") + error + dir.fileName());
      continue;
    }
#endif
    if (output.length())
    {
      // send string from previous iteration
      // as an HTTP chunk
      server.sendContent(output);
      output = ',';
    }
    else
    {
      output = '[';
    }

    output += "{\"type\":\"";
    if (dir.isDirectory())
    {
      output += "dir";
    }
    else
    {
      output += F("file\",\"size\":\"");
      output += dir.fileSize();
    }

    output += F("\",\"name\":\"");
    // Always return names without leading "/"
    if (dir.fileName()[0] == '/')
    {
      output += &(dir.fileName()[1]);
    }
    else
    {
      output += dir.fileName();
    }

    output += "\"}";
  }

  // send last string
  output += "]";
  server.sendContent(output);
  server.chunkedResponseFinalize();
}

void handle_saveIR()
{
  String IRjson = server.arg("IR");
  saveCommonFiletoJson("IRButtons", IRjson, 1);
#if defined(ir_code)
  updateIR();
#endif
}

void setup_FS(void)
{

  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DBG_OUTPUT_PORT.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));
  // SERVER INIT
  // list directory
}
void handleAJAX()
{
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, server.arg("json"));
  if (error)
  {
    Serial.print(F("deserializeJson() failed with handleAJAX code "));
    Serial.println(error.c_str());
    return;
  }
  uint8_t Topic_is = jsonDocument["t"].as<unsigned char>(); //
  int newValue = jsonDocument["v"];
  callback_socket(Topic_is, newValue);
}

void FunctionHTTP()
{
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed

  DeserializationError error = deserializeJson(jsonDocument, server.arg("json"));
  if (error)
  {
    Serial.print(F("deserializeJson() FunctionHTTP failed with code "));
    Serial.println(error.c_str());
    return;
  }

  if (jsonDocument.containsKey("reboot"))
  {
    if (captivePortal())
    {
      delay(500);
    }
    ESP.reset();
  }

  if (jsonDocument.containsKey("ws2811_setup"))
  {
#if defined(ws2811_include)
    loadLimits();
#endif
  }
  if (jsonDocument.containsKey("wifi_mac"))
  {
    server.send(200, "text/plain", WiFi.macAddress());
  }
  if (jsonDocument.containsKey("pin_setup_limits"))
  {
    String buffer = String(nWidgetsArray, DEC);
    server.send(200, "text/plain", buffer);
  }

  if (jsonDocument.containsKey("cond_setup"))
  {
    JsonObject json = jsonDocument.to<JsonObject>();
    json["ConNum"] = Condition;
    json["NumCon"] = Numbers;
    String buffer;
    serializeJson(json, buffer);
    server.send(200, "text/json", buffer);
  }

  if (jsonDocument.containsKey("WOL"))
  {
#if defined(wakeOnLan)
    const char *mac = jsonDocument["WOL"];
    wakeMyPC(mac);
#endif
  }

  if (jsonDocument.containsKey("setZeroFuel"))
  {
    String Page;
    Page += F("set_analogSubtracter_value: ");
    Page += setZeroFuel();
    Page += F(" ");
    Page += F("set_analogDivider_value: ");
    Page += String(analogDivider, DEC);
    Page += F(" ");

    server.send(200, "text/html", Page);
  }

  if (jsonDocument.containsKey("setFUllFuel"))
  {
    String Page;
    Page += F("set_analogSubtracter_value: ");
    Page += analogSubtracter;
    Page += F(" ");
    Page += F("set_analogDivider_value: ");
    Page += setFUllFuel(jsonDocument["setFUllFuel"]);
    Page += F(" ");
    server.send(200, "text/html", Page);
  }

  if (jsonDocument.containsKey("NextRepeat"))
  {
    uint8_t Condition = jsonDocument["NextRepeatCondition"];
    uint8_t Number = jsonDocument["NextRepeatNumber"];
    JsonObject json = jsonDocument.to<JsonObject>();
    json["actBtn_a_ch"] = 0;
#if defined(timerAlarm)
    json["times"] = type_value[Condition][Number];
#endif
    json["Number"] = Number;
    String buffer;
    serializeJson(json, buffer);
    server.send(200, "text/json", buffer);
  }

  if (jsonDocument.containsKey("Activation"))
  {
    uint8_t Activation = jsonDocument["Activation"];

    if (Activation == 0)
    {
      server.send(200, "text/plain", String(license, DEC));
    }
    else if (Activation == 1)
    {
      server.send(200, "text/plain", WiFi.macAddress());
    }
    else if (Activation == 2)
    {
      // calculate md5:
      MD5Builder md5;
      md5.begin();
      md5.add(WiFi.macAddress() + "password");
      md5.calculate();
      String generatedCode = md5.toString();
      String recievedCode = jsonDocument["code"].as<String>();

      if (recievedCode == generatedCode)
      {
        if (saveCommonFiletoJson("activation", recievedCode, 1))
        {
          server.send(200, "text/plain", "1");
          license = 1;
        }
        else
        {
          server.send(200, "text/plain", "failWrite");
          return;
        }
      }
      else
      {
        server.send(200, "text/plain", "0");
      }
    }
    else if (Activation == 4)
    {
      if (FileDelete("activation.txt"))
      {
        license = 0;
      }
      server.send(200, "text/plain", "license=" + String(license, DEC));
    }
  }
#if defined(as5600)
  if (jsonDocument.containsKey("EncoderIA"))
  {
    no_internet_timer = jsonDocument["rotations"];
    server.send(200, "text/plain", server.arg("json"));
  }
#endif

  if (jsonDocument.containsKey("sendIR"))
  {
#if defined(ir_code)
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    server.send(200, "text/plain", server.arg("json"));
#endif
  }
}

void handlews2811()
{
#if defined(ws2811_include)
  char buffer[200];
  server.arg("json").toCharArray(buffer, sizeof buffer);
  if (LoadData(buffer))
  { // include ws2811.in
    server.send(200, "text / plain", "OK");
  }
  loop_ws2811(); // include ws2811.in
#endif
}
void handlews2811set()
{
  char buffer[400];
  server.arg("json").toCharArray(buffer, sizeof buffer);
#if defined(ws2811_include)
  if (LoadData_set_leds(buffer))
  { // include ws2811.in
    server.send(200, "text / plain", "OK");
  }
  else
  {
    server.send(200, "text / plain", server.arg("json"));
  }
  loop_ws2811(); // include ws2811.in
#endif
}
void Server_begin()
{

  server_init();
  Captive_server_init();
  // server.begin();
}
void server_init()
{
  server.on("/sendAJAX", handleAJAX);
  server.on("/sendEmail", handle_sendEmail);
  server.on("/ws2811AJAX", handlews2811);
  server.on("/ws2811AJAXset", handlews2811set);
  server.on("/ws2811", []()
            { handleFileRead("/ws2811.html"); });
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/", HTTP_GET, []() { //
    if (WiFi.getMode() == WIFI_STA)
    {
      handleFileRead("/home.htm");
    }
    else
    {
      handleRoot();
    }

  });
  server.on("/setDate", HTTP_GET, handle_setTime);
  server.on("/other_setup", []()
            { handleFileRead("/other_setup.htm"); });
  server.on("/aRest", HTTP_GET, []() { /*192.168.1.108/aRest?Json={pin:1,val:100}*/
                                       // String json = server.arg("Json");
                                       Serial.println(server.arg("Json"));
                                       makeAres_sim(server.arg("Json"));
  });
  /*
    server.on("/CommonSave", []() { //???????? ??????? AJAX ????????
    saveCommonFiletoJson(server.arg("fileName"), server.arg("json"));
    });
  */
  // server.on("/CommonDelete", HTTP_DELETE, handleFileDelete);
  server.on("/IR_setup", []()
            {

    //Serial.println("IR true");
    Page_IR_opened = false;
    handleFileRead("/IR_setup.htm"); });

  server.on("/WaitIR", []() { // ???????? ??????? AJAX ???????? IR
    Page_IR_opened = true;
  });
  server.on("/open", HTTP_GET, handleOpenLockForTime);
  /*
    server.on("/SaveIR", []() {//!!!!!!!!!!!!!!!!!!????? ??????????
      handle_saveIR();
      //setup_IR(true);
    });
  */
  /*
    server.on("/IRButtons.txt", []() {
    String jsonConfig = readCommonFiletoJson("IRButtons");
    server.send(200, "text/json", jsonConfig);
    });
  */
  ////////////////////////////////////
  // server.on("/pin_setup.txt", handle_ConfigJSON_pinSetup); // ???????????? configs.json ???????? ??? ???????? ?????? ? web ?????????
  /*
    server.on("/pin_setup", HTTP_POST, []() {
    String jsonrecieve = server.arg("json_name");
    Serial.println(jsonrecieve);
    if (saveCommonFiletoJson("pin_setup", jsonrecieve)) {
      Serial.println("new Pinsetup");
    }
    updatepinsetup(jsonrecieve);
    Serial.println();
    Serial.println("recieve nested");
    handleFileRead("/pin_setup.htm");
    });
  */
  server.on("/pin_setup", []()
            { handleFileRead("/pin_setup.htm"); });
  server.on("/function", FunctionHTTP);
  server.on("/condition", []()
            { handleFileRead("/condition.htm"); });
  server.on("/help", []()
            { handleFileRead("/help.htm"); });
  /*
    server.on("/License", []() {
    if (server.arg("code")) {
      char code[5];
      server.arg("code").toCharArray(code, 5);
      saveEEPROM(0, code);
    } else {
      char* isLicensed = getEEPROM(0);
      server.send(404, "text/plain", isLicensed);
    }
    });
  */
  // server.on("/home", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileRead("/home.htm"));

  server.on("/edit", HTTP_GET, []()
            {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound"); });

  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on("/edit", HTTP_POST, []() {

  },
            handleFileUpload);

  // called when the url is not defined here
  // use it to load content from SPIFFS
  server.onNotFound([]()
                    {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound"); });

  // get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []()
            {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String(); });
}

void handleOpenLockForTime()
{
  // open_lock();
  // countdown_lock = //server.arg("t").charAt(0);
  countdown_lock = server.arg("t").toInt();
  sendMyheader();
  String Page = sendHead();
  if (countdown_lock != 0)
  {
    Page += F("<p>????? ?????? ??:");
    Page += (server.arg("t"));
    Page += F("?????</p>");
  }
  else
  {
    // lock_door();
    Page += F("<h1>????? ??????</h1>");
  }
  Page += F("<p><br><a href='/open?t=0'><button>???????</button></a></p>");
  server.send(200, "text/html", Page);
  Serial.println(countdown_lock, DEC);
}
void Captive_server_init()
{
  // server.on("/mode", []()
  //           {
  //   String mode = (WiFi.getMode() == WIFI_STA) ? "STA" : "AP";
  //   String jsonResponse = "{\"mode\":\"" + mode + "\"}";
  //   server.send(200, "application/json", jsonResponse); });
  server.on("/wifi", []()
            {
    if (WiFi.getMode() == WIFI_STA) {//??? ??????
      handleFileRead("/wifi_setup.htm");
    } else {
      handleWifi();
    } });
  server.on("/wifi", []()
            { handleFileRead("/wifi_setup.htm"); });
  server.on("/wifiList", handleWifilist);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot); // Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);       // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.

  server.serveStatic("/font", SPIFFS, "/font", "max-age=86400");
  server.serveStatic("/js", SPIFFS, "/js", "max-age=86400");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");

  // server.onNotFound ( handleNotFound );
  Serial.println("HTTP server started");
}



/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/
#if defined(ws433)
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup_w433() {
  //Serial.begin(9600);
  if (w433rcv != 255) {
    mySwitch.enableReceive(w433rcv);  // Receiver on interrupt 0 => that is pin #2
    Serial.println("W433_Enabled" + String(w433rcv, DEC));
    //mySwitch.disableReceive(15);
  }
}

void saveocde_to_file(String code) {
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
  // Load existing data from file
//  String existingData = readCommonFiletoJson("w433");
  File existingData = SPIFFS.open("/w433.txt", "r");
  DeserializationError error = deserializeJson(jsonDocument, existingData);
  if (error) {
    Serial.print(F("deserializeJson() failed with saveocde_to_file code "));
    Serial.println(error.c_str());
    return;
  }
  // Create a new JsonObject
  JsonObject jsonObj = jsonDocument.to<JsonObject>();
  // Add or update the code and time in the JsonObject
  jsonObj["code"] = code;
  jsonObj["time"] = String(hour()) + ":" + minute();
  // Convert JsonObject to a string
  String buffer;
  serializeJson(jsonObj, buffer);
  // Save the updated data to the file
  saveCommonFiletoJson("w433", buffer, 0);
}

void check_code_w433(String codeIR) {
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed

  File irlist = SPIFFS.open("/IRButtons.txt", "r");
  DeserializationError error = deserializeJson(jsonDocument, irlist);
  if (error) {
    Serial.print(F("deserializeJson() failed with code check_code_w433"));
    Serial.println(error.c_str());
    return;
  }

  char numbers_i = jsonDocument.containsKey("num") ? jsonDocument["num"] : 0;
  
  for (char i = 0; i < numbers_i; i++) {
    String code = jsonDocument["code"][i].as<String>();
    if (code == codeIR) { //found
      String name_i = jsonDocument["name"][i].as<String>();
      //write logic to do action
      for (char i1 = 0; i1 < char(nWidgets); i1++) {
        if (String(descr[i1]) == name_i) {
          Serial.print("do action:");
          Serial.println(name_i);
          callback_socket(i1, int(stat[i1]) ^ 1);
        }
      }
    }
  }
}

void loop_w433() {

  if (mySwitch.available()) {
    String codeIR;
    int value = mySwitch.getReceivedValue();

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      //Serial.print("Received ");
      //Serial.print( mySwitch.getReceivedValue() );
      codeIR = String(mySwitch.getReceivedValue());
      Serial.println(codeIR);
      if (!Page_IR_opened) {
        check_code_w433(codeIR);
        //        saveocde_to_file(codeIR);
      }
      else {
        server.send(200, "text/plain", codeIR);
      }
      //Serial.print("/");
      //Serial.print( mySwitch.getReceivedBitlength() );
      //Serial.print("bit ");
      //Serial.print("Protocol: ");
      //Serial.println( mySwitch.getReceivedProtocol() );
      //server.send(200, "text/plain", String(value));
      delay(500);
    }

    mySwitch.resetAvailable();
  }
  //mySwitch.resetAvailable();



}
#endif



void one_sec_lock() {
  
}



void callback_socket(uint8_t i, int payload_is)
{
  bool that_Ajax = false;
  bool saveEEPROM = false;
  if (i == 127)
  { // ??????? ?????? ??????
    pubStatusFULLAJAX_String(false);
    return;
  }

  if (pinmode[i] == 3)
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
    low_pwm[i] = payload_is;
    stat[i] = (payload_is);
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

void loop_pwm()
{
  int pwm_long = pwm_delay_long * 240;
  for (unsigned char i = 0; i < nWidgets; i++)
  {
    if (pinmode[i] == 5)
    {                        // low_pwm
      newtimePWM = millis(); // ?????? ??? 1 ???
      if (!low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (low_pwm[i] * (pwm_long / 1024)))
        {                          // ???? ???????????? ? ?????? ?????? ??????????? ????????
          digitalWrite(pin[i], 0); // ????? - ?????????
          low_pwm_off = true;
        }
      }
      if (newtimePWM - oldtimePWM > pwm_long)
      {                          // 1 sec//
        digitalWrite(pin[i], 1); // ???????? ?????
        oldtimePWM = newtimePWM;
        low_pwm_off = false;
      }
    }
    if (pinmode[i] == 7)
    {                        // MQ7
      newtimePWM = millis(); // ?????? ??? 1 ???
      if (low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (90 * 1000))
        {                            // 90 ??????
          analogWrite(pin[i], 1024); // 5V
          low_pwm_off = false;
          oldtimePWM = newtimePWM;
        }
      }
      else
      {
        if (newtimePWM - oldtimePWM > (60 * 1000))
        {                           // 60 ??????
          analogWrite(pin[i], 286); // 1.4V
          low_pwm_off = true;
          oldtimePWM = newtimePWM;
        }
      }
    }
  }
}

void pubStatusFULLAJAX_String(bool save_eeprom)
{ // ???????? ?? ?????? _nobuffer
  String stat1 = "{\"stat\":[";
  for (char i = 0; i < nWidgets; i++)
  {
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


void loop_websocket()
{

  int pwm_long = pwm_delay_long * 240;
  for (unsigned char i = 0; i < nWidgets; i++)
  {
    if (pinmode[i] == 5)
    {                        // low_pwm
      newtimePWM = millis(); // ?????? ??? 1 ???
      if (!low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (low_pwm[i] * (pwm_long / 1024)))
        {                          // ???? ???????????? ? ?????? ?????? ??????????? ????????
          digitalWrite(pin[i], 0); // ????? - ?????????
          low_pwm_off = true;
        }
      }
      if (newtimePWM - oldtimePWM > pwm_long)
      {                          // 1 sec//
        digitalWrite(pin[i], 1); // ???????? ?????
        oldtimePWM = newtimePWM;
        low_pwm_off = false;
      }
    }
    if (pinmode[i] == 7)
    {                        // MQ7
      newtimePWM = millis(); // ?????? ??? 1 ???
      if (low_pwm_off)
      {
        if (newtimePWM - oldtimePWM > (90 * 1000))
        {                            // 90 ??????
          analogWrite(pin[i], 1024); // 5V
          low_pwm_off = false;
          oldtimePWM = newtimePWM;
        }
      }
      else
      {
        if (newtimePWM - oldtimePWM > (60 * 1000))
        {                           // 60 ??????
          analogWrite(pin[i], 286); // 1.4V
          // analogWrite(pin[i], 0);//1.4V
          low_pwm_off = true;
          oldtimePWM = newtimePWM;
        }
      }
    }
  }
}



#if defined(wakeOnLan)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <WakeOnLan.h>

WiFiUDP UDP;
WakeOnLan WOL(UDP);


void wakeMyPC(const char* MACAddress) {
  //MACAddress = "54:04:A6:F2:2F:5A";
  for (uint8_t i = 0; i < 3; i++) {
    WOL.sendMagicPacket(MACAddress); // Send Wake On Lan packet with the above MAC address. Default to port 9.
    delay(100);
  }
  //return 1;
  // WOL.sendMagicPacket(MACAddress, 7); // Change the port number
}

void wakeOfficePC() {
  const char *MACAddress = "01:23:45:67:89:AB";
  const char *secureOn = "FE:DC:BA:98:76:54";

  WOL.sendSecureMagicPacket(MACAddress, secureOn); // Send Wake On Lan packet with the above MAC address and SecureOn feature. Default to port 9.
  // WOL.sendSecureMagicPacket(MACAddress, secureOn, 7); // Change the port number
}

void setup_WOL()
{
  WOL.setRepeat(3, 100); // Optional, repeat the packet three times with 100ms between. WARNING delay() is used between send packet function.

  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask()); // Optional  => To calculate the broadcast address, otherwise 255.255.255.255 is used (which is denied in some networks).

  //  wakeMyPC();
  //wakeOfficePC();
}
#endif



#if defined(ws2811_include)

#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//#define CLK_PIN   4
//#define DATA_PIN 15
#define DATA_PIN 15 //lock2,any 15
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    88//40//53 - ?????: 52 ????? 88, ????????? 8
//#if defined(ws2811_include)
CRGB leds[NUM_LEDS];

//CRGB leds_prep[NUM_LEDS];
#define NUM_CHARTS    1//2
#include <ArduinoJson.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER


#define BRIGHTNESS          100
#define FRAMES_PER_SECOND  500

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

unsigned char from_ws8211[NUM_CHARTS];
unsigned char to_ws8211[NUM_CHARTS];
unsigned char type_ws8211[NUM_CHARTS];
unsigned char dir_ws8211[NUM_CHARTS];
unsigned char sp_ws8211;
unsigned char duration = 255;
unsigned char fade = 20;
unsigned char bright = 255;
unsigned char fadetype = 3;
bool running_led = true;

unsigned char fade_sunrise[NUM_LEDS];

unsigned  char col_ws8211[NUM_CHARTS];
unsigned  char white_ws8211[NUM_CHARTS];
unsigned  char br__ws8211[NUM_CHARTS];
unsigned char delay_loop_8211 = 0;
//unsigned  char white_white_col_[NUM_CHARTS];

char num_ws8211 = 0;


short int pos[NUM_CHARTS];
long millis_strart;
//String Previous_set = "";
//uint8_t Previous_set_char;
bool reverse_set;
boolean inv;

void loadLimits() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["NUM_LEDS"] = NUM_LEDS;
  json["NUM_CHARTS"] = NUM_CHARTS;
  json["DATA_PIN"] = DATA_PIN;
  String buffer;
  json.printTo(buffer);
  server.send(200, "text/json", buffer);
}
void setup_ws2811() {

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //String dataLoad = "{\"from\":[0,20],\"to\":[20,40],\"type\":[4,4],\"dir\":[0,0],\"col\":[0,1],\"num\":2}";
  //String dataLoad = "{\"from\":[0,20],\"to\":[20,40],\"type\":[5,5],\"dir\":[0,10],\"col\":[0,1],\"num\":2}";
  //String dataLoad = "{\"from\":[0,21],\"to\":[21,40],\"type\":[1,6],\"dir\":[0,1],\"col\":[0,1],\"num\":1,\"sp\":10,\"dr\":255}";
  //String dataLoad = "{\"from\":[0,10],\"to\":[10,20],\"type\":[1,1],\"dir\":[0,1],\"col\":[0,0],\"num\":2,\"sp\":50,\"fd\":200}";
  //String dataLoad = "{\"from\":[0],\"to\":[40],\"type\":[13],\"dir\":[0],\"col\":[0],\"num\":1,\"sp\":10,\"fd\":50,\"br\":5,\"r\":1}";
  //LoadData(dataLoad);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  String DataLoad_8211 = readCommonFiletoJson("ws8211/0");
  char buffer[200];
  DataLoad_8211.toCharArray(buffer, sizeof buffer);
  LoadData(buffer);

}
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool LoadData_set_leds(char json[400]) {
  Serial.println(json);
  //running_led = true;
  DynamicJsonBuffer jsonBuffer;
  //char json[] =
  //ws2811AJAXset?json={%22g1%22:[192,192],%22g3%22:[192,192],%22num%22:2,%22br%22:255,%22wh%22:0}
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed LoadData:");
    Serial.println(json);
    return false;
  }
  unsigned char num_ = root["num"];
  unsigned char wh = root["wh"];
  unsigned char g1 = 255;
  for (unsigned char i = 0; i < NUM_LEDS; i++) {
    // unsigned char g1 = root["g1"][i];
    //unsigned char g2 = root["g2"][i];//??

    unsigned char g3 = root["g3"][i];//??
    if (i > num_) {
      leds[i] = CHSV( 0, 0, 0);//hue,white,bright
    } else {
      leds[i] = CHSV( g1, wh, g3);//hue,white,bright
    }

  }

  FastLED.show();
}

bool LoadData(char json[200]) {
  Serial.println(json);
  running_led = true;
  DynamicJsonBuffer jsonBuffer;
  //char json[] =
  //"{\"from\":[0,10],\"to\":[10,20],\"type\":[4,3],\"dir\":[1,0],\"col\":[0,1],\"num\":2}";
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed LoadData:");
    Serial.println(json);
    return false;
  }


  //if (root["num"] <= NUM_CHARTS) {
  unsigned char num_ = root["num"];
  num_ws8211 = num_ <= NUM_CHARTS ? num_ : NUM_CHARTS;
  //num_ = root["num"] > NUM_CHARTS ? NUM_CHARTS : = root["num"];
  if ( root["num"] > NUM_CHARTS) {
    Serial.print("ERROR NUM_CHARTS:");
    Serial.println(num_, DEC);
  }

  sp_ws8211 = root.containsKey("sp") ? root["sp"] : 10;
  //sp_ws8211 = ws8211_loop == false ? 0 : sp_ws8211 ;
  duration = root.containsKey("dr") ? root["dr"] : 255;
  fade = root.containsKey("fd") ? root["fd"] : 200;
  bright = root.containsKey("br") ? root["br"] : 255;
  reverse_set = root.containsKey("r") ? root["r"] : 0;
  fadetype = root.containsKey("fdt") ? root["fdt"] : 3;
  inv = root.containsKey("inv") ? root["inv"] : 0;
  //reverse_set = root["r"];
  if (!reverse_set) {
    //Previous_set = String(json);
    //Previous_set_char=
    //Serial.println("set return to this");
  }


  FastLED.setBrightness(bright);
  //}
  for (char i = 0; i < num_ws8211; i++) {
    unsigned char from_ = root["from"][i];//??
    unsigned char to_ = root["to"][i];//? ????
    unsigned char type_ = root["type"][i];//???
    unsigned char dir_ = root["dir"][i];//???????????
    unsigned char col_ = root["col"][i];//????
    unsigned char white_col_ = root["wh"][i];//????????????? ??????
    unsigned char br_ = root["br_"][i];//???????

    white_ws8211[i] = white_col_ == 0 ? 255 : white_col_;
    br__ws8211[i] = br_ == 0 ? 192 : br_;
    //Serial.print("white_col:");
    //Serial.println(white_ws8211[i], DEC);
    pos[i] = dir_ == 1 ? to_ : from_;
    //if (from_ <= NUM_LEDS) {
    from_ws8211[i] = from_ <= NUM_LEDS ? from_ : NUM_LEDS ;
    if ( from_ > NUM_LEDS) {
      Serial.print("ERROR from_:");
      Serial.println(from_, DEC);
    }
    //}
    //if (to_ws8211[i] <= NUM_LEDS) {
    to_ws8211[i] = to_ <= NUM_LEDS ? to_ : NUM_LEDS;
    if ( to_ > NUM_LEDS) {
      Serial.print("ERROR to_:");
      Serial.println(to_, DEC);
    }
    //}
    type_ws8211[i] = type_;
    dir_ws8211[i] = dir_;
    col_ws8211[i] = col_;
    //white_white_col_[i]=white_col_;
    /*
        Serial.print("from_ws8211:");
        Serial.println(from_, DEC);
        Serial.print("to_:");
        Serial.println(to_, DEC);
        Serial.print("type_:");
        Serial.println(type_, DEC);
        Serial.print("dir_:");
        Serial.println(dir_, DEC);
        Serial.print("col_ws8211:");
        Serial.println(col_ws8211[i], DEC);
    */
  }
  if (!ws8211_loop) {
    loop_ws2811();
    //delay_loop_8211 = 2;
    //ws8211_loop = true;
  }
  return true;
}
uint8_t one_min, one_hour;//, new_value_count;


void one_sec() {
#if defined(timerAlarm)
  if (timer_alarm_action_switch) {
    if (timer_alarm_action < timer_alarm_action_max) {
      timer_alarm_action++;
    } else {
      timer_alarm_action_switch = 0;
      timer_alarm_action = 0;
    }
  }
#endif
  if ((duration > 0) && (duration != 255)) {
    running_led = true;
    //    alarm_is_active[idWidget][i] = true;
    //Serial.print("Duration:");
    //Serial.println(duration, DEC);
    duration--;
  } else if (duration == 0)
  {
    running_led = false;
    if (reverse_set) {
      String DataLoad_8211 = readCommonFiletoJson("ws8211/0");
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
      LoadData(buffer);
      //Serial.println("Load reverse set:");
      //if (Previous_set.indexOf("\"r\":1") == -1) {
      //LoadData(Previous_set.c_str());
      //reverse_set = 0;
      // } else {
      //  reverse_set = 0;
      // }
    }
    //duration=255;
  }
  //  one_sec++;
  one_min++;

  /*
    new_value_count++;

    if (new_value_count > 1) {
    //new_value_count = 0;
    get_new_pin_value_ = true;
    }
    if (new_value_count > 9) {
    new_value_count = 0;
    get_new_pin_value_ = false;
    }
  */
  if (one_min >= 60) {

    one_min = 0;
    one_hour++;
    //check_for_license();
  }
  if (one_hour >= 24) {

  }
  if (delay_loop_8211 > 1) {
    delay_loop_8211--;
  }
}
void check_for_license() {
  if (one_hour > 1) {
    // license = getEEPROM_char(0);
    if (license != 1) {
      FileDelete("pin_setup.txt");
    }
  }
}
void load_pattern()
{

  // a colored dot sweeping back and forth, with fading trails

  switch (fadetype) {
    case 0:
      nscale8_video( leds, NUM_LEDS, fade);
      break;
    case 1:
      fade_video( leds, NUM_LEDS, fade);
      break;
    case 2:
      fadeLightBy( leds, NUM_LEDS, fade);
      break;
    case 3:
      fadeToBlackBy( leds, NUM_LEDS, fade);
      break;
    case 4:
      fade_raw( leds, NUM_LEDS, fade);
      break;
    case 5:
      //nscale8_raw( leds, NUM_LEDS, fade);
      break;
    case 6:
      nscale8( leds, NUM_LEDS, fade);
      break;
    case 7:
      //fadeUsingColor( leds, NUM_LEDS, fade);
      break;
    case 8:
      blur1d( leds, NUM_LEDS, fade);
      break;

  }

  //fade_video(leds, NUM_LEDS, fade);//??? ??????, ??? ????????)

  if (!running_led) return;
  //Serial.println(divide1,DEC);

  for (char i = 0; i < num_ws8211; i++) {

    if ( col_ws8211[i] != 0) {
      gHue = col_ws8211[i];
    }

    char mid = (to_ws8211[i] - from_ws8211[i]) / 2;
    char maxLight = 192 / mid;
    char i_on = 0, i_off = 0;
    char i_div = 1;
    char divide1 = (to_ws8211[i] - from_ws8211[i]) / 3;

    switch (type_ws8211[i]) {
      case 0://? ????? ???????
        leds[pos[i]] += CHSV( gHue, white_ws8211[i], br__ws8211[i]);//
        //fill_solid(leds, NUM_LEDS, CHSV(hue, white_ws8211[i], 192));
        //leds[pos[i]].maximizeBrightness(1);
        pos[i] = reverseDirection(i);
        //leds[i].setRGB(0,255,250);  // Set Color HERE!!!
        //leds[i].fadeLightBy(brightness);
        //
        break;
      case 1://? ???? ??????
        // case 5:
        pos[i] = reverseDirection(i);
        leds[pos[i]] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//???????? ?????

        unsigned char  calc_pos;
        calc_pos = (to_ws8211[i] - pos[i] + from_ws8211[i]);
        leds[calc_pos] = CHSV( gHue, white_ws8211[i], br__ws8211[i]); //???????? ?????
        break;
      case 2://????????? ? ?????
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          leds[i1] = CHSV(  gHue, white_ws8211[i], br__ws8211[i]);//??????? ????? ?????
        }
        break;
      case 3://???????? ? ?????
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (i1 < mid) {
            leds[i1] = CHSV(  gHue, white_ws8211[i], maxLight * i1);
          }
          else {
            leds[i1] = CHSV(  gHue, white_ws8211[i], maxLight * (to_ws8211[i] - i1));
          }
        }
        break;
      case 4://?? ????????
        leds[random(from_ws8211[i], to_ws8211[i])] += CHSV( gHue, white_ws8211[i], br__ws8211[i]); //???????? ?????
        break;
      case 5://??????? ????? ????
        if (pos[i] >= to_ws8211[i]) {
          dir_ws8211[i] = 1;
        } else if (pos[i] <= from_ws8211[i]) {
          dir_ws8211[i] = 0;
        }
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (pos[i] % 2 == 0) {
            if ( (i1 % 2) == 0) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//???????? ?????
            }
          } else {
            if ( (i1 % 2) != 0) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//???????? ?????
            }
          }
        }
        break;
      case 6://??????? ?? ??? ?????
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (i1 < (divide1 * i_div) + from_ws8211[i]) {
            if ((i_div & 0x01) == (pos[i] & 0x01)) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);
            }
          } else {
            i_div++;
          }
        }
        break;
      case 7:
        rainbow();
        break;
      case 8:
        rainbowWithGlitter();
        break;
      case 9:
        confetti();
        break;
      case 10:
        sinelon();
        break;
      case 11:
        bpm();
        break;
      case 12:
        juggle();
        break;
      case 13://glow_to_max
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {

          unsigned char bright_pos = pos[i] * (255 / (to_ws8211[i] - from_ws8211[i]));
          leds[i1] = CHSV( 0, 0, bright_pos);//???????? ?????
        }
        break;
      case 14://????????
        type_ws8211[i] = random(0, 13);
        break;
      case 15://???????????

        break;
    }
  }


  //Serial.println(pos[i]);

  //leds[num_leds_fade - pos] += CHSV( gHue, white_ws8211[i], br__ws8211[i]);

    if (inv) {
    for ( unsigned char i = 0; i < NUM_LEDS; i++) { //9948
      leds[i] = CHSV(0,0,255)-leds[i];    }
  }
  
}
unsigned char reverseDirection(unsigned char i) {

  switch (dir_ws8211[i]) {
    case 0:
      pos[i] = pos[i] >= to_ws8211[i] ? from_ws8211[i] : pos[i];
      break;
    case 1:
      pos[i] = pos[i] <= from_ws8211[i] ?  to_ws8211[i] : pos[i];
      break;
  }
  return  pos[i];
}
void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  //addGlitter(80);
}
/*
  void addGlitter(fract8 chanceOfGlitter)
  {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
  }
*/
void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}



void loop_ws2811()
{
  // Call the current pattern function once, updating the 'leds' array

  // send the 'leds' array out to the actual LED strip
  //gPatterns[gCurrentPatternNumber]();
  load_pattern();

  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(sp_ws8211);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    //nextPattern();  // change patterns periodically
  }

  //unsigned char one_period_speed = 2;//((sp_ws8211 * 16) / (192 / 2));
  if (millis() > (sp_ws8211)+ millis_strart) {
    for (char i = 0; i < num_ws8211; i++) {
      if (pos[0] == 0) {
        //
      }
      switch (dir_ws8211[i]) {
        case 0:
          pos[i] = pos[i] < to_ws8211[i] ?  pos[i] + 1 : to_ws8211[i];
          break;
        case 1:
          pos[i] =  pos[i] > from_ws8211[i] ? pos[i] - 1 : from_ws8211[i];
          break;
        case 3:
          pos[i] = random16( from_ws8211[i], to_ws8211[i]);
          break;
        case 4:
          pos[i] = 0;
          break;
      }

    }
    millis_strart = GET_MILLIS();
  }


  if (delay_loop_8211 == 1) {
    ws8211_loop = false;
  }
}
#endif



/*
   IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
   An IR detector/demodulator must be connected to the input RECV_PIN.
   Version 0.1 Sept, 2015
   Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
*/
#if defined(ir_code)

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define MIN_UNKNOWN_SIZE 12
#define CAPTURE_BUFFER_SIZE 250
//#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
// e.g. Kelvinator
// A value this large may swallow repeats of some protocols
//#else  // DECODE_AC
//#define TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
//#endif  // DECODE_AC

//???????? ??? ?????????????
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);//ON!!!
//IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);//ON!!!


uint64_t buff1 = 0;
uint64_t overflow = -1;

const char IRCodeString_numbers_array = 5;
char IRCodeId_numbers;
char IRCodeString[IRCodeString_numbers_array][50];




decode_results results;

void setup_IR()
{
  if (RECV_PIN != 255) {
    irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
    Serial.println("Setup IR");
    irrecv.enableIRIn(); // Start the receiver
    updateIR();
  } else {
    //Serial.println("Setup IR Fail RECV_PIN =-1");
  }
  if (SEND_PIN != -1) {
  }
}
void updateIR() {
  File irJson = SPIFFS.open("/IRButtons.txt", "r");
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed

  DeserializationError error = deserializeJson(jsonDocument, irJson);
  if (error) {
    Serial.print(F("deserializeJson() failed with code updateIR"));
    Serial.println(error.c_str());
    return;
  }

  unsigned int numberChosed = jsonDocument["num"];
  if (numberChosed > IRCodeString_numbers_array) {
    numberChosed = IRCodeString_numbers_array;
  }
  IRCodeId_numbers = numberChosed;

  for (char i = 0; i < numberChosed; i++) {
    const char* IRCode = jsonDocument["code"][i].as<const char*>();
    if (IRCode) {
      strncpy(IRCodeString[i], IRCode, sizeof(IRCodeString[i]) - 1);
      IRCodeString[i][sizeof(IRCodeString[i]) - 1] = '\0'; // Ensure null-termination
    }
  }
}


uint64_t StrToHex(const char* str)
{
  return (uint64_t) strtoul(str, 0, 16);
}
long long toLongLong(String x) {
  unsigned long long y = 0;
  for (int i = 0; i < x.length(); i++) {
    char c = x.charAt(i);
    if (c < '0' || c > '9') break;
    y *= 10;
    y += (c - '0');
  }
  return y;
}
void send_IR_code(const char* full_code_char) {
  String full_code = String(full_code_char);
  if (full_code.length() < 2) {//????? ?????? ????
//    String jsonSend = readCommonFiletoJson("IrRaw_Code" + full_code);
    File jsonSend = SPIFFS.open("/IrRaw_Code" + full_code + ".txt", "r");
    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, jsonSend);
    if (error) {
      Serial.print(F("deserializeJson() failed with code send_IR_code"));
      Serial.println(error.c_str());
      return;
    }
    int codeLen = jsonDocument["len"];
    if (codeLen > 250) return;
    uint16_t Signal_ON_0[250];
    for (int i = 0; i < codeLen; i++) {
      Signal_ON_0[i] = jsonDocument["c"][i];
      // Serial.print(Signal_ON_0[i]);
    }
    irsend.sendRaw(Signal_ON_0, codeLen, 38);
  } else {
    irsend.sendNEC(StrToHex(full_code_char), 32);
  }
}

void send_IR(char ButtonNumber) {

  if (ButtonNumber != char(-1)) {
    if (IrButtonID[ButtonNumber] != 255) {
      Serial.println("send_IR:" + String(ButtonNumber, DEC));
      if (IrButtonID[ButtonNumber] < IRCodeString_numbers_array) {
        //int code = IRCodeString[IrButtonID[ButtonNumber]];
        Serial.println("SEND IR IRCodeString:" + String(IRCodeString[IrButtonID[ButtonNumber]]) + "IrButtonID:" + String(IrButtonID[ButtonNumber], DEC));
        /*   uint64_t number = toLongLong(IRCodeString[IrButtonID[ButtonNumber]]);
           unsigned long long1 = (unsigned long)((number & 0xFFFF0000) >> 16 );
           unsigned long long2 = (unsigned long)((number & 0x0000FFFF));
        */

        send_IR_code(IRCodeString[IrButtonID[ButtonNumber]]);
      } else {
        Serial.println("Fail IrButtonID[ButtonNumber]" + String(IrButtonID[ButtonNumber], DEC));
      }
    }
  }
}


void check_code_IR(String codeIR) {
  for (char i = 0; i < IRCodeId_numbers; i++) {
    //if (IRCodeString[i] == codeIR) {//?????????? ???????
    if (strcmp(IRCodeString[i], codeIR.c_str()) == 0) {
      for (char i1 = 0; i1 < nWidgets; i1++) {
        if (IrButtonID[i1] == i) {
          Serial.println("FIND IR:" + String(descr[i1]) + " IrButtonID[i1]:" + String(IrButtonID[i1], DEC) + " i:" + String(i, DEC));
          stat[i1] ^= 1;
          save_stat_void();
          digitalWrite(pin[i1], stat[i1] );
          //stat[i1] = new_state;
          delay(300);
        }
      }
    }
  }
}
/*
  void loop_IR5() {
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    Serial.println((results.rawlen));
    //Serial.println((results.rawbuf[0]));
    for (int i = 0; i < results.rawlen; i++) {
      Serial.print((results.rawbuf[i]));
      Serial.print("\t");

    }
    Serial.println("");  // Blank line between entries
    yield();  // Feed the WDT (again)
  }
  }
*/
void loop_IR() {
  if (irrecv.decode(&results)) {
    String codeIR;
    unsigned char len = results.rawlen;
    if ((len > 100) && ((Page_IR_opened))) {
      /////ITs RAW/////////
      //unsigned int Signal_ON_0[250];
      String sendJSON;
      sendJSON = "{\"raw\":\"true\",\"len\":";
      sendJSON += results.rawlen;
      sendJSON += ",\"c\":[";
      for (uint16_t i = 1; i < results.rawlen; i++) {
        uint32_t usecs;
        for (usecs = results.rawbuf[i] * RAWTICK;
             usecs > UINT16_MAX;
             usecs -= UINT16_MAX) {
          sendJSON += uint64ToString(UINT16_MAX);
          if (i % 2)
            sendJSON += ",0,";
          else
            sendJSON += ",0,";
        }
        sendJSON += uint64ToString(usecs, 10);
        if (i < results.rawlen - 1)
          sendJSON += ", ";  // ',' not needed on the last one
        if (i % 2 == 0)  sendJSON += " ";  // Extra if it was even.
      }
      sendJSON += "]}";
      //json["c"] = Signal_ON_0;
      Serial.println(sendJSON);
      //saveCommonFiletoJson("IR_reciever", sendJSON, 1);
      yield();  // Feed the WDT (again)
      server.send(200, "text/plain", sendJSON);
      yield();  // Feed the WDT (again)
      Page_IR_opened = false;
      //irrecv.resume();
    } if (len < 100) {
      char charBuff1[21];
      sprintf(charBuff1, "%X", results.value);
      //saveCommonFiletoJson("IR_reciever", charBuff1, 1);
      Serial.println("short code:" + String(charBuff1));
      yield();  // Feed the WDT (again)
      buff1 = 0;
      codeIR = String(charBuff1);
      if (!Page_IR_opened) {
#if defined(ir_code)
        check_code_IR(codeIR);
#endif
      }
      else {
        server.send(200, "text/plain", codeIR);
        Page_IR_opened = false;
      }
      //irrecv.resume();
    }

    // Receive the next value IRRecieve
    //Serial.println(results.rawlen);
    //    yield();  // Feed the WDT (again)
  }
}
/*
  void loop_IR2() {

  if (!IR_loop) {//???? ???????? IR, ??
    //last_millis = millis();//????????? ????????? ?????
  }
  // if (WebSocketConnected) {
  // else if (IR_loop) {
  if (irrecv.decode(&results)) {
    String codeIR;
    if ((results.value != overflow) && (buff1 != 0)) {
      if (buff1 != results.value) {
        char charBuff1[21];
        sprintf(charBuff1, "%X", buff1);
        char charBuff[21];
        sprintf(charBuff, "%X", results.value);
        Serial.println(charBuff1);
        Serial.println(charBuff);
        codeIR = String(charBuff1);
        buff1 = 0;
        if (!Page_IR_opened) {
          check_code_IR(codeIR);
        }
        else {
          server.send(200, "text/plain", codeIR);
        }
      }
      else {
        char charBuff1[21];
        sprintf(charBuff1, "%X", buff1);
        Serial.println(charBuff1);
        buff1 = 0;
      }
      buff1 = 0;
    }
    if (buff1 == 0) {
      buff1 = results.value;
      irrecv.resume();
    }
    if (results.value == overflow) {
      //Serial.print("overflow");
      char charBuff1[21];
      sprintf(charBuff1, "%X", buff1);
      buff1 = 0;
      Serial.println(charBuff1);
    }

    //Serial.print("Str:");

    //char buff[21];
    // copy to buffer
    //sprintf(buff, "%X", results.value);
    //Serial.println(buff);
    irrecv.resume();  // Receive the next value
  }
  // }
  }
*/

#endif


/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

String getHttp(String request) {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    http.begin(wclient, "http://" + request); //?????? HTTP//"http://api.2ip.ua/geo.json?ip="
    Serial.println(request);
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();

    if (httpCode > 0) {

      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
        return payload;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      return ("fail");
    }

    http.end();


    //delay(20000);
  }
  else {
    return ("fail");
  }
  return ("fail");
}




