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
//char timezone = 2;               // часовой пояс GTM






byte sendEmail(String message) {
  ///////////load email
  File load_other_setup = fileSystem->open("/other_setup.txt", "r");
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
  DeserializationError error = deserializeJson(jsonDocument, load_other_setup);

  if (error) {
    Serial.println("Failed to parse JSON! loadConfig for email");
    return false;
  }
  char smtp_arr[64] = "mail.smtp2go.com";
  // Use int8_t for timezone to avoid ArduinoJson 'char' error
  int8_t timezone = 2;

  const char *buff_smtp_arr = jsonDocument["smtp_arr"];
  if (buff_smtp_arr) {
    snprintf(smtp_arr, sizeof smtp_arr, "%s", buff_smtp_arr);
    smtp_arr[sizeof(smtp_arr) - 1] = '\0';
  }

  short unsigned int smtp_port = jsonDocument["smtp_port"];
  String to_email_addr = jsonDocument["to_email_addr"].as<String>();
  String from_email_addr = jsonDocument["from_email_addr"].as<String>();
  String emaillogin = jsonDocument["emaillogin"].as<String>();
  String password_email = jsonDocument["password_email"].as<String>();

  jsonDocument.containsKey("timezone") ? timezone = jsonDocument["timezone"].as<int8_t>() : timezone = 2;
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
  wclient.print("MAIL FROM: <");  // identify sender
  wclient.print(from_email_addr);
  wclient.println(">");
  if (!eRcv()) return 0;

  // change to recipient address
  Serial.println(F("Sending To"));
  wclient.print("RCPT TO: <");  // identify recipient
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
  wclient.print(device_id);
  wclient.println();

  uint8_t lenMessage = (message.length() / 50) + 1;
  unsigned int FromIndexComma = 0;
  unsigned int toIndexComma = 0;
  /*
    for (uint8_t i = 0; i < 10; i++) {
      FromIndexComma = message.indexOf(",", toIndexComma + 1);
      toIndexComma = message.substring(FromIndexComma + 1, message.length()).indexOf(",", FromIndexComma + 1);
      toIndexComma > 50 ? toIndexComma = 50 : true;
      wclient.println();
      wclient.println(message.substring(FromIndexComma, toIndexComma));
    }
  */
  //String SubstringMessage = message;

  for (uint8_t i = 0; i < lenMessage; i++) {
    wclient.println();
    wclient.println(message.substring((i * 50), (i * 50 + 50)));
    wclient.println();
  }

  //Serial.println(message);


  //    wclient.print(off_time);
  //    wclient.println(" секунд\r\n");

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

byte eRcv() {
  if (!enable_email_sending) return 0;
  
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!wclient.available()) {
    yield(); // Позволяем ESP8266 обрабатывать WiFi
    delay(1);
    loopCount++;

    if (loopCount > 5000) { // Сокращаем до 5 сек
      wclient.stop();
      Serial.println(F("\r\nEmail timeout"));
      return 0;
    }
  }

  respCode = wclient.peek();

  while (wclient.available()) {
    thisByte = wclient.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4') {
    efail();
    return 0;
  }

  return 1;
}


void efail() {
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

  while (wclient.available()) {
    thisByte = wclient.read();
    Serial.write(thisByte);
  }

  wclient.stop();

  Serial.println(F("disconnected"));
}

extern char mqttServerName[60];
extern unsigned int mqttport;
extern char mqttuser[15];
extern char mqttpass[15];
