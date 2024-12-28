
static bool fsOK;
String unsupportedFiles = String();
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
  deleteRecursive(path);
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
  uint8_t Topic_is = jsonDocument["t"].as<uint8_t>(); //
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
  // server.on("/mode", []()
  //           {
  //   String mode = (WiFi.getMode() == WIFI_STA) ? "STA" : "AP";
  //   String jsonResponse = "{\"mode\":\"" + mode + "\"}";
  //   server.send(200, "application/json", jsonResponse); });
  server.on("/wifi", []()
            {
    if (WiFi.getMode() == WIFI_STA) {//как клиент
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

  // server.serveStatic("/font", SPIFFS, "/font", "max-age=86400");
  // server.serveStatic("/js", SPIFFS, "/js", "max-age=86400");
  // server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");

  // server.onNotFound ( handleNotFound );
  Serial.println("HTTP server started");
}
