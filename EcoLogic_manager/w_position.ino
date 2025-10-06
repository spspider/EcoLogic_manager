/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

String getHttp(String request) {
  if (!enable_http_requests || (WiFi.status() != WL_CONNECTED)) {
    return "fail";
  }
  
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  http.begin(wclient, "http://" + request);
  http.setTimeout(5000);
  Serial.println(request);
  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      http.end();
      return payload;
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return "fail";
}
