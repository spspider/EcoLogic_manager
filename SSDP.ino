void SSDP_init(void) {
  // SSDP дескриптор
  server.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(server.client());
  });
  //Если версия  2.0.0 закаментируйте следующую строчку
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(ipport);
  SSDP.setName(deviceID);
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("LED");
  SSDP.setModelNumber("000000000001");
  //SSDP.setModelURL("http://esp8266-arduinoide.ru/step3-ssdp/");
  SSDP.setManufacturer("Sergey");
  //SSDP.setManufacturerURL("http://www.esp8266-arduinoide.ru");
  SSDP.begin();
}
