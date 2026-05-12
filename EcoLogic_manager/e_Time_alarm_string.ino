#if defined(timeLibraryUsing)
#include <TimeLib.h>
#endif
#if defined(timerAlarm)
bool En_a[MAX_CONDITIONS];
uint8_t type_a[MAX_CONDITIONS];
uint8_t act_a[MAX_CONDITIONS];
unsigned int type_value[MAX_CONDITIONS];
uint8_t source_pin[MAX_CONDITIONS];  // PIN for each condition

// Pin change interrupt tracking - hardware ISR based
volatile bool pin_interrupt_pending[17] = {false};  // One flag per GPIO pin (ESP8266: 0-16)
struct PinCondition {
  uint8_t idx;  // condition index
  uint8_t edge_mode;  // 0=any, 1=rising, 2=falling
  uint8_t pin;
} pin_conditions[MAX_CONDITIONS];  // Array for registered pin conditions
uint8_t pin_condition_count = 0;

uint8_t num_conditions = 0;  // actual number of loaded conditions
bool alarm_is_active[MAX_CONDITIONS];

bool timer_alarm_action_switch = 0;
uint8_t timer_alarm_action = 0, timer_alarm_action_max = 20;

void setup_alarm() {
  // Load single flat conditions list from Condition0.txt
  String jsonCondition = readCommonFiletoJson("Condition0");
  if (jsonCondition != "") {
    load_Current_condition(jsonCondition);
  }
  Serial.println("setup_alarm() - OK");
}

bool load_Current_condition(String jsonCondition) {
  if (jsonCondition != "") {
    DynamicJsonDocument jsonDocument(2048);  // Larger for flat list
    DeserializationError error = deserializeJson(jsonDocument, jsonCondition);
    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return false;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();
    int Numbers_that = rootjs["Numbers"];  // number of conditions in flat list
    Numbers_that > MAX_CONDITIONS ? Numbers_that = MAX_CONDITIONS : true;
    num_conditions = Numbers_that;

    for (uint8_t i = 0; i < Numbers_that; i++) {
      type_a[i] = rootjs["type"][i].as<uint8_t>();
      act_a[i] = rootjs["act"][i].as<uint8_t>();
      type_value[i] = rootjs["type_value"][i].as<unsigned int>();
      source_pin[i] = rootjs["tID"][i].as<uint8_t>();  // source pin
      En_a[i] = rootjs["En"][i].as<bool>();
      alarm_is_active[i] = false;
    }
    Serial.println("Loaded " + String(num_conditions) + " conditions");
  }
  return true;
}

#if defined(timeLibraryUsing)
void check_if_there_timer_once() {  // проверка установки таймера
  for (uint8_t i = 0; i < num_conditions; i++) {
    if (type_a[i] == 5) {  // таймер
      unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
      type_a[i] = type_a[i] + 10;  // заведен
      type_value[i] = nowsec + type_value[i];

      Serial.println("время сейчас:" + String(nowsec / 3600, DEC) + ":" + String(nowsec % 3600 / 60, DEC) + ":" + String(nowsec % 60, DEC));
      Serial.println("установлен таймер:" + String(type_value[i] / 3600, DEC) + ":" + String(type_value[i] % 3600 / 60, DEC) + ":" + String(type_value[i] % 60, DEC));
    }
  }
}
#endif

#if defined(timeLibraryUsing)
void check_if_there_next_times() {  // вызывается каждую секунду
  for (uint8_t i = 0; i < num_conditions; i++) {
    if (En_a[i]) {
      unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
      if (type_a[i] == 15) {
        if ((nowsec == type_value[i])) {
          test_action = false;
          Serial.println("событие по времени: " + String(type_value[i], DEC) + " сейчас:" + String(nowsec, DEC));
          make_action(i, false);
          type_a[i] = type_a[i] - 10;
        }
      }
    }
  }
}
#endif

static uint8_t l_minute;
void loop_alarm() {
#if defined(timeLibraryUsing)
  if (minute() != l_minute) {
    l_minute = minute();
  }
#endif
}

void CheckInternet(String request) {
  if (millis() - lastCheck < 60000) return; // Проверка раз в минуту
  
  uint8_t timezone;
  String respond = getHttp(request);
  lastCheck = millis();

  if (respond == "fail") {  // интернета нет
    Serial.println("Интернета нет");
    relayRouter();
  } else {                                   // интернет есть
    DynamicJsonDocument jsonDocument(1024);  // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, respond);

    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();

    //"2018-08-23T07:43";
    String currentDateTime = rootjs["currentDateTime"];
#if defined(timeLibraryUsing)
    if (timeStatus() == timeNotSet) {
      setTime(
        currentDateTime.substring(11, 13).toInt() + timezone,
        currentDateTime.substring(14, 16).toInt(),
        0,
        currentDateTime.substring(8, 10).toInt(),
        currentDateTime.substring(5, 7).toInt(),
        currentDateTime.substring(0, 4).toInt());
      // setup_alarm();
    } else {
      timezone = hour() - currentDateTime.substring(11, 13).toInt();
      Serial.println("timezone:" + String(timezone, DEC));
    }
#endif
    Serial.println("Интернет есть");
  }
}

void check_for_changes() {
  if (timer_alarm_action_switch == 0) {
    for (uint8_t i = 0; i < num_conditions; i++) {
      if (type_a[i] == 2) {  // равно (по уровню)
        int stat = (int)get_new_widjet_value(source_pin[i]);
        if (type_value[i] == stat) {
          MakeIfTrue(i);
        } else {
          MakeIfFalse(i);
        }
      }
      if (type_a[i] == 3) {  // больше
        int stat = (int)get_new_widjet_value(source_pin[i]);
        if (type_value[i] < stat) {
          MakeIfTrue(i);
        } else {
          MakeIfFalse(i);
        }
      }
      if (type_a[i] == 4) {  // меньше
        int stat = (int)get_new_widjet_value(source_pin[i]);
        if (type_value[i] > stat) {
          MakeIfTrue(i);
        } else {
          MakeIfFalse(i);
        }
      }
    }
  }
}

// ISR stubs for GPIO pins - each sets its corresponding flag
void IRAM_ATTR isr_pin_0() { pin_interrupt_pending[0] = true; }
void IRAM_ATTR isr_pin_1() { pin_interrupt_pending[1] = true; }
void IRAM_ATTR isr_pin_2() { pin_interrupt_pending[2] = true; }
void IRAM_ATTR isr_pin_3() { pin_interrupt_pending[3] = true; }
void IRAM_ATTR isr_pin_4() { pin_interrupt_pending[4] = true; }
void IRAM_ATTR isr_pin_5() { pin_interrupt_pending[5] = true; }
void IRAM_ATTR isr_pin_12() { pin_interrupt_pending[12] = true; }
void IRAM_ATTR isr_pin_13() { pin_interrupt_pending[13] = true; }
void IRAM_ATTR isr_pin_14() { pin_interrupt_pending[14] = true; }
void IRAM_ATTR isr_pin_15() { pin_interrupt_pending[15] = true; }
void IRAM_ATTR isr_pin_16() { pin_interrupt_pending[16] = true; }

typedef void (*ISRFunctionPtr)();
const ISRFunctionPtr isr_functions[17] = {
  isr_pin_0, isr_pin_1, isr_pin_2, isr_pin_3, isr_pin_4,
  isr_pin_5, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
  isr_pin_12, isr_pin_13, isr_pin_14, isr_pin_15, isr_pin_16
};

void setup_pin_changes() {
  Serial.println("PIN CHANGE INTERRUPT SYSTEM initialized");
  pin_condition_count = 0;

  for (uint8_t i = 0; i < num_conditions; i++) {
    if (type_a[i] == 6 && En_a[i]) {
      uint8_t widget_idx = source_pin[i];   // tID = widget index
      uint8_t gpio = pin[widget_idx];       // actual GPIO from pin setup
      Serial.print("Cond ");
      Serial.print(i);
      Serial.print(": widget=");
      Serial.print(widget_idx);
      Serial.print(", GPIO=");
      Serial.println(gpio);

      if (gpio == 16 || gpio == 255) {
        Serial.println("  -> SKIPPED (GPIO16 or unset)");
        continue;
      }

      pin_conditions[pin_condition_count].idx = i;
      pin_conditions[pin_condition_count].edge_mode = type_value[i];
      pin_conditions[pin_condition_count].pin = gpio;
      pin_condition_count++;

      if (isr_functions[gpio] != nullptr) {
        attachInterrupt(digitalPinToInterrupt(gpio), isr_functions[gpio], CHANGE);
        Serial.print("  -> Attached interrupt to GPIO");
        Serial.println(gpio);
      } else {
        Serial.print("  -> No ISR for GPIO");
        Serial.println(gpio);
      }
    }
  }
  Serial.print("Total pin conditions registered: ");
  Serial.println(pin_condition_count);
}

void check_pin_changes() {
  // Check which pins triggered interrupts and process conditions
  for (uint8_t pin_num = 0; pin_num < 17; pin_num++) {
    if (!pin_interrupt_pending[pin_num]) continue;

    uint8_t current_state = digitalRead(pin_num);

    for (uint8_t c = 0; c < pin_condition_count; c++) {
      if (pin_conditions[c].pin != pin_num) continue;

      uint8_t idx = pin_conditions[c].idx;
      uint8_t edge_mode = pin_conditions[c].edge_mode;

      bool edge_matches = false;
      if (edge_mode == 0) {
        edge_matches = true;  // ANY edge
      } else if (edge_mode == 1 && current_state == 1) {
        edge_matches = true;  // RISING
      } else if (edge_mode == 2 && current_state == 0) {
        edge_matches = true;  // FALLING
      }

      if (edge_matches) {
        Serial.print("PIN INT - cond: ");
        Serial.print(idx);
        Serial.print(", pin: ");
        Serial.print(pin_num);
        Serial.print(", state: ");
        Serial.print(current_state);
        Serial.print(", edge: ");
        Serial.println(edge_mode == 0 ? "ANY" : (edge_mode == 1 ? "RISING" : "FALLING"));

        make_action(idx, false);
      }
    }

    pin_interrupt_pending[pin_num] = false;
  }
}

// functions to be called when an alarm triggers:

void MakeIfTrue(uint8_t i) {
  if (alarm_is_active[i]) {
    make_action(i, false);
    timer_alarm_action_switch = 1;
    alarm_is_active[i] = false;
  }
}
void MakeIfFalse(uint8_t i) {
  if (!alarm_is_active[i]) {
    if ((act_a[i] != 5) || (act_a[i] != 7) || (act_a[i] != 8)) {
      make_action(i, true);
    }
    alarm_is_active[i] = true;
  }
}

void disable_En(uint8_t idx) {
  // Placeholder for future logic if needed
}

void parseStringToArray(String inputString, uint8_t values[], uint8_t &numValues) {
  numValues = 0;  // Initialize the number of values
  while (inputString.length() > 0) {
    uint8_t spaceIndex = inputString.indexOf(' ');  // Find the index of the next space
    if (spaceIndex == -1) {
      Serial.print("Parsed value number:");
      Serial.print(numValues);
      Serial.print("is:");
      values[numValues++] = inputString.toInt();  // Convert the remaining substring to an integer
      Serial.print(inputString.toInt());
      break;
    }
    String substring = inputString.substring(0, spaceIndex);
    values[numValues++] = substring.toInt();
    inputString = inputString.substring(spaceIndex + 1);
  }
}

// Reads actOn[idx] from Condition0.txt on demand — no persistent cache
String actBtn_a_ch_string(uint8_t idx) {
  String jsonCondition = readCommonFiletoJson("Condition0");
  if (jsonCondition == "") return "";
  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, jsonCondition) != DeserializationError::Ok) return "";
  return doc["actOn"][idx].as<String>();
}

void make_action(uint8_t i, bool opposite) {
  if (En_a[i] == true) {  // если он включен
    if (act_a[i] == 2) {  // "нажать кнопку"
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(i), values_back, max_values);
      write_new_widjet_value(values_back[0], values_back[1]);
    } else if (act_a[i] == 4) {  // HTTP call
      String host = actBtn_a_ch_string(i);
      if (host.length() > 0) {
        Serial.println("HTTP call: " + host);
        String respond = getHttp(host);
        Serial.println("HTTP response: " + respond);
      } else {
        Serial.println("HTTP call skipped: empty URL");
      }
    } else if (act_a[i] == 1) {  // установить пин
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(i), values_back, max_values);
      write_new_widjet_value(values_back[0], values_back[1]);
    } else if (act_a[i] == 3) {  // удаленная кнопка
      String host = actBtn_a_ch_string(i);
      if (host.length() > 0) {
        int val_first = host.indexOf("val:");
        int val_last = host.indexOf("}");
        int value = host.substring(val_first + 4, val_last).toInt();

        if ((value > 1) && (opposite)) {
          value = 0;
        } else if (value < 2) {
          value ^= opposite;
        }
        host = host.substring(0, val_first + 4) + String(value, DEC) + "}";
        Serial.println("удаленная кнопка:" + host);
        String respond = getHttp(host);
      } else {
        Serial.println("Remote button skipped: empty host");
      }
    } else if (act_a[i] == 7) {  // ws2811
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(i), values_back, max_values);
      String DataLoad_8211 = readCommonFiletoJson("ws8211/" + String(values_back[0], DEC));
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
#if defined(ws2811_include)
      LoadData(buffer);
#endif
    } else if (act_a[i] == 8) {  // WakeOnLan
#if defined(wakeOnLan)
      char addresWakePC[20];
      strcpy(addresWakePC, actBtn_a_ch_string(i).c_str());
      wakeMyPC(addresWakePC);
#endif
    } else if (act_a[i] == 9) {  // timer
      uint8_t max_values = 5;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(i), values_back, max_values);

      uint8_t typePinsDuration_int = values_back[0];
      uint8_t typePinsTimeChoise_int = values_back[1];
      uint8_t typePinsrepeats_int = values_back[2];
      uint8_t typeDelay_int = values_back[3];
      bool typeOpposite_int = values_back[4];

      unsigned int typePinsTimeChoise_int_mult;
      switch (typePinsTimeChoise_int) {
        case 0:
          typePinsTimeChoise_int_mult = 1;  // секунды
          break;
        case 1:
          typePinsTimeChoise_int_mult = 60 * typePinsTimeChoise_int;  // минуты
          break;
        case 2:  // часы
          typePinsTimeChoise_int_mult = 60 * 60 * typePinsTimeChoise_int;
          break;
      }

      if ((typePinsrepeats_int >= 0) && (typePinsrepeats_int != 255)) {
        if (!typeOpposite_int) {
          type_value[i] = type_value[i] + typePinsDuration_int * typePinsTimeChoise_int_mult;
        } else {
          type_value[i] = type_value[i] + typeDelay_int * 60;
        }
        typeOpposite_int = typeOpposite_int ^ 1;
        typePinsrepeats_int--;
      } else if (typePinsrepeats_int == 255) {
        String jsonCondition = readCommonFiletoJson("Condition0");
        jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
        Serial.println("!!!!!!!!!!!!!!!!!!!!загрузка условия завершена");
      }
    } else if (act_a[i] == 10) {  // передвинуть слайдер
      float payload = get_new_widjet_value(source_pin[i]);
      uint8_t minTemp, maxTemp, button_;
      if (payload != 0) {
        uint8_t max_values = 3;
        uint8_t values_back[max_values];
        parseStringToArray(actBtn_a_ch_string(i), values_back, max_values);
        minTemp = values_back[0];
        maxTemp = values_back[1];
        button_ = values_back[2];

        payload = ((payload - minTemp * 1.0) * (1024.0)) / (maxTemp - minTemp) * 1.0;
        payload = payload < 0 ? 0 : payload;
        payload = payload > 1024 ? 1024 : payload;

        write_new_widjet_value(button_, payload);
      }
    }
  }
}

#endif