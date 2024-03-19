/*
   TimeAlarmExample.pde

   This example calls alarm functions at 8:30 am and at 5:45 pm (17:45)
   and simulates turning lights on at night and off in the morning
   A weekly timer is set for Saturdays at 8:30:30

   A timer is called every 15 seconds
   Another timer is called once only after 10 seconds

   At startup the time is set to Jan 1 2011  8:29 am
*/
#include <TimeLib.h>
//#include <TimeAlarms.h>

bool En_a[Condition][Numbers];
uint8_t type_a[Condition][Numbers];
uint8_t act_a[Condition][Numbers];
//char actBtn_a_ch[Condition][Numbers][20];
//String actBtn_a_ch[Condition][Numbers];

//unsigned int times[Condition][Numbers];//время в формате часы*60+минуты.
unsigned int  type_value[Condition][Numbers];


char NumberIDs[Condition];
bool alarm_is_active[Condition][Numbers];

bool timer_alarm_action_switch = 0;
unsigned char timer_alarm_action = 0, timer_alarm_action_max = 20;

void setup_alarm() {

  for (char i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
    char thatCondition = i1;//idWidget кнопка
    NumberIDs[thatCondition] = 0;
    String NameFile = "Condition" + String(thatCondition, DEC);
    String jsonCondition = readCommonFiletoJson(NameFile);
    if (jsonCondition != "") {
      load_Current_condition(jsonCondition);
    }
  }
  Serial.println("setup_alarm() - OK");
}
//String saveConditiontoJson(char CondWidjet) {
//  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
//  JsonObject json = jsonDocument.to<JsonObject>();
//
//  json["ID"] = String(CondWidjet, DEC);
//  json["Numbers"] = String(NumberIDs[CondWidjet], DEC);
//
//  JsonArray En_json = json.createNestedArray("En");
//  JsonArray times_json = json.createNestedArray("times");
//  JsonArray bySignal_json = json.createNestedArray("bySignal");
//  JsonArray bySignalPWM_json = json.createNestedArray("bySignalPWM");
//  JsonArray type_json = json.createNestedArray("type");
//  JsonArray act_json = json.createNestedArray("act");
//  JsonArray actBtn_json = json.createNestedArray("actBtn");
//  JsonArray actOn_json = json.createNestedArray("actOn");
//
//  for (char i = 0; i < (int)NumberIDs[CondWidjet]; i++) {
//    En_json.add(En_a[CondWidjet][i]);
//    times_json.add(times[CondWidjet][i]);
//    bySignal_json.add((int)type_value[CondWidjet][i]);
//    bySignalPWM_json.add(bySignalPWM[CondWidjet][i]);
//    type_json.add(type_a[CondWidjet][i]);
//    act_json.add(act_a[CondWidjet][i]);
////    actBtn_json.add(actBtn_a_ch[CondWidjet][i]);
//
//    alarm_is_active[CondWidjet][i] = false;
//  }
//
//  String buffer;
//  serializeJson(json, buffer);
//  saveCommonFiletoJson("Condition" + String(CondWidjet, DEC), buffer, 1);
//  return buffer;
//}
/// parse String



bool load_Current_condition(String jsonCondition) {
  //String jsonCondition = LoadCondition(thatCondition);//загружаем условия кнопки 0;

  //Serial.println(jsonCondition);
  if (jsonCondition != "") {
    DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, jsonCondition);

    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return false;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();
    if (error) {
      Serial.println("PARSE FAIL!!");
      return false;
    }

    char WidjetIds = rootjs["ID"]; // 1 //номер кнопки
    int Numbers_that = rootjs["Numbers"]; // 1 //в одном файле кол-во таймеров
    char thatCondition = WidjetIds;
    Numbers_that > Numbers ? Numbers_that = Numbers : true;

    for (char i = 0; i < Numbers_that; i++) { //от всего колимчества таймеров
      char type_that = rootjs["type"][i];
      int timer_that = rootjs["timer"][i];
      char act_that = rootjs["act"][i];
      //      strncpy(actBtn_a_ch[thatCondition][i], rootjs["actBtn"][i], sizeof(actBtn_a_ch[thatCondition][i])); //error
      //      actBtn_a_ch[thatCondition][i] = rootjs["actBtn"][i];

      char actOn_that = rootjs["actOn"][i];



      bool En_that = rootjs["En"][i];

      type_a[thatCondition][i] = 0;
      type_a[thatCondition][i] = type_that;
      act_a[thatCondition][i] = act_that;

      type_value[thatCondition][i] = rootjs["type_value"][i].as<unsigned int>();
      En_a[thatCondition][i] = En_that;

      alarm_is_active[thatCondition][i] = alarm_is_active[thatCondition][i] ^ true;

#if defined(pubClient)
      if (client.connected()) {
        if ((type_value[thatCondition][i] == 2) || (type_value[thatCondition][i] == 3)) {
          //          Serial.println("POSSIBLE PUBLISH bySignalPWM[c][n]:" + String(bySignalPWM[thatCondition][i], DEC));
          char pubstatus[40];
          sprintf(pubstatus, "%s/PLUS/%d/%d", deviceID, thatCondition, i);
          //          pubStatus(pubstatus, setStatus(bySignalPWM[thatCondition][i]));
        }
      }
#endif
    }

    NumberIDs[thatCondition] = Numbers_that; //количество в этом условии (на этой кнопке);
    NumberIDs[thatCondition] > 10 ? NumberIDs[thatCondition] = 0 : true;
    //check_if_there_timer_times(thatCondition);
  }
  return true;
}

void check_if_there_timer_once(uint8_t idWidget) {//проверка установки таймера
  for (uint8_t i = 0; i < NumberIDs[idWidget]; i++) {
    if (type_a[idWidget][i] == 5) {//таймер
      unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
      type_a[idWidget][i] = type_a[idWidget][i] + 10; //заведен
      type_value[idWidget][i] = nowsec + type_value[idWidget][i];

      //idA[idWidget][i] = Alarm.timerOnce( timer_a[idWidget][i] * multiply, OnceOnly);
      Serial.println("время сейчас:" + String( nowsec / 3600, DEC) + ":" + String( nowsec % 3600 / 60, DEC) + ":" + String( nowsec % 60, DEC) );
      Serial.println("установлен таймер:" + String( type_value[idWidget][i] / 3600, DEC) + ":" + String( type_value[idWidget][i] % 3600 / 60, DEC) + ":" + String( type_value[idWidget][i] % 60, DEC) );
    }
  }
}
/*
  void check_if_there_load_times(int thatCondition) {//не задействован, можно активировать вместо будильника Alarm
  String NameFile = "Condition" + String(thatCondition);
  String jsonCondition = readCommonFiletoJson(NameFile);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(jsonCondition);
  if (!rootjs.success()) {
    Serial.println("PARSE FAIL!!");
  }
  uint8_t Numbers_that = rootjs["Numbers"];//1//в одном файле кол-во таймеров
  unsigned short int nowsec = (minute()) + (hour() * 60);
  for (char i = 0; i < Numbers_that; i++) {//от всего колимчества таймеров
    int times_that = rootjs["times"][i];
    if (nowsec == times[thatCondition][i]) {
      Serial.println("событие по времени: " + String(times[thatCondition][i], DEC) + " сейчас:" + String(times[thatCondition][i], DEC));
      make_action(thatCondition, i, false);
    }
  }
  }
*/
void check_if_there_next_times() {//вызывается каждую секунду

  for (uint8_t idWidget = 0; idWidget < Condition; idWidget++) {//пробегаемся по всем кнопкам
    for (unsigned char i = 0; i < Numbers; i++) {
      if (En_a[idWidget][i]) {
        unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);
        if (type_a[idWidget][i] == 15) {
          if ((nowsec == type_value[idWidget][i])) {
            test_action = false;
            Serial.println("событие по времени: " + String(type_value[idWidget][i], DEC) + " сейчас:" + String(nowsec, DEC));
            make_action(idWidget, i, false);
            type_a[idWidget][i] = type_a[idWidget][i] - 10;
          }
        }
      }
    }
  }
}
/*
  void check_if_there_timer_times(uint8_t idWidget) {//проверка установки буильника//отключен
  for (uint8_t i = 0; i < NumberIDs[idWidget]; i++) {
    if (type_a[idWidget][i] == 1) {//по достижению времени
      // int *Time_alarm = getTime_min_hour(times[idWidget][i]);
      int Time_alarm[2];
      Time_alarm[0] = times[idWidget][i] / 3600;
      Time_alarm[1] = times[idWidget][i] % 3600/60;
      idA[idWidget][i] = Alarm.alarmRepeat(Time_alarm[0] , Time_alarm[1], 0, OnceOnly); //H:M
      Serial.println("Cond type by time: " + String( Time_alarm[0], DEC) + ":" + String( Time_alarm[1], DEC) + " H:M" + " idAlarm:" + String(idA[idWidget][i], DEC));
      //если текущее время больше установленного
      unsigned int nowsec = (minute()) + (hour() * 60);

            if (times[idWidget][i] < nowsec) {
              Serial.println("уже установлено: " + String(times[idWidget][i], DEC) + "<" + String(nowsec, DEC));
              make_action(idWidget, i, false);
            }


    }
  }
  }
*/
/*
  int *getTime(unsigned int sec) {//берем время из секунд
  static int Time[3];
  //float timeSet = 0.0F;
  float hour_set_float = sec / 3600.0;
  float minutes_set_float = ((hour_set_float - (int)(hour_set_float)) * 60.0);
  float sec_set_float = ((minutes_set_float - (int)(minutes_set_float)) * 60.0);
  Time[0] = round(hour_set_float);
  Time[1] = round(minutes_set_float);
  Time[2] = round(sec_set_float);

  return Time;
  }
*/
/*
  uint8_t *getDelimeters(String DelString, String Delby) {
  static uint8_t Delimeters[3];
  int i = 0;
  while (DelString.indexOf(Delby) >= 0) {
    int delim = DelString.indexOf(Delby);
    Delimeters[i] = (DelString.substring(0, delim)).toInt();
    DelString = DelString.substring(delim + 1, DelString.length());
    i++;
    if (DelString.indexOf(Delby) == -1) {
      Delimeters[i] = DelString.toInt();
    }
  }
  //Serial.println("Delimeters: кнопка:" + String(Delimeters[0]) + " номер по порядку:" + String(Delimeters[1]));
  return Delimeters;
  }
*/
/*
  int *getTime_min_hour(unsigned int sec) {//берем время из секунд
  static int Time[2];
  //float timeSet = 0.0F;
  float hour_set_float = sec / 60.0;
  float minutes_set_float = ((hour_set_float - (int)(hour_set_float)) * 60.0);
  //float sec_set_float = ((minutes_set_float - (int)(minutes_set_float)) * 60.0);
  Time[0] = (int)(hour_set_float);
  Time[1] = (int)(minutes_set_float);
  //Time[2] = round(sec_set_float);

  return Time;
  }
*/
/*
  int GetTime(String Time, int pos) {//берем время из текста 08:00:00
  if (pos == 0) {
    int i = Time.indexOf(":");
    Time = Time.substring(0, i);
  }
  else if (pos == 1)  {
    int i = Time.indexOf(":");
    Time = Time.substring(i + 1);
    i = Time.indexOf(":");
    Time = Time.substring(0, i);
  } else if (pos == 2) {
    int i = Time.indexOf(":");
    Time = Time.substring(i + 1);
    i = Time.indexOf(":");
    Time = Time.substring(i + 1, Time.length());
  }
  int Time_int = Time.toInt();
  return Time_int;
  }
*/
static unsigned char l_minute;
void loop_alarm() {

  //if (alarm_is_active) {
  // Alarm.delay(0); // wait one second between clock display
  //}



  if (minute() != l_minute) {

    l_minute = minute();
  }
}
void CheckInternet(String request) {
  char timezone;
  String respond = getHttp(request);

  if (respond == "fail") { //интернета нет
    Serial.println("Интернета нет");
    relayRouter();
  } else { //интернет есть
    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, respond);

    if (error) {
      Serial.println("JSON Parsing Error: " + String(error.c_str()));
      return;
    }

    JsonObject rootjs = jsonDocument.as<JsonObject>();

    //"2018-08-23T07:43";
    String currentDateTime = rootjs["currentDateTime"];
    if (timeStatus() == timeNotSet) {
      setTime(
        currentDateTime.substring(11, 13).toInt() + timezone,
        currentDateTime.substring(14, 16).toInt(),
        0,
        currentDateTime.substring(8, 10).toInt(),
        currentDateTime.substring(5, 7).toInt(),
        currentDateTime.substring(0, 4).toInt()
      );
      //setup_alarm();
    } else {
      timezone = hour() - currentDateTime.substring(11, 13).toInt();
      Serial.println("timezone:" + String(timezone, DEC));
    }
    Serial.println("Интернет есть");
  }
}

/*
  void check_that_digital_read(int value, int idWidget, int i) {
  if (digitalRead(pin[idWidget]) == value) {
    if (alarm_is_active[idWidget][i]) {
      make_action(idWidget, i, false);
      alarm_is_active[idWidget][i] = false;
    }
  } else {
    if (!alarm_is_active[idWidget][i]) {
      if (act_a[idWidget][i] != 5) {// переключить условие
        make_action(idWidget, i, true);
      }
      alarm_is_active[idWidget][i] = true;
    }

  }
  }
*/
#if defined(pubClient)

char subscr_loop_PLUS_i = 0;
char subscr_loop_PLUS_i1 = 0;

void subscr_loop_PLUS() {
  if ((!client.connected()) || (!IOT_Manager_loop))  return;
  if (subscr_loop_PLUS_i <= 3) {
    if (subscr_loop_PLUS_i1 <= NumberIDs[subscr_loop_PLUS_i]) {
      //  if (bySignalPWM[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] != -1) {
      if ((type_value[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 2) || (type_value[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 3)) {
        //String __topic_subscr = deviceID + "/PLUS/" + String(subscr_loop_PLUS_i, DEC) + "/" + String(subscr_loop_PLUS_i1, DEC) + "/status";
        char topic_subscr_char[50];

        sprintf(topic_subscr_char, "%s/PLUS/%d/%d/status", deviceID, subscr_loop_PLUS_i, subscr_loop_PLUS_i1);
        if (!client.subscribe(topic_subscr_char)) {
          Serial.print("Client subscribe FAIL!:");
          Serial.println(topic_subscr_char);
        }
        else {
          Serial.print("Client subscribe SUCCSESS!:");
          Serial.println(topic_subscr_char);
          subscribe_loop++;
        };
      }
      subscr_loop_PLUS_i1++;
    } else {
      subscr_loop_PLUS_i++;//ERRRRRRRRRRRRROOOOOORRRRRRRRRR может подписываться до 3 и дальше останавливается
      subscr_loop_PLUS_i1 = 0;
    }
  }
}
#endif
void check_for_changes() {
  //Serial.println("check");
  if (timer_alarm_action_switch == 0) {
    for (uint8_t i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
      uint8_t idWidget = i1;
      for (uint8_t i = 0; i < Numbers; i++) {//от всего колимчества таймеров
        stat[idWidget] = (int) get_new_pin_value(idWidget);
        if (type_a[idWidget][i] == 2) {//равно (по уровню)
          if (type_value[idWidget][i] == stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
        if (type_a[idWidget][i] == 3) {//больше
          if (type_value[idWidget][i] < stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
        if (type_a[idWidget][i] == 4) {//меньше
          if (type_value[idWidget][i] > stat[idWidget]) {
            MakeIfTrue(idWidget, i);
          } else {
            MakeIfFalse(idWidget, i);
          }
        }
      }

    }
  }
}



// functions to be called when an alarm triggers:

void MakeIfTrue(uint8_t idWidget, uint8_t i) {
  if (alarm_is_active[idWidget][i]) {
    make_action(idWidget, i, false);
    timer_alarm_action_switch = 1;
    alarm_is_active[idWidget][i] = false;
  }
}
void MakeIfFalse(char idWidget, char i) {
  if (!alarm_is_active[idWidget][i]) {
    if ((act_a[idWidget][i] != 5) ||  (act_a[idWidget][i] != 7) || (act_a[idWidget][i] != 8)) { // переключить условие//act_a[i1][i] != 4)(act_a[idWidget][i] != 7)||(act_a[idWidget][i] != 8))
      make_action(idWidget, i, true);
    }

    alarm_is_active[idWidget][i] = true;

    //Serial.println("opposite adc");
  }
}

void disable_En(uint8_t that_condtion_widget, uint8_t that_number_cond) {
  for (uint8_t i1 = 0; i1 < Condition; i1++) {
    for (uint8_t i = 0; i < NumberIDs[i1]; i++) {

      //if (actBtn_a[that_condtion_widget][that_number_cond] ==  actBtn_a[i1][i]) {
      //      if (strcmp (actBtn_a_ch[that_condtion_widget][that_number_cond], actBtn_a_ch[i1][i]) == 0) {
      //        Serial.println("схожее:" + String(i1) + String(i) + String(actBtn_a_ch[i1][i]) + " " + String(actBtn_a_ch[that_condtion_widget][that_number_cond]));
      //        if ((that_number_cond != i) || (that_condtion_widget != i1)) {
      //          //Нашли совпадение, и если это не то, что выполнено, то отключаем его
      //          Serial.println("!!!!!!!!!!!!!!отключаем условие, которое тоже задействовано на эту кнопку" + String(i1, DEC) + String(i, DEC));
      //          En_a[i1][i] = false;
      //        }
      //      }

    }

  }

}

void parseStringToArray(String inputString, uint8_t values[], uint8_t& numValues) {
  numValues = 0; // Initialize the number of values
  while (inputString.length() > 0) {
    uint8_t spaceIndex = inputString.indexOf(' '); // Find the index of the next space
    if (spaceIndex == -1) {
      Serial.print("Parsed value number:");
      Serial.print(numValues);
      Serial.print("is:");
      values[numValues++] = inputString.toInt(); // Convert the remaining substring to an integer
      Serial.print(inputString.toInt());
      break;
    }
    String substring = inputString.substring(0, spaceIndex);
    values[numValues++] = substring.toInt();
    inputString = inputString.substring(spaceIndex + 1);
  }
}

String actBtn_a_ch_string(uint8_t that_condtion_widget, uint8_t that_number_cond) {
  File actBtn_a_ch_string_file = SPIFFS.open("/Condition" + String(that_condtion_widget, DEC) + ".txt", "r");
  String actBtn_a_ch = "";
  DynamicJsonDocument rootjs(1024); // Adjust the capacity as needed
  DeserializationError error = deserializeJson(rootjs, actBtn_a_ch_string_file);
  if (error) {
    Serial.println("PARSE Condition" + String(that_condtion_widget, DEC) + "FAIL!!");
    return actBtn_a_ch;
  }
  char WidjetIds = rootjs["ID"];
  int Numbers_that = rootjs["Numbers"];

  actBtn_a_ch = rootjs["actOn"][that_number_cond].as<String>();

  return actBtn_a_ch;
}
void make_action(uint8_t that_condtion_widget, uint8_t that_number_cond, bool opposite) {
  if (En_a[that_condtion_widget][that_number_cond] == true) {//если он включен
    if (act_a[that_condtion_widget][that_number_cond] == 2) { ////////////////////////////"нажать кнопку"////////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      callback_scoket(values_back[0], values_back[1]);
      //values_back[0] - первое второе и т.д.
    }
    else if ((act_a[that_condtion_widget][that_number_cond] == 4) && (!opposite)) { //"отправить Email"//////////////////////////////////////////////////////////
      String buffer;
      buffer += actBtn_a_ch_string(that_condtion_widget, that_number_cond); //сообщение в условии
      //      buffer = "сработала тревога на датчике:" + String(descr[that_condtion_widget]) + " топик:" + String(sTopic_ch[that_condtion_widget]) + " на пине:" + String(digitalRead(pin[that_condtion_widget]));
      buffer = "сработала тревога на датчике:" + String(descr[that_condtion_widget]) + " на пине:" + String(digitalRead(pin[that_condtion_widget]));

      buffer += "\n";
      buffer += "время на устройстве:" + String(hour()) + ":" + String(minute());
      buffer += "последнее местоположение:";
      buffer += readCommonFiletoJson("ip_gps");
      buffer += "\n";
      Serial.print("Sending Email:");
      Serial.println(sendEmail(buffer));
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 1) { /////////////////////////////установить пин///////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      callback_scoket(values_back[0], values_back[1]);
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 3) { ///////////////////////////удаленная кнопка///////////////////////////////////////////////////

      String host = "";//String(actBtn_a_ch[that_condtion_widget][that_number_cond]);//узнаем хост и кнопку
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
      //Serial.println("value:" + String(value^opposite));
      String respond = getHttp(host);
      //Serial.println(respond);


    }
    else if (act_a[that_condtion_widget][that_number_cond] == 6) { ///////////////////////////mqtt запрос/////////////////////////////////////////////////
#ifdef pubClient
      DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
      String mqtt_parse = "";//String(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      Serial.println(mqtt_parse);
      DeserializationError error = deserializeJson(jsonDocument, mqtt_parse);

      if (error) {
        Serial.println("Parsing fail mqtt");
        return;
      }

      JsonObject root = jsonDocument.as<JsonObject>();
      char char_arr[10];
      int value = root["msg"];
      value ^= opposite;
      sprintf(char_arr, "%d", value);


      String topic = root["Topic"].as<String>();
      String message = String(char_arr);

      Serial.println("Publish : (topic:" + topic + " msg:" + message + ")");
      if (client.connected()) {
        if (client.publish(topic.c_str(), message.c_str())) {
          Serial.println("Publish SUCCESS! (topic:" + topic + " msg:" + message + ")");
        } else {
          Serial.println("Publish FAIL! (topic:" + topic + " msg:" + message + ")");
        }
      } else {
        Serial.println("Publish FAIL! client disconnected");
      }
#endif
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 7) { ///////////////////////////8211/////////////////////////////////////////////////
      uint8_t max_values = 3;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
      String DataLoad_8211 = readCommonFiletoJson("ws8211/" + String(values_back[0], DEC));
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
#if defined(ws2811_include)
      LoadData(buffer);
#endif
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 8) { /////////////////////////WakeOnLan///////////////////////////
      char addresWakePC[20];
      strcpy(addresWakePC, actBtn_a_ch_string(that_condtion_widget, that_number_cond).c_str());
      wakeMyPC(addresWakePC);
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 9) { /////////////////////////timer///////////////////////////
      //      switch_action(that_condtion_widget, that_number_cond, opposite);
      //      callback_scoket(values_back[0], values_back[1]);
      uint8_t max_values = 5;
      uint8_t values_back[max_values];
      parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);


      unsigned char typePinsDuration_int = values_back[0];
      unsigned char  typePinsTimeChoise_int = values_back[1];
      unsigned char typePinsrepeats_int = values_back[2];
      unsigned char typeDelay_int = values_back[3];
      bool typeOpposite_int = values_back[4];

      Serial.print("продолжительность:"); Serial.println(typePinsDuration_int);
      Serial.print("мин:"); Serial.println(typePinsTimeChoise_int);
      Serial.print("повтор:"); Serial.println(typePinsrepeats_int);
      Serial.print("перерыв:"); Serial.println(typeDelay_int);
      Serial.print("typeOpposite_int:"); Serial.println(typeOpposite_int);
      //установить новое время
      //узнаем старое время:
      unsigned int typePinsTimeChoise_int_mult;
      switch (typePinsTimeChoise_int) {
        case 0:
          typePinsTimeChoise_int_mult = 1;//секунды
          break;
        case 1:
          typePinsTimeChoise_int_mult = 60 * typePinsTimeChoise_int;//минуты
          break;
        case 2://часы
          typePinsTimeChoise_int_mult = 60 * 60 * typePinsTimeChoise_int;
          break;
      }

      if ((typePinsrepeats_int >= 0) && (typePinsrepeats_int != 255)) {
        if (!typeOpposite_int) {//включение первый раз на полив
          type_value[that_condtion_widget][that_number_cond] = type_value[that_condtion_widget][that_number_cond] + typePinsDuration_int * typePinsTimeChoise_int_mult;
        } else {
          type_value[that_condtion_widget][that_number_cond] = type_value[that_condtion_widget][that_number_cond] + typeDelay_int * 60;
        }
        typeOpposite_int = typeOpposite_int ^ 1;
        typePinsrepeats_int--;
        //        sprintf(actBtn_a_ch[that_condtion_widget][that_number_cond], "%d %d %d %d %d", typePinsDuration_int, typePinsTimeChoise_int, typePinsrepeats_int, typeDelay_int, typeOpposite_int);
      }
      else if (typePinsrepeats_int == 255) {
        String NameFile = "Condition" + String(that_condtion_widget, DEC);
        String jsonCondition = readCommonFiletoJson(NameFile);
        jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
        //        switch_action(that_condtion_widget, that_number_cond, 1);
        Serial.println("!!!!!!!!!!!!!!!!!!!!загрузка условия заврешена");
      }
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 10) { /////////////////////////////передвинуть сл///////////////////////////////////////////


      float payload = get_new_pin_value(that_condtion_widget);//узнаем какой уровень на пине который опрашиваем
      unsigned char minTemp, maxTemp, button_;
      if (payload != 0) {
        char * pEnd;
        String inputString = actBtn_a_ch_string(that_condtion_widget, that_number_cond);
        /////////////
        uint8_t max_values = 3;
        uint8_t values_back[max_values];
        parseStringToArray(actBtn_a_ch_string(that_condtion_widget, that_number_cond), values_back, max_values);
        minTemp = values_back[0];
        maxTemp = values_back[1];
        button_ = values_back[2];

        payload = ((payload - minTemp * 1.0) * (1024.0)) / (maxTemp  - minTemp) * 1.0;
        payload = payload < 0 ? 0 : payload;
        payload = payload > 1024 ? 1024 : payload;

        //uint8_t id_button = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], NULL, 10);
        callback_scoket(button_, payload);
      }
    }
  }
}


