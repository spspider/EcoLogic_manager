/*
   IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
   An IR detector/demodulator must be connected to the input RECV_PIN.
   Version 0.1 Sept, 2015
   Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
*/
#if defined(USE_IRUTILS)

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define MIN_UNKNOWN_SIZE 12
#define CAPTURE_BUFFER_SIZE 250
//#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.


//включить для кондиционеров
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);  //ON!!!
//IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);  //ON!!!


uint64_t buff1 = 0;
uint64_t overflow = -1;

const char IRCodeString_numbers_array = 5;
char IRCodeId_numbers;
char IRCodeString[IRCodeString_numbers_array][50];

decode_results results;

void setup_IR() {
  irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
  Serial.println("Setup IR");
  irrecv.enableIRIn();  // Start the receiver
  updateIR();
}
void updateIR() {
  File irJson = fileSystem->open("/IRButtons.txt", "r");
  DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed

  DeserializationError error = deserializeJson(jsonDocument, irJson);
  if (error) {
    Serial.print(F("deserializeJson() failed with code updateIR"));
    Serial.println(error.c_str());
    return;
  }

  unsigned int numberChosed = jsonDocument["num"];
  if (numberChosed > IRCodeString_numbers_array) {
    numberChosed = IRCodeString_numbers_array;
  }
  IRCodeId_numbers = numberChosed;

  for (uint8_t i = 0; i < numberChosed; i++) {
    const char* IRCode = jsonDocument["code"][i].as<const char*>();
    if (IRCode) {
      strncpy(IRCodeString[i], IRCode, sizeof(IRCodeString[i]) - 1);
      IRCodeString[i][sizeof(IRCodeString[i]) - 1] = '\0';  // Ensure null-termination
    }
  }
}


uint64_t StrToHex(const char* str) {
  return (uint64_t)strtoul(str, 0, 16);
}
long long toLongLong(String x) {
  unsigned long long y = 0;
  for (int i = 0; i < x.length(); i++) {
    char c = x.charAt(i);
    if (c < '0' || c > '9') break;
    y *= 10;
    y += (c - '0');
  }
  return y;
}
void send_IR_code(const char* full_code_char) {
  String full_code = String(full_code_char);
  if (full_code.length() < 2) {  //цифра вместо кода
    //    String jsonSend = readCommonFiletoJson("IrRaw_Code" + full_code);
    File jsonSend = fileSystem->open("/IrRaw_Code" + full_code + ".txt", "r");
    DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, jsonSend);
    if (error) {
      Serial.print(F("deserializeJson() failed with code send_IR_code"));
      Serial.println(error.c_str());
      return;
    }
    int codeLen = jsonDocument["len"];
    if (codeLen > 250) return;
    uint16_t Signal_ON_0[250];
    for (int i = 0; i < codeLen; i++) {
      Signal_ON_0[i] = jsonDocument["c"][i];
      // Serial.print(Signal_ON_0[i]);
    }
    irsend.sendRaw(Signal_ON_0, codeLen, 38);
  } else {
    irsend.sendNEC(StrToHex(full_code_char), 32);
  }
}

void send_IR(char ButtonNumber) {

  if (ButtonNumber != char(-1)) {
    if (IrButtonID[ButtonNumber] != 255) {
      Serial.println("send_IR:" + String(ButtonNumber, DEC));
      if (IrButtonID[ButtonNumber] < IRCodeString_numbers_array) {
        //int code = IRCodeString[IrButtonID[ButtonNumber]];
        Serial.println("SEND IR IRCodeString:" + String(IRCodeString[IrButtonID[ButtonNumber]]) + "IrButtonID:" + String(IrButtonID[ButtonNumber], DEC));
        /*   uint64_t number = toLongLong(IRCodeString[IrButtonID[ButtonNumber]]);
           unsigned long long1 = (unsigned long)((number & 0xFFFF0000) >> 16 );
           unsigned long long2 = (unsigned long)((number & 0x0000FFFF));
        */

        send_IR_code(IRCodeString[IrButtonID[ButtonNumber]]);
      } else {
        Serial.println("Fail IrButtonID[ButtonNumber]" + String(IrButtonID[ButtonNumber], DEC));
      }
    }
  }
}


void check_code_IR(String codeIR) {
  for (uint8_t i = 0; i < IRCodeId_numbers; i++) {
    //if (IRCodeString[i] == codeIR) {//совпадение найдено
    if (strcmp(IRCodeString[i], codeIR.c_str()) == 0) {
      for (uint8_t i1 = 0; i1 < nWidgets; i1++) {
        if (IrButtonID[i1] == i) {
          Serial.println("FIND IR:" + String(descr[i1]) + " IrButtonID[i1]:" + String(IrButtonID[i1], DEC) + " i:" + String(i, DEC));
          stat[i1] ^= 1;
          //          save_stat_void();
          digitalWrite(pin[i1], stat[i1]);
          //stat[i1] = new_state;
          delay(300);
        }
      }
    }
  }
}

void loop_IR() {
  if (irrecv.decode(&results)) {
    irrecv.resume();  // Always resume as early as possible

    // Filter out very short signals (likely noise)
    if (results.rawlen < 30 || results.value == 0xFFFFFFFF || results.value == 0) {
      irrecv.resume();
      return;
    }

    // Handle RAW long signal
    if (results.rawlen > 100 && Page_IR_opened) {
      StaticJsonDocument<1024> doc;
      JsonArray array = doc.createNestedArray("c");
      doc["raw"] = true;
      doc["len"] = results.rawlen;

      for (uint16_t i = 1; i < results.rawlen; i++) {
        uint32_t usecs = results.rawbuf[i] * RAWTICK;
        while (usecs > UINT16_MAX) {
          array.add(UINT16_MAX);
          array.add(0);
          usecs -= UINT16_MAX;
        }
        array.add(usecs);
      }

      String sendJSON;
      serializeJson(doc, sendJSON);
      server.send(200, "application/json", sendJSON);
      Page_IR_opened = false;
      return;
    }

    // Handle standard code (NEC, etc.)
    if (results.value != 0xFFFFFFFF && results.value != 0) {
      char codeHex[21];
      sprintf(codeHex, "%X", results.value);
      Serial.println("short code: " + String(codeHex));

      String codeIR = String(codeHex);

      // Only handle known codes
      check_code_IR(codeIR);
      sendIRCode_toServer(strtoul(codeHex, nullptr, 16));
      if (Page_IR_opened) {
        server.send(200, "text/plain", codeIR);
        Page_IR_opened = false;
      }
    }
  }
}


#endif