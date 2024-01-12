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


//AlarmId alalrmId;
//uint8_t tID_a[Condition][Numbers];
bool En_a[Condition][Numbers];
uint8_t type_a[Condition][Numbers];
//short int timer_a[Condition][Numbers];
//uint8_t timerType_a[Condition][Numbers];//надо переделать в int
uint8_t act_a[Condition][Numbers];
//String actBtn_a[Condition][Numbers];
char actBtn_a_ch[Condition][Numbers][20];

int times[Condition][Numbers];//время в формате часы*60+минуты.
//String dates[Condition][Numbers];
char bySignal[Condition][Numbers];

//int pwmTypeAct[Condition][Numbers];
char actOn_a[Condition][Numbers];//надо переделать в int
char NumberIDs[Condition];
bool alarm_is_active[Condition][Numbers];
//bool opposite_action[Condition][Numbers];
//AlarmId idA[Condition][Numbers];
unsigned int update_alarm_active, check_bySignal_variable = 0;
//uint8_t alarmRepeats = 255;

bool timer_alarm_action_switch = 0;
unsigned char timer_alarm_action = 0, timer_alarm_action_max = 20;

void setup_alarm() {

  for (char i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
    char thatCondition = i1;//idWidget кнопка
    NumberIDs[thatCondition] = 0;
    String NameFile = "Condition" + String(thatCondition, DEC);
    String jsonCondition = readCommonFiletoJson(NameFile);
    jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
  }
  Serial.println("setup_alarm() - OK");
}
String saveConditiontoJson(char CondWidjet) {
  DynamicJsonDocument jsonDocument(2048); // Adjust the capacity as needed
  JsonObject json = jsonDocument.to<JsonObject>();

  json["ID"] = String(CondWidjet, DEC);
  json["Numbers"] = String(NumberIDs[CondWidjet], DEC);

  JsonArray En_json = json.createNestedArray("En");
  JsonArray times_json = json.createNestedArray("times");
  JsonArray bySignal_json = json.createNestedArray("bySignal");
  JsonArray bySignalPWM_json = json.createNestedArray("bySignalPWM");
  JsonArray type_json = json.createNestedArray("type");
  JsonArray act_json = json.createNestedArray("act");
  JsonArray actBtn_json = json.createNestedArray("actBtn");
  JsonArray actOn_json = json.createNestedArray("actOn");

  for (char i = 0; i < (int)NumberIDs[CondWidjet]; i++) {
    En_json.add(En_a[CondWidjet][i]);
    times_json.add(times[CondWidjet][i]);
    bySignal_json.add((int)bySignal[CondWidjet][i]);
    bySignalPWM_json.add(bySignalPWM[CondWidjet][i]);
    type_json.add(type_a[CondWidjet][i]);
    act_json.add(act_a[CondWidjet][i]);
    actBtn_json.add(actBtn_a_ch[CondWidjet][i]);
    actOn_json.add(actOn_a[CondWidjet][i]);

    alarm_is_active[CondWidjet][i] = false;
  }

  String buffer;
  serializeJson(json, buffer);
  saveCommonFiletoJson("Condition" + String(CondWidjet, DEC), buffer, 1);
  return buffer;
}


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
      strncpy(actBtn_a_ch[thatCondition][i], rootjs["actBtn"][i], sizeof(actBtn_a_ch[thatCondition][i]));
      char actOn_that = rootjs["actOn"][i];

      unsigned int times_that = rootjs["times"][i];
      int bySignal_that = rootjs["bySignal"][i];
      int bySignalPWM_that = rootjs["bySignalPWM"][i];
      bool En_that = rootjs["En"][i];

      type_a[thatCondition][i] = 0;
      type_a[thatCondition][i] = type_that;
      act_a[thatCondition][i] = act_that;
      actOn_a[thatCondition][i] = actOn_that;

      times[thatCondition][i] = times_that;
      bySignal[thatCondition][i] = (char)bySignal_that;
      bySignalPWM[thatCondition][i] = bySignalPWM_that;
      En_a[thatCondition][i] = En_that;

      alarm_is_active[thatCondition][i] = alarm_is_active[thatCondition][i] ^ true;

#if defined(pubClient)
      if (client.connected()) {
        if ((bySignal[thatCondition][i] == 2) || (bySignal[thatCondition][i] == 3)) {
          Serial.println("POSSIBLE PUBLISH bySignalPWM[c][n]:" + String(bySignalPWM[thatCondition][i], DEC));
          char pubstatus[40];
          sprintf(pubstatus, "%s/PLUS/%d/%d", deviceID, thatCondition, i);
          pubStatus(pubstatus, setStatus(bySignalPWM[thatCondition][i]));
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
    if (type_a[idWidget][i] == 3) {//таймер
      unsigned int nowsec = (second() + minute() * 60) + (hour() * 3600);

      times[idWidget][i] = nowsec + times[idWidget][i];
      type_a[idWidget][i] = type_a[idWidget][i] + 10; //заведен
      //idA[idWidget][i] = Alarm.timerOnce( timer_a[idWidget][i] * multiply, OnceOnly);
      Serial.println("время сейчас:" + String( nowsec / 3600, DEC) + ":" + String( nowsec % 3600 / 60, DEC) + ":" + String( nowsec % 60, DEC) );
      Serial.println("установлен таймер:" + String( times[idWidget][i] / 3600, DEC) + ":" + String( times[idWidget][i] % 3600 / 60, DEC) + ":" + String( times[idWidget][i] % 60, DEC) );
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
        //if ((type_a[idWidget][i] == 1) || (type_a[idWidget][i] == 3)) { //по достижению времени
        // Serial.print("nowsec:");
        // Serial.print(nowsec);
        // Serial.print(" times[idWidget][i]:");
        // Serial.println(times[idWidget][i]);
        //if (nowsec == times[idWidget][i]) {
        if ((type_a[idWidget][i] == 1) || (type_a[idWidget][i] == 13)) {
          if ((nowsec == times[idWidget][i]) || (test_action)) {
            test_action = false;
            //Serial.println("событие по времени: " + String(times[idWidget][i], DEC) + " сейчас:" + String(nowsec, DEC));
            // bool opposite = (bool)String(delimeter(actBtn_a_ch[idWidget][i], ":", 3)).toInt();
            char* pEnd;
            bool one = strtol(actBtn_a_ch[idWidget][i], &pEnd, 10),
                 two = strtol(pEnd, &pEnd, 10),
                 three = strtol(pEnd, &pEnd, 10),
                 four = strtol(pEnd, &pEnd, 10),
                 opposite = strtol(pEnd, NULL, 10);


            //Serial.print("opposite");
            //Serial.println(opposite);

            make_action(idWidget, i, opposite);
            if ((type_a[idWidget][i] == 13)) { //таймер
              type_a[idWidget][i] = type_a[idWidget][i] - 10;
              //загружаем условие
              String NameFile = "Condition" + String(idWidget, DEC);
              String jsonCondition = readCommonFiletoJson(NameFile);
              jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
              //switch_action(idWidget, i, 1);
              Serial.println("!!!!!!!!!!!!!!!!!!!!загрузка условия заврешена");
            }
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
  if (onesec > check_bySignal_variable + 1 ) {
    check_for_changes();
    check_bySignal_variable = onesec;
#if defined(pubClient)
    subscr_loop_PLUS();
#endif
  }
  if (check_internet) {
    if (onesec > update_alarm_active + 21600 ) { //каждые 6 часов
      CheckInternet("worldclockapi.com/api/json/utc/now");
      for (uint8_t i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
        uint8_t idWidget = i1;
        for (uint8_t i = 0; i < Numbers; i++) {//от всего колимчества таймеров
          //если это не "переключить условие" То всегда будет срабатывать opposite
        }
      }
      update_alarm_active = onesec;
    }
  }
  /*
    if (onesec_240 > check_my_alarm + 59 ) {//каждую минуту
      Serial.println("min");

      Serial.println(check_my_alarm);
      check_my_alarm = onesec_240;
    }
  */

  if (minute() != l_minute) {

    l_minute = minute();
  }
}
void CheckInternet(String request) {
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
      if ((bySignal[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 2) || (bySignal[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 3)) {
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
        if (type_a[idWidget][i] == 2) {//по уровню
          if ((bySignal[idWidget][i] == 0) || (bySignal[idWidget][i] == 1)) {//out,in
            //stat[idWidget] = digitalRead(pin[idWidget]);
            stat[idWidget] = (int) get_new_pin_value(idWidget);
            //Serial.println(digitalRead(4), DEC);
            if (bySignal[idWidget][i] == 0) { //положительный
              stat[idWidget] == 1 ? MakeIfTrue(idWidget, i) : MakeIfFalse(idWidget, i);
            }
            else if (bySignal[idWidget][i] == 1) { //отрицательный
              stat[idWidget] == 0 ? MakeIfTrue(idWidget, i) : MakeIfFalse(idWidget, i);
            }
          }
          if ((bySignal[idWidget][i] == 2) || (bySignal[idWidget][i] == 3)) {//adc
            if (pinmode[idWidget] == 4) {//adc
              //stat[idWidget] = ((analogRead(pin[idWidget]) / analogDivider) + analogSubtracter);
              stat[idWidget] = (int) get_new_pin_value(idWidget);
            }
            if ((pinmode[idWidget] == 3) || (pinmode[idWidget] == 5) || (pinmode[idWidget] == 6) || (pinmode[idWidget] == 8)) {
              //stat[idWidget] = (analogRead(pin[idWidget]) / analogDivider) + analogSubtracter);
            }
            if (bySignal[idWidget][i] == 2) { //больше adc
              stat[idWidget]  > bySignalPWM[idWidget][i] ? MakeIfTrue(idWidget, i) : MakeIfFalse(idWidget, i);
            }
            else if (bySignal[idWidget][i] == 3) { //меньше adc
              stat[idWidget]  < bySignalPWM[idWidget][i] ? MakeIfTrue(idWidget, i) : MakeIfFalse(idWidget, i);
            }

          }
          if (bySignal[idWidget][i] == 4) { //делитель
            make_action(idWidget, i, false);
          }
          /*
            if (bySignal[idWidget][i] == 0) { //положительный
            if (digitalRead(pin[idWidget]) == 1) {
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
            else if (bySignal[idWidget][i] == 1) { //отрицательный
            if (digitalRead(pin[idWidget]) == 0) {
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


        }
      }
      // }
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
void OnceOnly() {
  //AlarmId idAlarm = Alarm.getTriggeredAlarmId();
  //  Serial.println ("Alarm Triggered. ID:" + String(idAlarm, DEC));
  uint8_t that_condtion_widget = 255;
  uint8_t that_number_cond = 255;
  /*
    for (uint8_t i1 = 0; i1 < Condition; i1++) {
    uint8_t Numbers_that = NumberIDs[i1];
    for (uint8_t i = 0; i < Numbers_that; i++) {//о
      if (idA[i1][i] == idAlarm) {
        that_condtion_widget = i1;
        that_number_cond = i;
        Serial.println("that_condtion_widget:" + String(that_condtion_widget, DEC) + "that_number_cond:" + String(that_number_cond, DEC));
        break; break;
      }

    }
    }
  */
  if ((that_condtion_widget != 255) && (that_number_cond != 255)) {
    make_action(that_condtion_widget, that_number_cond, false);
  }
  else {
    Serial.println("Timer ERROR id:" + String(that_condtion_widget, DEC) + " numberCondtioins:" + String(that_number_cond, DEC));
    return;
  }

  //Serial.println("TimerID :" + String(idAlarm, DEC) + " tIDa" + String(tID_a[that_condtion_widget][that_number_cond], DEC) + " type_a:" + String(type_a[that_condtion_widget][that_number_cond], DEC) + " actBtn_a:" + String(actBtn_a_ch[that_condtion_widget][that_number_cond]));
  //alarm_is_active = false;
  // use Alarm.free() to disable a timer and recycle its memory.
  // Alarm.free(alalrmId);
  // optional, but safest to "forget" the ID after memory recycled
  //alalrmId = dtINVALID_ALARM_ID;
  // you can also use Alarm.disable() to turn the timer off, but keep
  // it in memory, to turn back on later with Alarm.enable().
}
void Repeats() {
  //AlarmId idAlarm = Alarm.getTriggeredAlarmId();
  //Serial.println ("Alarm Triggered. ID:" + String(idAlarm, DEC));
}
void disable_En(uint8_t that_condtion_widget, uint8_t that_number_cond) {
  for (uint8_t i1 = 0; i1 < Condition; i1++) {
    for (uint8_t i = 0; i < NumberIDs[i1]; i++) {
      //Serial.println("ищем условие, которое тоже задействовано на эту кнопку" + String(i1) + String(i));
      //Serial.println("проверяем:" +String(i1)+String(i)+ String(actBtn_a[i1][i]) + " " + String(actBtn_a[that_condtion_widget][that_number_cond]));
      //if (actBtn_a[that_condtion_widget][that_number_cond] ==  actBtn_a[i1][i]) {
      if (strcmp (actBtn_a_ch[that_condtion_widget][that_number_cond], actBtn_a_ch[i1][i]) == 0) {
        Serial.println("схожее:" + String(i1) + String(i) + String(actBtn_a_ch[i1][i]) + " " + String(actBtn_a_ch[that_condtion_widget][that_number_cond]));
        if ((that_number_cond != i) || (that_condtion_widget != i1)) {
          //Нашли совпадение, и если это не то, что выполнено, то отключаем его
          Serial.println("!!!!!!!!!!!!!!отключаем условие, которое тоже задействовано на эту кнопку" + String(i1, DEC) + String(i, DEC));
          En_a[i1][i] = false;
        }
      }

    }

  }

}
void make_action(uint8_t that_condtion_widget, uint8_t that_number_cond, bool opposite) {
  if (En_a[that_condtion_widget][that_number_cond] == true) {//если он включен
    if (act_a[that_condtion_widget][that_number_cond] == 2) { ////////////////////////////"нажать кнопку"////////////////////////////////////////////
      uint8_t i = that_condtion_widget;
      uint8_t payload_is = actOn_a[that_condtion_widget][that_number_cond];
      uint8_t id_button = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], NULL, 10);
      switch_action(id_button, that_number_cond, opposite);
    }
    else if ((act_a[that_condtion_widget][that_number_cond] == 4) && (!opposite)) { //"отправить Email"//////////////////////////////////////////////////////////
      String buffer;
      buffer += String(actBtn_a_ch[that_condtion_widget][that_number_cond]); //сообщение в условии
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
      uint8_t thatpin ;//= atoi(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      thatpin = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], NULL, 10);
      int new_value = actOn_a[that_condtion_widget][that_number_cond];

    }
    else if (act_a[that_condtion_widget][that_number_cond] == 5) { //переключить условие/////////////////////////////////////////////////////////
      short int StatusEn[2];

      char* pEnd;
      StatusEn[0] = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], &pEnd, 10);
      StatusEn[1] = strtol(pEnd,    NULL, 10);
      uint8_t on_off = actOn_a[that_condtion_widget][that_number_cond];
      if (type_a[that_condtion_widget][that_number_cond] == 1) { //будильник
        disable_En(StatusEn[0], StatusEn[1]);
      }
      //если не таймер
      if (on_off == 1) {//включить условие
        En_a[StatusEn[0]][StatusEn[1]] = 1 ^ opposite;
      } else if (on_off == 2) {//выключить условие
        En_a[StatusEn[0]][StatusEn[1]] = 0 ^ opposite;
      }
      if (En_a[StatusEn[0]][StatusEn[1]]) {
        Serial.println("вкл кнопки:" + String(StatusEn[0], DEC) + " номер:" + String(StatusEn[1], DEC) + "положение:" + String(En_a[StatusEn[0]][StatusEn[1]], DEC) + " opposite:" + String(opposite, DEC));
      } else {
        Serial.println("выкл кнопки:" + String(StatusEn[0], DEC) + " номер:" + String(StatusEn[1], DEC) + "положение:" + String(En_a[StatusEn[0]][StatusEn[1]], DEC) + " opposite:" + String(opposite, DEC));
      }
      //check_if_there_timer_times(that_condtion_widget);
      check_if_there_timer_once(that_condtion_widget);//устанавливаем таймер,если такой есть

      if (type_a[StatusEn[0]][StatusEn[1]] == 3) {//таймер
        if (En_a[StatusEn[0]][StatusEn[1]]) {
          //En_a[StatusEn[0]][StatusEn[1]] = true;
          Serial.println("выполняем обратную установку таймера:");
          make_action(StatusEn[0], StatusEn[1], true);
          //idA[StatusEn[0]][StatusEn[1]] = Alarm.timerOnce( timer_a[StatusEn[0]][StatusEn[1]], OnceOnly);
          //Serial.println("заводим таймер:" + String( timer_a[StatusEn[0]][StatusEn[1]], DEC) + " sec" + " idAlarm:" + String(idA[StatusEn[0]][StatusEn[1]], DEC));
        }
      }
      //сохранить текущее состояние
      saveConditiontoJson(StatusEn[0]);
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 3) { ///////////////////////////удаленная кнопка///////////////////////////////////////////////////

      String host = String(actBtn_a_ch[that_condtion_widget][that_number_cond]);//узнаем хост и кнопку
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
      DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
      String mqtt_parse = String(actBtn_a_ch[that_condtion_widget][that_number_cond]);
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

#ifdef pubClient
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
      //String openPath = "ws8211/" + String(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      char openPath[11] = "ws8211/";
      strcat(openPath, actBtn_a_ch[that_condtion_widget][that_number_cond]);
      String DataLoad_8211 = readCommonFiletoJson(String(openPath));
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
#if defined(ws2811_include)
      LoadData(buffer);//include ws2811.in
#endif
      //LoadData(DataLoad_8211.c_str());
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 8) { /////////////////////////WakeOnLan///////////////////////////
      //char addresWakePC[20];
      //strcpy(addresWakePC,actBtn_a_ch[that_condtion_widget][that_number_cond])
      wakeMyPC(actBtn_a_ch[that_condtion_widget][that_number_cond]);

    }
    else if (act_a[that_condtion_widget][that_number_cond] == 9) { /////////////////////////timer///////////////////////////
      switch_action(that_condtion_widget, that_number_cond, opposite);
      // char numbers[50];
      //strcpy(numbers, actBtn_a_ch[that_condtion_widget][that_number_cond]);
      char * pEnd;
      bool  typeOpposite_int;

      unsigned char typePinsDuration_int = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], &pEnd, 10), // преобразовать первую часть строки в значение 10-й СС
                    typePinsTimeChoise_int = strtol(pEnd,    &pEnd, 10), // преобразовать часть строки в значение 16-й СС
                    typePinsrepeats_int = strtol(pEnd,    &pEnd,  10),
                    typeDelay_int = strtol(pEnd,    &pEnd,  10);
      typeOpposite_int = (bool)strtol(pEnd,     NULL,  10);


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
          times[that_condtion_widget][that_number_cond] = times[that_condtion_widget][that_number_cond] + typePinsDuration_int * typePinsTimeChoise_int_mult;
        } else {
          times[that_condtion_widget][that_number_cond] = times[that_condtion_widget][that_number_cond] + typeDelay_int * 60;
        }
        //opposite_action[that_condtion_widget][that_number_cond] = true;
        typeOpposite_int = typeOpposite_int ^ 1;
        typePinsrepeats_int--;
        sprintf(actBtn_a_ch[that_condtion_widget][that_number_cond], "%d %d %d %d %d", typePinsDuration_int, typePinsTimeChoise_int, typePinsrepeats_int, typeDelay_int, typeOpposite_int);
      }
      else if (typePinsrepeats_int == 255) {
        String NameFile = "Condition" + String(that_condtion_widget, DEC);
        String jsonCondition = readCommonFiletoJson(NameFile);
        jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
        switch_action(that_condtion_widget, that_number_cond, 1);
        Serial.println("!!!!!!!!!!!!!!!!!!!!загрузка условия заврешена");
      }
    }

    else if (act_a[that_condtion_widget][that_number_cond] == 10) { /////////////////////////////передвинуть сл///////////////////////////////////////////


      float payload = get_new_pin_value(that_condtion_widget);//узнаем какой уровень на пине который опрашиваем
      unsigned char minTemp, maxTemp, button_;
      if (payload != 0) {
        if (times[that_condtion_widget][that_number_cond] == -1) {
          char * pEnd;
          minTemp = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], &pEnd, 10), // преобразовать первую часть строки в значение 10-й СС //minTemp
          maxTemp = strtol(pEnd,    &pEnd, 10), // преобразовать часть строки в значение 16-й СС //maxTemp
          button_ = strtol(pEnd,    &pEnd,  10); //button_
          //сейчас нужно переписать формулу (1024 / (maxTemp - minTemp)); что бы освободить переменную maxTemp

        }
        //        Serial.print("minTemp:"); Serial.println(minTemp);
        //        Serial.print("maxTemp:"); Serial.println(maxTemp);
        //        Serial.print("button_:"); Serial.println(button_);

        payload = ((payload - minTemp * 1.0) * (1024.0)) / (maxTemp  - minTemp) * 1.0;
        payload = payload < 0 ? 0 : payload;
        payload = payload > 1024 ? 1024 : payload;


        //uint8_t id_button = strtol(actBtn_a_ch[that_condtion_widget][that_number_cond], NULL, 10);
        callback_scoket(button_, payload);
      }
    }
  }
}
void switch_action(uint8_t that_condtion_widget, uint8_t that_number_cond, bool opposite) {
  switch (actOn_a[that_condtion_widget][that_number_cond]) {
    case 1://вкл
      callback_scoket(that_condtion_widget, 1 ^ opposite);
      break;
    case 2://выкл
      callback_scoket(that_condtion_widget, 0 ^ opposite);
      break;
    case 3://pwm
      //callback_scoket(that_condtion_widget, pwmTypeAct[that_condtion_widget][that_number_cond] );
      break;
  }
}

