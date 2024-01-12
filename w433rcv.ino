/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/
#if defined(ws433)
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
void check_code_w433(String codeIR) {
  //open all w433 codes,
  //find match
  //find match with button
  //do button
  char numbers_i = 0;
  DynamicJsonBuffer jsonBuffer;
  String irlist = readCommonFiletoJson("IRButtons");
  JsonObject& root = jsonBuffer.parseObject(irlist);
  root.containsKey("num") ? numbers_i = root["num"] : numbers_i = 0;
  for (char i = 0; i < numbers_i; i++) {
    String code = root["code"][i];
    if (code == codeIR) { //found
      String name_i = root["name"][i];
      //write logic to do action
      for (char i1 = 0; i1 < char(nWidgets); i1++) {
        if (String(descr[i1]) == name_i) {
          Serial.print("do action:");
          Serial.println(name_i);
          callback_scoket(i1, int(stat[i1]) ^ 1);

        }
      }
    }
  }
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
        check_code_w433(codeIR);
        //        saveocde_to_file(codeIR);
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
      delay(500);
    }

    mySwitch.resetAvailable();
  }
  //mySwitch.resetAvailable();



}
#endif
