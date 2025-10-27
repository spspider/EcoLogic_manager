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
  pinMode(SEND_PIN, OUTPUT);
  pinMode(RECV_PIN, INPUT);
  irrecv.setUnknownThreshold(MIN_UNKNOWN_SIZE);
  Serial.println("Setup IR");
  irrecv.enableIRIn();  // Start the receiver
  updateIR();
}
void updateIR() {
  File irJson = fileSystem->open("/IRButtons.txt", "r");
  if (!irJson) { // Проверяем открытие файла
    Serial.println("Failed to open IRButtons.txt");
    return;
  }
  
  DynamicJsonDocument jsonDocument(1024);
  DeserializationError error = deserializeJson(jsonDocument, irJson);
  irJson.close(); // Закрываем файл сразу после чтения
  
  if (error) {
    Serial.print(F("deserializeJson() failed in updateIR: "));
    Serial.println(error.c_str());
    return;
  }

  // Получаем количество IR кодов
  unsigned int numberChosed = jsonDocument["num"];
  if (numberChosed > IRCodeString_numbers_array) {
    numberChosed = IRCodeString_numbers_array; // Ограничиваем максимум
  }
  IRCodeId_numbers = numberChosed;

  // Загружаем IR коды в массив
  for (uint8_t i = 0; i < numberChosed; i++) {
    const char* IRCode = jsonDocument["code"][i].as<const char*>();
    if (IRCode) {
      strncpy(IRCodeString[i], IRCode, sizeof(IRCodeString[i]) - 1);
      IRCodeString[i][sizeof(IRCodeString[i]) - 1] = '\0'; // Обеспечиваем null-termination
      Serial.println("Loaded IR code " + String(i) + ": " + String(IRCodeString[i]));
    }
  }
  Serial.println("IR codes loaded: " + String(IRCodeId_numbers));
}


uint64_t StrToHex(const char* str) {
  return (uint64_t)strtoul(str, 0, 16); // Конвертируем HEX строку в число
}
// Удалена неиспользуемая функция toLongLong
void send_IR_code(const char* full_code_char) {
  String full_code = String(full_code_char);
  
  if (full_code.length() < 2) { // Короткий код - значит это RAW сигнал
    File jsonSend = fileSystem->open("/IrRaw_Code" + full_code + ".txt", "r");
    if (!jsonSend) {
      Serial.println("Failed to open RAW IR file: IrRaw_Code" + full_code + ".txt");
      return;
    }
    
    DynamicJsonDocument jsonDocument(1024);
    DeserializationError error = deserializeJson(jsonDocument, jsonSend);
    jsonSend.close(); // Закрываем файл
    
    if (error) {
      Serial.print(F("deserializeJson() failed in send_IR_code: "));
      Serial.println(error.c_str());
      return;
    }
    
    int codeLen = jsonDocument["len"];
    if (codeLen > 250 || codeLen <= 0) { // Проверяем длину
      Serial.println("Invalid RAW signal length: " + String(codeLen));
      return;
    }
    
    uint16_t Signal_ON_0[250];
    for (int i = 0; i < codeLen; i++) {
      Signal_ON_0[i] = jsonDocument["c"][i]; // Загружаем RAW данные
    }
    irsend.sendRaw(Signal_ON_0, codeLen, 38); // Отправляем RAW сигнал
    Serial.println("Sent RAW IR signal, length: " + String(codeLen));
  } else {
    irsend.sendNEC(StrToHex(full_code_char), 32); // Отправляем NEC код
    Serial.println("Sent NEC IR code: " + String(full_code_char));
  }
}

void send_IR(char ButtonNumber) {
  // Проверяем корректность номера кнопки
  if (ButtonNumber == char(-1) || IrButtonID[ButtonNumber] == 255) {
    return; // Некорректный номер или нет IR кода
  }
  
  // Проверяем что IR код существует
  if (IrButtonID[ButtonNumber] >= IRCodeString_numbers_array) {
    Serial.println("IR code index out of range: " + String(IrButtonID[ButtonNumber], DEC));
    return;
  }
  
  Serial.println("Sending IR for button " + String(ButtonNumber, DEC) + ", code: " + String(IRCodeString[IrButtonID[ButtonNumber]]));
  send_IR_code(IRCodeString[IrButtonID[ButtonNumber]]); // Отправляем IR код
}


void check_code_IR(String codeIR) {
  // Проходим по всем IR кодам из файла IRButtons.txt
  for (uint8_t i = 0; i < IRCodeId_numbers; i++) {
    // Проверяем совпадение полученного кода с кодом из файла
    if (strcmp(IRCodeString[i], codeIR.c_str()) == 0) {
      // Ищем кнопку с соответствующим IR кодом
      for (uint8_t i1 = 0; i1 < nWidgets; i1++) {
        if (IrButtonID[i1] == i) { // Найдена кнопка с этим IR кодом
          Serial.println("FIND IR:" + String(descr[i1]) + " IrButtonID[i1]:" + String(IrButtonID[i1], DEC) + " i:" + String(i, DEC));
          
          // Определяем тип кнопки и переключаем соответственно
          if (pinmode[i1] == 2) { // Switch (цифровой выход)
            stat[i1] = stat[i1] ? 0 : 1; // Переключаем 0↔1
            digitalWrite(pin[i1], stat[i1]); // Применяем к пину
          } 
          else if (pinmode[i1] == 3) { // PWM
            stat[i1] = (stat[i1] > 0) ? 0 : 1024; // Переключаем 0↔1024
            analogWrite(pin[i1], stat[i1]); // Применяем PWM
          }
          
          delay(300); // Защита от дребезга
          break; // Выходим из цикла поиска кнопки
        }
      }
      break; // Выходим из цикла поиска кода (код найден)
    }
  }
}

void loop_IR() {
  if (!irrecv.decode(&results)) return; // Нет сигнала - выходим
  
  irrecv.resume(); // Возобновляем прием как можно раньше

  // Фильтруем шум и повторы
  if (results.rawlen < 30 || results.value == 0xFFFFFFFF || results.value == 0) {
    return; // irrecv.resume() уже вызван выше
  }

  // Обработка RAW сигнала для настройки IR
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

  // Обработка стандартного кода (NEC и др.)
  if (results.value != 0xFFFFFFFF && results.value != 0) {
    char codeHex[21];
    sprintf(codeHex, "%X", results.value); // Конвертируем в HEX строку
    Serial.println("IR code received: " + String(codeHex));

    String codeIR = String(codeHex);
    
    // Основная логика - проверяем код и переключаем кнопки
    check_code_IR(codeIR);

    // Отправляем код на сервер Node-RED (если настроен)
    sendIRCode_toServer(strtoul(codeHex, nullptr, 16));
    
    // Отправляем код в веб-интерфейс (если страница IR открыта)
    if (Page_IR_opened) {
      server.send(200, "text/plain", codeIR);
      Page_IR_opened = false;
    }
  }
}


#endif