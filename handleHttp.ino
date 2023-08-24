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
  Page += F(
            "<p><br><a href='/home.htm'><button>домашняя страница</button></a></p>"
            "<p><br><a href='/wifi'><button>подключение к Wifi</button></a></p>"
            "<p><br><a href='/other_setup'><button>другие настройки</button></a></p>"
            "<p><br><a href='/pin_setup'><button>pin setup</button></a></p>"
            "<p><br><a href='/IR_setup'><button>ИК настройки</button></a></p>"
            "<p><br><a href='/condition'><button>условия</button></a></p>"
            "<p><br><a href='/function?json={reboot:1}'><button>перезапуск</button></a></p>"
          );


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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["ssid"] = ssid;
  json["WiFilocalIP"] = toStringIp(WiFi.localIP());
  json["softAP"] = softAP_ssid;
  json["WiFisoftAPIP"] = toStringIp(WiFi.softAPIP());


  JsonArray& scan_array = json.createNestedArray("scan");
  JsonArray& enc_array = json.createNestedArray("enc");
  JsonArray& RSSI_array = json.createNestedArray("RSSI");
  int n = WiFi.scanNetworks();
  if (n > 0) {
    json["n"] = n;
    for (int i = 0; i < n; i++) {
      scan_array.add(WiFi.SSID(i));
      enc_array.add(String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? +"Free" : ""));
      RSSI_array.add(WiFi.RSSI(i));
    }
  }
  String buffer;
  json.printTo(buffer);
  server.send(200, "text/json", buffer);
}
void handleWifi() {
  String Page = sendHead();
  Page += F(
            "<h1>Wifi config</h1>");
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
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
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


void save_wifiList(String s, String p) {
  String WifiList = readCommonFiletoJson("wifilist");

  s.toCharArray(ssid, sizeof(ssid) - 1);
  p.toCharArray(password, sizeof(password) - 1);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(WifiList);
  JsonObject& rootjsCreate = jsonBuffer.createObject();

  JsonArray& name_array = rootjsCreate.createNestedArray("name");
  JsonArray& pass_array = rootjsCreate.createNestedArray("pass");

  if (!rootjs.success()) {
    Serial.println("parseObject() pasrse_wifiList failed");
    //rootjsCreate["num"] = 10;
    //rootjsCreate["last"] = 0;

    name_array.add(s);
    pass_array.add(p);

    String buffer;
    rootjsCreate.printTo(buffer);
    //Serial.println(buffer);
    saveCommonFiletoJson("wifilist", buffer, 1);
    return;
  }
  char num = rootjs["name"].size();
  bool  ssid_not_found = true;
  bool write_array = false;
  for (unsigned char i = 0; i < num; i++) {

    char nameWifi[20];
    char passWifi[20];
    strcpy(nameWifi, rootjs["name"][i]);
    strcpy(passWifi, rootjs["pass"][i]);

    //name_array.add(rootjs["name"][i]);

    if ((strcmp (nameWifi, ssid) == 0) && (strcmp (passWifi, password) != 0)) { //совпадают ssid но не совпадает пароль
      Serial.println("update ssid password");
      //need update
      name_array.add(rootjs["name"][i]);
      pass_array.add(password);
      write_array = true;
      //break;
    } else {
      Serial.println("write ssd as previous");
      name_array.add(rootjs["name"][i]);
      pass_array.add(rootjs["pass"][i]);
    }
    
    if (strcmp (nameWifi, ssid) != 0) { //ни разу не совпал ssid
      ssid_not_found = false;
    }

  }
  if (ssid_not_found ) {
    Serial.println("add new ssd");
    name_array.add(ssid);
    pass_array.add(password);
    write_array = true;
  }
  if (write_array) {
    String buffer;
    rootjsCreate.printTo(buffer);
    //Serial.println(buffer);
    saveCommonFiletoJson("wifilist", buffer, 1);
  }
  saveCredentials();
  // }
  //else {
  //  saveCommonFiletoJson("wifilist", "{num : 1, last : 0, name : [" + String(s) + "], pass : [" + String(p) + "]}");
  //}
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
  /*
    WiFi.disconnect();
    if (WiFi.getMode() != WIFI_AP)
    {
    Serial.println("Enable Wifi STA");
    WiFi.mode(WIFI_STA);
    delay(1000);
    }
  */
  Serial.println("scan start");
  char n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (char(n) > 0) {
    String WifiList = readCommonFiletoJson("wifilist");

    DynamicJsonBuffer jsonBuffer;
    JsonObject& rootjs = jsonBuffer.parseObject(WifiList);
    if (!rootjs.success()) {
      Serial.println("parseObject() save_wifiList failed");
      return false;
    }
    for (char i = 0; i < 10; i++) {
      String nameWifi = rootjs["name"][i];
      String passWifi = rootjs["pass"][i];
      //Serial.println(nameWifi);
      for (char in = 0; in < char(n); in++) {
        //Serial.println(WiFi.SSID((in)));
        if (String(WiFi.SSID(in)) == nameWifi) {
          nameWifi.toCharArray(ssid, sizeof(ssid) - 1);
          passWifi.toCharArray(password, sizeof(password) - 1);
          Serial.println(ssid);
          Serial.println(password);
          break;
        }
      }
    }
  }
  return true;
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


