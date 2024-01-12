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
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
  // Load existing data from file
  String existingData = readCommonFiletoJson("w433");
  DeserializationError error = deserializeJson(jsonDocument, existingData);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }
  // Create a new JsonObject
  JsonObject jsonObj = jsonDocument.to<JsonObject>();
  // Add or update the code and time in the JsonObject
  jsonObj["code"] = code;
  jsonObj["time"] = String(hour()) + ":" + minute();
  // Convert JsonObject to a string
  String buffer;
  serializeJson(jsonObj, buffer);
  // Save the updated data to the file
  saveCommonFiletoJson("w433", buffer, 0);
}

void check_code_w433(String codeIR) {
  DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed

  String irlist = readCommonFiletoJson("IRButtons");

  DeserializationError error = deserializeJson(jsonDocument, irlist);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  char numbers_i = jsonDocument.containsKey("num") ? jsonDocument["num"] : 0;
  
  for (char i = 0; i < numbers_i; i++) {
    String code = jsonDocument["code"][i].as<String>();
    if (code == codeIR) { //found
      String name_i = jsonDocument["name"][i].as<String>();
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
