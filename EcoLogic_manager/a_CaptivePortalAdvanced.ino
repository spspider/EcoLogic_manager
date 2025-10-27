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

bool staReady = false;  // Don't connect right away

uint8_t status = WL_IDLE_STATUS;
uint8_t FreeWIFIid[5];
bool freeWIFIConnected = false;

// Static variables for non-blocking WiFi connection
static unsigned long connectStart = 0;
static bool connecting = false;
static char wifiAttempt = 0;

void connect_as_AccessPoint() {
  delay(1000);
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.disconnect();
  WiFi.persistent(false);  // w/o this a flash write occurs at every boot
  WiFi.mode(WIFI_OFF);     // Prevent use of SDK stored credentials
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (WiFi.softAP(softAP_ssid + WiFi.macAddress(), softAP_password)) {
    Serial.println("SoftAP started successfully.");
  } else {
    Serial.println("Failed to start SoftAP.");
  }

  delay(500);  // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  lastConnectTry = onesec = 0;
}

void captive_setup() {  // starting void
#if defined(USE_DNS_SERVER)
  dnsServer.setTTL(0);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
#endif
  Captive_server_init();
  connect = true;
  server_init();
  server.begin();
}

// Non-blocking WiFi connection
void connectWifi(char ssid_that[32], char password_that[32]) {
  if (!connecting) {
    WiFi.disconnect();
    status = WL_IDLE_STATUS;
    WiFi.mode(WIFI_STA);
    if (use_static_ip && strlen(static_ip) > 0) {
      IPAddress ip, gw, sn;
      ip.fromString(static_ip);
      gw.fromString(gateway);
      sn.fromString(subnet);
      WiFi.config(ip, gw, sn);
      Serial.println("Using static IP: " + String(static_ip));
    }
    Serial.println("Connecting as wifi client...");
    Serial.println(ssid_that);
    WiFi.begin(ssid_that, strlen(password_that) > 0 ? password_that : nullptr);
    connecting = true;
    connectStart = millis();
    wifiAttempt = 0;
    return;
  }
  
  if (connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      connecting = false;
      Serial.println("\nWiFi connected!");
    } else if (millis() - connectStart > 500) {
      wifiAttempt++;
      connectStart = millis();
      Serial.print(".");
      if (wifiAttempt >= 50) {
        connecting = false;
        connect_as_AccessPoint();
      }
    }
  }
}

void relayRouter() {
  if (router != 255) {
    pinMode(router, OUTPUT);
    digitalWrite(router, digitalRead(router) ^ 1);
    Serial.println("Router relay state:" + String(digitalRead(router), DEC));
  }
}

void captive_loop() {
  if (connect) {
    status = WL_IDLE_STATUS;
    connect = false;
    Serial.println("connect: true");
    if (load_ssid_pass()) {
      Serial.println("SSID loaded");
      connectWifi(ssid, password);
    } else {
      connect_as_AccessPoint();
    }
  }
  
  // Continue WiFi connection process if in progress
  if (connecting) {
    connectWifi(ssid, password);
  }
  
#if defined(USE_PUBSUBCLIENT)
  if (try_MQTT_access) {
    if (IOT_Manager_loop) {
      loop_IOTManager();
    }
  }
#endif
  uint8_t s = (uint8_t)WiFi.status();
  if (onesec >= lastConnectTry + 30) {
    if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() == 0)) {
      WiFi.disconnect();
      Serial.println("Connecting as Wifi client due AP not connected");
      lastConnectTry = onesec = 0;
      captive_setup();
#if defined(USE_DNS_SERVER)
      dnsServer.enableForwarder(myHostname, WiFi.dnsIP(0));
      if (dnsServer.isDNSSet()) {
        CONSOLE_PRINTF("  Forward other lookups to DNS: %s\r\n", dnsServer.getDNS().toString().c_str());
      }
#endif
      return;
    } else if ((wifi_softap_get_station_num() == 0) && ((WiFi.status() == WL_DISCONNECTED) || (WiFi.status() == WL_IDLE_STATUS) || (WiFi.status() == WL_NO_SSID_AVAIL))) {
      WiFi.disconnect();
      Serial.println("Connecting as AP, due WL_DISCONNECTED");
      lastConnectTry = onesec = 0;
      connect_as_AccessPoint();
      relayRouter();
      internet = false;
      try_MQTT_access = false;
      return;
    } else if (!internet && geo_enable) {
      Serial.println("Status WIFI: " + String(WiFi.status()));
    } else if ((WiFi.getMode() == WIFI_AP) && (wifi_softap_get_station_num() != 0)) {
      Serial.print("stations connected as AP:");
      Serial.println(wifi_softap_get_station_num());
    }
    lastConnectTry = onesec = 0;
  }
  if (status != s) {  // WLAN status change
    Serial.print("Status: ");
    Serial.println(s);
    status = s;
    if (s == WL_CONNECTED) {
      try_MQTT_access = true;
      /* Just connected to WLAN */
      Serial.println("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      server.send(200, "text/plain", toStringIp(WiFi.localIP()));
      save_wifiList(ssid, password);
      httpUpdater.setup(&server);  // setupOTA

      // Setup MDNS responder
      if (!MDNS.begin(myHostname)) {
        Serial.println("Error setting up MDNS responder!");
      } else {
        Serial.println("mDNS responder started");
        MDNS.addService("http", "tcp", 80);
      }
      uploadConfig_ecologicclient();
      if (geo_enable)
        sendLocationData();
#if defined(timerAlarm)
      setup_alarm();
#endif
#ifdef use_telegram
      setup_telegram();
#endif
    } else if (s == WL_NO_SSID_AVAIL) {
      internet = false;
      try_MQTT_access = false;
      WiFi.disconnect();
      Serial.println("Connecting as AP, due WL_NO_SSID_AVAIL");
      connect_as_AccessPoint();
      relayRouter();
    }
#if defined(timerAlarm)
    setup_alarm();
#endif
  }
  if (s == WL_CONNECTED) {
    MDNS.update();
  }

#if defined(USE_DNS_SERVER)
  dnsServer.processNextRequest();
#endif
  server.handleClient();

  // WiFi connection event
  if (WiFi.status() == WL_CONNECTED) {
#if defined(USE_PICOMQTT)
    static bool mqtt_started = false;
    if (!mqtt_started) {
      setup_picoMqtt();
      mqtt_started = true;
    }
#endif

  }
}

// Non-blocking geolocation with rate limiting
void sendLocationData() {
  if (!enable_geo_location) return;
  
  static unsigned long lastGeoCheck = 0;
  if (millis() - lastGeoCheck > 300000) { // Проверка каждые 5 мин
    String pos = getHttp("api.2ip.ua/geo.json?ip=");
    if (pos != "fail") {
      internet = true;
      saveCommonFiletoJson("ip_gps", pos, 1);
      if (enable_email_sending) {
        sendEmail(pos);
      }
    } else {
      internet = false;
      Serial.println("Internet connection failed");
    }
    lastGeoCheck = millis();
  }
}