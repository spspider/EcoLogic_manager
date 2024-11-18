File fsUploadFile;

// format bytes
String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename)
{
  if (server.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path)
{
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if ((path.endsWith("bootstrap.min.css")) && (WiFi.getMode() != WIFI_STA))
  {
    Serial.println("Bootstrap requested");
    path = "";
    return false;
  }
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
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    if (contentType != "text/plain")
    {
      // server.sendHeader("Cache-Control", "public, max-age=86400, must-revalidate");
      // server.sendHeader("Pragma", "public");
      // server.sendHeader("Expires", "86400");
    }

    size_t sent = server.streamFile(file, contentType);
    server.client().stop();
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload()
{
  if (server.uri() != "/edit")
    return;
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: ");
    DBG_OUTPUT_PORT.println(filename);
    if (!filename.startsWith("/"))
      filename = "/" + filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: ");
    DBG_OUTPUT_PORT.println(upload.totalSize);
    server.send(200, "text/plain", "ok");
  }
}

void handleFileDelete()
{
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
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
void handleFileCreate()
{
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}
void handle_sendEmail()
{
  // String email_txt = server.arg("Email"); // Получаем значение ssdp из запроса сохраняем в глобальной переменной
  if (sendEmail(server.arg("Email")))
  {
    server.send(200, "text/plain", "send OK"); // отправляем ответ о выполнении
  }
  else
  {
    server.send(200, "text/plain", "Fail"); // отправляем ответ о выполнении
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
  if (!server.hasArg("dir"))
  {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next())
  {
    File entry = dir.openFile("r");
    if (output != "[")
      output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}

void handle_saveIR()
{
  String IRjson = server.arg("IR");
  saveCommonFiletoJson("IRButtons", IRjson, 1);
  updateIR();
}

void setup_FS(void)
{

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }

  // SERVER INIT
  // list directory
}
// void handleAJAX()
// {
//   DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
//   DeserializationError error = deserializeJson(jsonDocument, server.arg("json"));
//   if (error)
//   {
//     Serial.print(F("deserializeJson() failed with handleAJAX code "));
//     Serial.println(error.c_str());
//     return;
//   }
//   char Topic_is = jsonDocument["t"];
//   int newValue = jsonDocument["v"];
//   callback_socket(Topic_is, newValue);
// }
void handleAJAX()
{
  // Extract JSON from the server argument
  const char *json = server.arg("json").c_str();

  // Parse "t" value (Topic_is)
  uint8_t Topic_is = 0;
  const char *tStart = strstr(json, "\"t\":");
  if (tStart)
  {
    tStart += 4; // Move past "\"t\":"
    Topic_is = (uint8_t)atoi(tStart);
  }

  // Parse "v" value (newValue)
  uint16_t newValue = 0;
  const char *vStart = strstr(json, "\"v\":");
  if (vStart)
  {
    vStart += 4; // Move past "\"v\":"
    newValue = (uint16_t)atoi(vStart);
  }

  // Call the callback function with parsed values
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

  if (jsonDocument.containsKey("EncoderIA"))
  {
    no_internet_timer = jsonDocument["rotations"];
    server.send(200, "text/plain", server.arg("json"));
  }

  if (jsonDocument.containsKey("sendIR"))
  {
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    send_IR_code(jsonDocument["sendIR"]);
    server.send(200, "text/plain", server.arg("json"));
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
    server.on("/CommonSave", []() { //получаем методом AJAX включаем
    saveCommonFiletoJson(server.arg("fileName"), server.arg("json"));
    });
  */
  // server.on("/CommonDelete", HTTP_DELETE, handleFileDelete);
  server.on("/IR_setup", []()
            {

    //Serial.println("IR true");
    Page_IR_opened = false;
    handleFileRead("/IR_setup.htm"); });

  server.on("/WaitIR", []() { // получаем методом AJAX включаем IR
    Page_IR_opened = true;
  });
  server.on("/open", HTTP_GET, handleOpenLockForTime);
  /*
    server.on("/SaveIR", []() {//!!!!!!!!!!!!!!!!!!можно переделать
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
  // server.on("/pin_setup.txt", handle_ConfigJSON_pinSetup); // формирование configs.json страницы для передачи данных в web интерфейс
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

  DBG_OUTPUT_PORT.println("HTTP server started");
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
    Page += F("<p>замок открыт на:");
    Page += (server.arg("t"));
    Page += F("минут</p>");
  }
  else
  {
    // lock_door();
    Page += F("<h1>замок закрыт</h1>");
  }
  Page += F("<p><br><a href='/open?t=0'><button>закрыть</button></a></p>");
  server.send(200, "text/html", Page);
  Serial.println(countdown_lock, DEC);
}
void Captive_server_init()
{
  // server.on("/setup", handleRoot);
  //   server.on("/wifi", handleWifi);

  server.on("/wifi", []()
            {
    if (WiFi.getMode() == WIFI_STA) {//как клиент
      handleFileRead("/wifi_setup.htm");
    } else {
      handleWifi();
    } });
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
