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


void save_wifiList(String s, String p) {
  File WifiList = SPIFFS.open("/wifilist.txt", "r");

  s.toCharArray(ssid, sizeof(ssid) - 1);
  p.toCharArray(password, sizeof(password) - 1);

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
  for (uint8_t i = 0; i < num; i++)
  {
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


