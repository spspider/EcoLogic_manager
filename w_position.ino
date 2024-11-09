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
    http.begin("http://" + request); //запрос HTTP//"http://api.2ip.ua/geo.json?ip="
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

