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
  if (WiFi.softAP(softAP_ssid + WiFi.macAddress(), softAP_password))
  {
    Serial.println("SoftAP started successfully.");
  }
  else
  {
    Serial.println("Failed to start SoftAP.");
  }
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
  connect = true;
  server_init();
  server.begin();
}

void connectWifi(char ssid_that[32], char password_that[32])
{
  WiFi.disconnect();
  status = WL_IDLE_STATUS; // обнуляем статус, для подключения
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
//   uint8_t n = WiFi.scanNetworks();//сканируем Wifi сети
//   if (n > 0) {//если сетей больше одной
//     uint8_t number = 0;
//     for (uint8_t i = 0; i < n; i++) {
//       if (WiFi.encryptionType(i) == ENC_TYPE_NONE)  {//если эта сеть доступна
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
    if (load_ssid_pass())
    {
      Serial.println("SSID loaded");
    }
    connectWifi(ssid, password);
    lastConnectTry = onesec;
  }
#if defined(pubClient)
  if (try_MQTT_access)
  {
    if (IOT_Manager_loop)
    {
      loop_IOTManager();
    }
  }
#endif
  // if ((!try_MQTT_access) && (WiFi.status() == WL_CONNECTED) && (WiFi.getMode() == WIFI_STA)) {
  //   if (onesec > no_internet_timer + 300 ) {//пробовать подключится каждые30 сек
  //     try_MQTT_access = true; //пробуем подключить MQTT
  //     Serial.println("Connect again");
  //     no_internet_timer = onesec;
  //   }
  // }
  {
    // typedef enum {
    //     WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    //     WL_IDLE_STATUS      = 0,
    //     WL_NO_SSID_AVAIL    = 1,
    //     WL_SCAN_COMPLETED   = 2,
    //     WL_CONNECTED        = 3,
    //     WL_CONNECT_FAILED   = 4,
    //     WL_CONNECTION_LOST  = 5,
    //     WL_DISCONNECTED     = 6
    // } wl_status_t;
    uint8_t s = (uint8_t)WiFi.status();

    if (onesec >= lastConnectTry + 30)
    {
      Serial.print("Wfif status:");
      Serial.println(WiFi.status());
      if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() == 0))
      {
        Serial.println("Connecting as Wifi client due AP not connected");

        connect = true;

#if defined(USE_DNS_SERVER)
        dnsServer.setTTL(600); // 10 minutes
        dnsServer.enableForwarder(myHostname, WiFi.dnsIP(0));
        if (dnsServer.isDNSSet())
        {
          CONSOLE_PRINTF("  Forward other lookups to DNS: %s\r\n", dnsServer.getDNS().toString().c_str());
        }
#endif
      }
      else if ((wifi_softap_get_station_num() == 0) && (WiFi.status() == WL_DISCONNECTED) || ((wifi_softap_get_station_num() == 0) && (WiFi.status() == WL_IDLE_STATUS)))
      {
        WiFi.disconnect();
        Serial.println("Connecting as AP, due WL_DISCONNECTED");
        lastConnectTry = onesec;
        connect_as_AccessPoint();
        relayRouter();
      }
      else if (!internet && geo_enable)
      {
        Serial.println("Status WIFI: " + String(WiFi.status()));
      }
      else if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() != 0))
      {
        Serial.print("stations connected as AP:");
        Serial.println(wifi_softap_get_station_num());
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
        try_MQTT_access = true; // можно попытаться подключиться к интернету
        /* Just connected to WLAN */
        Serial.println("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        server.send(200, "text/plain", toStringIp(WiFi.localIP()));
        save_wifiList(ssid, password);
        httpUpdater.setup(&server); // setupOTA
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
        Serial.println("Connecting as AP, due WL_NO_SSID_AVAIL");
        lastConnectTry = onesec;
        connect_as_AccessPoint();
        relayRouter();
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
