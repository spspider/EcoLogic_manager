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
    "</head><body>");
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
    "<p><br><a href='/function?data={reboot:1}'><button>reboot</button></a></p>"));

  server.send(200, "text / html", Page);
}
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("/home.htm"), true);
    server.send(302, "text/plain", "");
    server.client().stop();
    return true;
  }
  return false;
}
void handleWifilist() {
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  JsonObject json = jsonDocument.to<JsonObject>();

  json["ssid"] = ssid;
  json["WiFilocalIP"] = toStringIp(WiFi.localIP());
  json["softAP_ssid"] = softAP_ssid;
  json["softAP_password"] = softAP_password;
  json["WiFisoftAPIP"] = toStringIp(WiFi.softAPIP());
  json["wifi_mode"] = wifi_mode;

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

void handleWifiLight() {
  String Page = sendHead();
  Page += F(
    "<h1>Wifi Config</h1>"
    "<style>"
    "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; text-align: center; background: #f9f9f9; color: #333; }"
    "h1 { color: #444; }"
    "#message { margin-top: 20px; font-size: 18px; color: #666; }"
    "</style>"
    "<div id='message'>Waiting...</div>"
    "<script>"
    "setTimeout(() => { window.location.href = '/wififull'; }, 500);"
    "</script>");
  server.send(200, "text/html", Page);
  server.client().stop();  // Stop is needed because we sent no content length
}

void handleWifiFull() {
  String Page = sendHead();
  Page += F(
    "<h1>Wifi Config</h1>"
    "<style>"
    "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background: #f9f9f9; color: #333; }"
    "h1 { text-align: center; color: #444; }"
    "table { width: 90%; margin: 10px auto; border-collapse: collapse; }"
    "th, td { padding: 10px; text-align: left; border: 1px solid #ddd; }"
    "th { background: #f0f0f0; }"
    "input, button { width: calc(100% - 20px); margin: 10px auto; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }"
    "input[type='submit'] { background: #4CAF50; color: white; border: none; cursor: pointer; }"
    "input[type='submit']:hover { background: #45a049; }"
    "a { color: #4CAF50; text-decoration: none; }"
    "a:hover { text-decoration: underline; }"
    "</style>"
    "<script>function populateSSID(ssid) {document.getElementById(\"network\").value = ssid;}</script>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
      "\r\n<br />"
      "<table><tr><th>SoftAP Config</th></tr>"
      "<tr><td>SSID "))
    + String(softAP_ssid) + F("</td></tr>"
                              "<tr><td>IP ")
    + toStringIp(WiFi.softAPIP()) + F("</td></tr>"
                                      "</table>"
                                      "\r\n<br />"
                                      "<table><tr><th>WLAN Config</th></tr>"
                                      "<tr><td>SSID ")
    + String(ssid) + F("</td></tr>"
                       "<tr><td>IP ")
    + toStringIp(WiFi.localIP()) + F("</td></tr>"
                                     "</table>"
                                     "\r\n<br />"
                                     "<table><tr><th>WLAN List (refresh if any missing)</th></tr>");
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td><a href='#' onclick='populateSSID(\"")) + WiFi.SSID(i) + String(F("\")'>")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</a></td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
    "</table>"
    "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to Network:</h4>"
    "<input type='text' id='network' placeholder='Network' name='n'/>"
    "<input type='password' placeholder='Password' name='p'/>"
    "<input type='submit' value='Connect/Disconnect'/>"
    "</form>"
    "<p><a href='/'>Return to Home Page</a></p>"
    "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop();  // Stop is needed because we sent no content length
}

void handleWifiSave() {
  Serial.println("wifi save");
  
  char new_ssid[32];
  char new_password[32];
  server.arg("n").toCharArray(new_ssid, sizeof(new_ssid) - 1);
  server.arg("p").toCharArray(new_password, sizeof(new_password) - 1);
  
  // Only update if SSID is not empty
  if (strlen(new_ssid) > 0) {
    strcpy(ssid, new_ssid);
    // Only update password if provided
    if (strlen(new_password) > 0) {
      strcpy(password, new_password);
    }
  }
  
  if (server.hasArg("mode")) {
    wifi_mode = server.arg("mode").toInt();
  }
  if (server.hasArg("ap_p") && server.arg("ap_p").length() > 0) {
    server.arg("ap_p").toCharArray(softAP_password, sizeof(softAP_password) - 1);
  } else if (strlen(softAP_password) == 0) {
    strcpy(softAP_password, "12345678");
  }

  save_wifiList(ssid, password);

  String Page = "<html><head><script>setTimeout(function(){window.location.href='/function?data={reboot:1}';},2000);</script></head><body><h2>Settings saved. Rebooting...</h2></body></html>";
  server.send(200, "text/html", Page);
}

void save_wifiList(const char *ssid, const char *password) {
  File WifiList = fileSystem->open("/wifilist.txt", "r");

  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, WifiList);
  WifiList.close();

  if (error) {
    Serial.print(F("deserializeJson() save_wifiList failed with code "));
    Serial.println(error.c_str());
  }

  JsonArray name_array = jsonDocument.createNestedArray("name");
  JsonArray pass_array = jsonDocument.createNestedArray("pass");

  unsigned char num = jsonDocument.containsKey("name") ? jsonDocument["name"].size() : 0;
  bool ssid_found = false;

  if (num == 0) {
    jsonDocument["num"] = 1;
  }
  
  for (unsigned char i = 0; i < num; i++) {
    const char* nameWifi = jsonDocument["name"][i];
    const char* passWifi = jsonDocument["pass"][i];

    if (strcmp(nameWifi, ssid) == 0) {
      name_array.add(ssid);
      pass_array.add(password);
      ssid_found = true;
    } else {
      name_array.add(nameWifi);
      pass_array.add(passWifi);
    }
  }

  if (!ssid_found) {
    name_array.add(ssid);
    pass_array.add(password);
  }

  jsonDocument["wifi_mode"] = wifi_mode;
  jsonDocument["softAP_password"] = softAP_password;

  char buffer[512];
  serializeJson(jsonDocument, buffer);
  writeFile(LittleFS, "/wifilist.txt", buffer);
}

/*
  String *read_wifiList(uint8_t index) {
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
  File WifiList = fileSystem->open("/wifilist.txt", "r");

  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, WifiList);
  WifiList.close();
  
  if (error) {
    Serial.print(F("load_ssid_pass deserializeJson() failed load_ssid_pass with code "));
    Serial.println(error.c_str());
    return false;
  }

  const char *nameWifi = jsonDocument["name"][0];
  const char *passWifi = jsonDocument["pass"][0];

  strcpy(ssid, jsonDocument["name"][0]);
  strcpy(password, jsonDocument["pass"][0]);
  
  wifi_mode = jsonDocument.containsKey("wifi_mode") ? jsonDocument["wifi_mode"] : 3;
  
  if (jsonDocument.containsKey("softAP_password")) {
    strcpy(softAP_password, jsonDocument["softAP_password"]);
  }
  
  Serial.println(ssid);
  Serial.println(password);
  Serial.print("WiFi mode: ");
  Serial.println(wifi_mode);

  return true;
}

void handleNotFound() {
  if (captivePortal()) {  // If caprive portal redirect instead of displaying the error page.
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
  for (unsigned int i = 0; i < str.length(); i++) {
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
