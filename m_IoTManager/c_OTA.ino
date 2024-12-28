/*********
  Руи Сантос (Rui Santos)
  Более подробно о проекте на: http://randomnerdtutorials.com
  Пример в IDE Arduino: File > Examples > Arduino OTA > BasicOTA.ino
                       (Файл > Примеры > Arduino OTA > BasicOTA.ino)
*********/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266HTTPUpdateServer httpUpdater;

void setup_ota() {
httpUpdater.setup(&server);
}

