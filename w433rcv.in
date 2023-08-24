/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup_w433() {
  //Serial.begin(9600);
  if (w433rcv != 255) {
    mySwitch.enableReceive(w433rcv);  // Receiver on interrupt 0 => that is pin #2
    Serial.println("W433_Enabled" + String(w433rcv, DEC));
    //mySwitch.disableReceive(15);
  }
}

void saveocde_to_file(String code) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonObj = jsonBuffer.createObject();
  jsonObj["code"] = code;
  jsonObj["time"] = String(hour() + ":" + minute());
  String buffer;
  jsonObj.printTo(buffer);
  saveCommonFiletoJson("w433", buffer, 0);
}

void loop_w433() {

  if (mySwitch.available()) {
    String codeIR;
    int value = mySwitch.getReceivedValue();

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      //Serial.print("Received ");
      //Serial.print( mySwitch.getReceivedValue() );
      codeIR = String(mySwitch.getReceivedValue());
      Serial.println(codeIR);
      if (!Page_IR_opened) {
        check_code_IR(codeIR);
        saveocde_to_file(codeIR);
      }
      else {
        server.send(200, "text/plain", codeIR);
      }
      //Serial.print("/");
      //Serial.print( mySwitch.getReceivedBitlength() );
      //Serial.print("bit ");
      //Serial.print("Protocol: ");
      //Serial.println( mySwitch.getReceivedProtocol() );
      //server.send(200, "text/plain", String(value));
    }

    mySwitch.resetAvailable();
  }
  //mySwitch.resetAvailable();



}
