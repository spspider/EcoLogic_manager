/*
   IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
   An IR detector/demodulator must be connected to the input RECV_PIN.
   Version 0.1 Sept, 2015
   Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
*/
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define MIN_UNKNOWN_SIZE 12
#define CAPTURE_BUFFER_SIZE 250
//#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
// e.g. Kelvinator
// A value this large may swallow repeats of some protocols
//#else  // DECODE_AC
//#define TIMEOUT 15U  // Suits most messages, while not swallowing many repeats.
//#endif  // DECODE_AC

uint16_t RECV_PIN = 2;
uint16_t SEND_PIN = 15;

//включить для кондиционеров
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);//ON!!!
//IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);//ON!!!


uint64_t buff1 = 0;
uint64_t overflow = -1;

const char IRCodeString_numbers_array = 5;
char IRCodeId_numbers;
String IRCodeString[IRCodeString_numbers_array];




decode_results results;

void setup_IR(bool isOn)
{

  if (RECV_PIN != -1) {
    irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
    Serial.println("Setup IR");
    irrecv.enableIRIn(); // Start the receiver
    updateIR();
  } else {
    //Serial.println("Setup IR Fail RECV_PIN =-1");
  }
  if (SEND_PIN != -1) {
  }
}
void updateIR() {
  String irJson = readCommonFiletoJson("IRButtons");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(irJson);
  unsigned int numberChosed = rootjs["num"];
  if (numberChosed > IRCodeString_numbers_array) {
    numberChosed = IRCodeString_numbers_array;
  }
  IRCodeId_numbers = numberChosed;
  for (char i = 0; i < numberChosed; i++) {
    String IRCode = rootjs["code"][i];
    if (IRCode) {
      IRCodeString[i] = IRCode;
    }
  }

}

uint64_t StrToHex(const char* str)
{
  return (uint64_t) strtoul(str, 0, 16);
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
        String full_code = IRCodeString[IrButtonID[ButtonNumber]];
        if (full_code.length() < 2) {//цифра вместо кода
          String jsonSend = readCommonFiletoJson("IrRaw_Code" + full_code);
          DynamicJsonBuffer jsonBuffer;
          JsonObject& rootjs = jsonBuffer.parseObject(jsonSend);
          if (!rootjs.success()) {
            Serial.println("parseObject() failed IrRaw_Code");
            //return;
          }
          int codeLen = rootjs["len"];
          uint16_t Signal_ON_0[250];
          if (codeLen > 250)return;
          for (int i = 0; i <= codeLen; i++) {
            Signal_ON_0[i] = rootjs["c"][i];
            //Serial.print(Signal_ON_0[i]);
          }
          irsend.sendRaw(Signal_ON_0, codeLen, 38);
        } else {
          irsend.sendNEC(StrToHex(IRCodeString[IrButtonID[ButtonNumber]].c_str()), 32);
        }
      } else {
        Serial.println("Fail IrButtonID[ButtonNumber]" + String(IrButtonID[ButtonNumber], DEC));
      }
    }
  }
}


void check_code_IR(String codeIR) {
  for (char i = 0; i < IRCodeId_numbers; i++) {
    if (IRCodeString[i] == codeIR) {//совпадение найдено
      for (char i1 = 0; i1 < nWidgets; i1++) {
        if (IrButtonID[i1] == i) {
          Serial.println("FIND IR:" + descr[i1] + " IrButtonID[i1]:" + String(IrButtonID[i1], DEC) + " i:" + String(i, DEC));
          stat[i1] ^= 1;
          save_stat();
          digitalWrite(pin[i1], stat[i1] );
          //stat[i1] = new_state;
          delay(300);
        }
      }
    }
  }
}
/*
  void loop_IR5() {
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    Serial.println((results.rawlen));
    //Serial.println((results.rawbuf[0]));
    for (int i = 0; i < results.rawlen; i++) {
      Serial.print((results.rawbuf[i]));
      Serial.print("\t");

    }
    Serial.println("");  // Blank line between entries
    yield();  // Feed the WDT (again)
  }
  }
*/
void loop_IR() {
  if (irrecv.decode(&results)) {
    String codeIR;
    unsigned char len = results.rawlen;
    if ((len > 100) && ((Page_IR_opened))) {
      /////ITs RAW/////////
      //unsigned int Signal_ON_0[250];
      String sendJSON;
      sendJSON = "{\"raw\":\"true\",\"len\":";
      sendJSON += results.rawlen;
      sendJSON += ",\"c\":[";
      for (uint16_t i = 1; i < results.rawlen; i++) {
        uint32_t usecs;
        for (usecs = results.rawbuf[i] * RAWTICK;
             usecs > UINT16_MAX;
             usecs -= UINT16_MAX) {
          sendJSON += uint64ToString(UINT16_MAX);
          if (i % 2)
            sendJSON += ",0,";
          else
            sendJSON += ",0,";
        }
        sendJSON += uint64ToString(usecs, 10);
        if (i < results.rawlen - 1)
          sendJSON += ", ";  // ',' not needed on the last one
        if (i % 2 == 0)  sendJSON += " ";  // Extra if it was even.
      }
      sendJSON += "]}";
      //json["c"] = Signal_ON_0;
      Serial.println(sendJSON);
      yield();  // Feed the WDT (again)
      server.send(200, "text/plain", sendJSON);
      yield();  // Feed the WDT (again)
      Page_IR_opened = false;
      //irrecv.resume();
    } if (len < 100) {
      char charBuff1[21];
      sprintf(charBuff1, "%X", results.value);
      Serial.println("short code:" + String(charBuff1));
      yield();  // Feed the WDT (again)
      buff1 = 0;
      codeIR = String(charBuff1);
      if (!Page_IR_opened) {
        check_code_IR(codeIR);
      }
      else {
        server.send(200, "text/plain", codeIR);
        Page_IR_opened = false;
      }
      //irrecv.resume();
    }

    // Receive the next value IRRecieve
    //Serial.println(results.rawlen);
    //    yield();  // Feed the WDT (again)
  }
}
/*
  void loop_IR2() {

  if (!IR_loop) {//если выключен IR, то
    //last_millis = millis();//обновляем последнее время
  }
  // if (WebSocketConnected) {
  // else if (IR_loop) {
  if (irrecv.decode(&results)) {
    String codeIR;
    if ((results.value != overflow) && (buff1 != 0)) {
      if (buff1 != results.value) {
        char charBuff1[21];
        sprintf(charBuff1, "%X", buff1);
        char charBuff[21];
        sprintf(charBuff, "%X", results.value);
        Serial.println(charBuff1);
        Serial.println(charBuff);
        codeIR = String(charBuff1);
        buff1 = 0;
        if (!Page_IR_opened) {
          check_code_IR(codeIR);
        }
        else {
          server.send(200, "text/plain", codeIR);
        }
      }
      else {
        char charBuff1[21];
        sprintf(charBuff1, "%X", buff1);
        Serial.println(charBuff1);
        buff1 = 0;
      }
      buff1 = 0;
    }
    if (buff1 == 0) {
      buff1 = results.value;
      irrecv.resume();
    }
    if (results.value == overflow) {
      //Serial.print("overflow");
      char charBuff1[21];
      sprintf(charBuff1, "%X", buff1);
      buff1 = 0;
      Serial.println(charBuff1);
    }

    //Serial.print("Str:");

    //char buff[21];
    // copy to buffer
    //sprintf(buff, "%X", results.value);
    //Serial.println(buff);
    irrecv.resume();  // Receive the next value
  }
  // }
  }
*/
