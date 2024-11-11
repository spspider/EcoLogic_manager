

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
/*
  char *softAP_ssid = "ESP_ap";
  char *softAP_password = "12345678";
  char ssid[32] = "";
  char password[32] = "";

*/
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

//unsigned char softap_stations_cnt;
//struct station_info *stat_info;
struct ip_addr *IPaddress;
uint32 uintaddress;

// Web server
//ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned int lastConnectTry = 0;
/** Current WLAN status */
uint8_t status = WL_IDLE_STATUS;
//String scannedFREEWIFI_False[5];
uint8_t FreeWIFIid[5];
bool freeWIFIConnected = false;
unsigned int buff_ESP_busy;
bool ESP_busy;
void connect_as_AccessPoint() {
  delay(1000);
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid + WiFi.macAddress(), softAP_password);
  delay( 500 ); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());


}
void captive_setup() {

  //  delete server;

  ESP_busy = true;

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  server_init();
  Captive_server_init();
  if ( load_ssid_pass()) {
    connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
    if (connect) {
      try_MQTT_access = true;
    }
    //connectWifi();
  }
  else {

    try_MQTT_access = false;
    connect_as_AccessPoint();
  }
  server.begin();
}

void connectWifi(char ssid_that[32], char password_that[32]) {
  WiFi.disconnect();
  status = WL_IDLE_STATUS;//обнуляем статус, для подключения
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println("Connecting as wifi client...");
  Serial.println(ssid_that);
  Serial.println(password_that);
  Serial.print("Wifi status:");
  Serial.println(WiFi.status());
  delay(100);
  if (strlen(password_that) > 0) {
    WiFi.begin ( ssid_that, password_that );
  } else {
    WiFi.begin (ssid_that);
  }
  int whileConnected = 0;
  status = WL_IDLE_STATUS;//обнуляем статус, для подключения
  char waitWhile = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    waitWhile++;
    if (waitWhile > 50) {
      break;
    }
  }
  int connRes = WiFi.waitForConnectResult();
  Serial.print ( "connRes: " );
  Serial.println ( connRes );
  if (connRes != 3) {
    Serial.print ( "will write access point due connection result is not 3" );
    connect_as_AccessPoint();
  }

}
void try_connect_free() {
  Serial.println("!!!scan for FREE WIFI!!!");
  uint8_t n = WiFi.scanNetworks();//сканируем Wifi сети
  if (n > 0) {//если сетей больше одной
    uint8_t number = 0;
    for (uint8_t i = 0; i < n; i++) {
      if (WiFi.encryptionType(i) == ENC_TYPE_NONE)  {//если эта сеть доступна
        FreeWIFIid[number] = i;
        number++;
      }
    }
    if (number > 0) {
      uint8_t ssid_number = random(number - 1);
      //Serial.println("random:" + String(ssid_number));
      WiFi.SSID(FreeWIFIid[ssid_number]).toCharArray(ssid, sizeof(ssid) - 1);
      strncpy( password, "\0", sizeof(password) - 1);
      lastConnectTry = onesec;
      connect = false;
      connectWifi(ssid, password);
      freeWIFIConnected = true;
    }
    else {
      Serial.println("no Free Wifi");
    }
  }
}
void relayRouter() {
  //сперва нужно послать http запрос на наличие интернета
  if (router != 255) {//если подключение через реле может реле инвертированно
    pinMode(router, OUTPUT);
    digitalWrite(router, digitalRead(router) ^ 1); //подключаем роутер
    Serial.println("реле роутера:" + String(router) + "состояние:" + String(digitalRead(router), DEC));
  }
}
void captive_loop() {

  if (connect) {//если есть SSID в EEPROM
    status = WL_IDLE_STATUS;//обнуляем статус, для подключения
    Serial.println ( "Connect requested" );
    connect = false;
    connectWifi(ssid, password);
    lastConnectTry = onesec;
  }
  if (try_MQTT_access) {
    if (IOT_Manager_loop) {
#if defined(pubClient)
      loop_IOTMAnager();
#endif
    }
  }
  if ((!try_MQTT_access) && (WiFi.status() == WL_CONNECTED) && (WiFi.getMode() == WIFI_STA)) {
    if (onesec > no_internet_timer + 300 ) {//пробовать подключится каждые30 сек
      try_MQTT_access = true; //пробуем подключить MQTT
      Serial.println("Connect again");
      no_internet_timer = onesec;
    }
  }
  {

    uint8_t s = WiFi.status();

    if ( onesec > lastConnectTry + 60 ) {
      if (((s == 0) || (s == 6) )) { //или нет подключения, или подключен как точка доступа
        if (WiFi.getMode() != WIFI_AP ) {//
          WiFi.disconnect();
          Serial.println ( "Wrong connection" );
          lastConnectTry = onesec;
          connect_as_AccessPoint();//должен подключаться если нет подключения
          relayRouter();//если нет вещания, значит нужно переключить роутер
          return;
        }
        else if (((WiFi.getMode() == WIFI_AP)  && (wifi_softap_get_station_num() == 0)) && (wifi_scan)) {
          Serial.println("Connect Creditnails");
          if (load_ssid_pass()) {
            WiFi.disconnect();
            connect = true;
          }
          return;
        }
        else if ((!internet) && (!ESP_busy) && (geo_enable)) {
          Serial.println("Status WIFI:" + String(WiFi.status()));
          try_connect_free();//если нет интрнета в любом случае не зависимо от подключения, но должен возвращаться в исходное состояние точка доступа или клиент
        }

      }
      lastConnectTry = onesec;
    }
    if (ESP_busy) {
      if (onesec > buff_ESP_busy + 30 ) {
        ESP_busy = false;
      }
    } else {
      buff_ESP_busy = onesec;
    }
    if (status != s) { // WLAN status change
      Serial.print ( "Status: " );
      Serial.println ( s );
      status = s;
      if (s == WL_CONNECTED) {

        try_MQTT_access = true;//можно попытаться подключиться к интернету
        /* Just connected to WLAN */
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( ssid );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );
        server.send(200, "text/plain", toStringIp(WiFi.localIP()));
        save_wifiList(String(ssid), String(password));



        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          setup_ota();
          Serial.println("mDNS responder started");
          MDNS.addService("http", "tcp", ipport);
          //////////отправляем местоположение
          if (geo_enable) {
            String pos = getHttp("api.2ip.ua/geo.json?ip=");
            if (pos != "fail") {
              internet = true;
            } else {
              internet = false;
              Serial.println("!!!internet false");
            }

            saveCommonFiletoJson("ip_gps", pos, 1);
            sendEmail(pos);
          }
          //          CheckInternet("worldclockapi.com/api/json/utc/now");
          setup_alarm();
          ///////////
#ifdef use_telegram
      setup_telegram();
#endif

        }

      } else if (s == WL_NO_SSID_AVAIL) {
        internet = false;
        try_MQTT_access = false;
        WiFi.disconnect();
      }
      if (freeWIFIConnected) {
        freeWIFIConnected = false;
        WiFi.disconnect();
        lastConnectTry = 0;
        loadCredentials();
        connect = true;
      }
      setup_alarm();// delete if something wrong with alarm

    }

    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  // Do work:
  //DNS
  //
  //HTTP

  dnsServer.processNextRequest();
  server.handleClient();

}

