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
#include <TimeAlarms.h>


AlarmId alalrmId;
uint8_t tID_a[Condition][Numbers];
bool En_a[Condition][Numbers];
uint8_t type_a[Condition][Numbers];
int timer_a[Condition][Numbers];
uint8_t timerType_a[Condition][Numbers];//надо переделать в int
uint8_t act_a[Condition][Numbers];
//String actBtn_a[Condition][Numbers];
char actBtn_a_ch[Condition][Numbers][20];

int times[Condition][Numbers];//время в формате часы*60+минуты.
//String dates[Condition][Numbers];
char bySignal[Condition][Numbers];

int pwmTypeAct[Condition][Numbers];
char actOn_a[Condition][Numbers];//надо переделать в int
char NumberIDs[Condition];
bool alarm_is_active[Condition][Numbers];
AlarmId idA[Condition][Numbers];
unsigned int update_alarm_active, check_bySignal_variable = 0;

void setup_alarm() {
  for (char i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
    for (char i = 0; i < Numbers; i++) {//пробегаемся по всем кнопкам
      idA[i1][i] = -1;//обнуляем все будильники
      Alarm.free(idA[i1][i]);
    }
  }

  for (char i1 = 0; i1 < Condition; i1++) {//пробегаемся по всем кнопкам
    char thatCondition = i1;//idWidget кнопка
    NumberIDs[thatCondition] = 0;
    String NameFile = "Condition" + String(thatCondition, DEC);
    String jsonCondition = readCommonFiletoJson(NameFile);
    jsonCondition != "" ? load_Current_condition(jsonCondition) : false;
  }

}
String saveConditiontoJson(char CondWidjet) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["ID"] = String(CondWidjet, DEC);
  json["Numbers"] = String(NumberIDs[CondWidjet], DEC);

  JsonArray& tID_json = json.createNestedArray("tID");
  JsonArray& En_json = json.createNestedArray("En");
  JsonArray& times_json = json.createNestedArray("times");
  //JsonArray& dates_json = json.createNestedArray("dates");
  JsonArray& bySignal_json = json.createNestedArray("bySignal");
  JsonArray& bySignalPWM_json = json.createNestedArray("bySignalPWM");
  JsonArray& type_json = json.createNestedArray("type");
  JsonArray& timer_json = json.createNestedArray("timer");
  //JsonArray& timerType_json = json.createNestedArray("timerType");
  JsonArray& act_json = json.createNestedArray("act");
  JsonArray& actBtn_json = json.createNestedArray("actBtn");
  JsonArray& actOn_json = json.createNestedArray("actOn");
  JsonArray& pwmTypeAct_json = json.createNestedArray("pwmTypeAct");


  // Serial.println("NUmbersID: " + String(NumberIDs[0], DEC));
  for (char i = 0; i < (int)NumberIDs[CondWidjet]; i++) {
    tID_json.add(tID_a[CondWidjet][i]);
    En_json.add(En_a[CondWidjet][i]);
    times_json.add(times[CondWidjet][i]);
    bySignal_json.add((int)bySignal[CondWidjet][i]);
    bySignalPWM_json.add(bySignalPWM[CondWidjet][i]);//correct
    type_json.add( type_a[CondWidjet][i]);//correct
    timer_json.add(timer_a[CondWidjet][i]);
    act_json.add( act_a[CondWidjet][i]);//correct
    //actBtn_json.add(actBtn_a[CondWidjet][i]);//correct
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    actBtn_json.add(actBtn_a_ch[CondWidjet][i]);//correct
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!
    actOn_json.add( actOn_a[CondWidjet][i]);//correct
    pwmTypeAct_json.add( pwmTypeAct[CondWidjet][i]);//correct

    alarm_is_active[CondWidjet][i] = false;

  }
  String buffer;
  json.printTo(buffer);
  //Serial.println(buffer);
  saveCommonFiletoJson("Condition" + String(CondWidjet, DEC), buffer, 1);
  return buffer;
}

bool load_Current_condition(String jsonCondition) {
  //String jsonCondition = LoadCondition(thatCondition);//загружаем условия кнопки 0;

  //Serial.println(jsonCondition);
  if (jsonCondition != "") {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& rootjs = jsonBuffer.parseObject(jsonCondition);
    if (!rootjs.success()) {
      Serial.println("PARSE FAIL!!");
      return false;

    }
    char WidjetIds = rootjs["ID"];//1//номер кнопки
    int Numbers_that = rootjs["Numbers"];//1//в одном файле кол-во таймеров
    char thatCondition = WidjetIds;
    Numbers_that > Numbers ? Numbers_that = Numbers : true;
    for (char i = 0; i < Numbers_that; i++) {//от всего колимчества таймеров
      unsigned char tID_that = rootjs["tID"][i];
      char type_that = rootjs["type"][i];
      int timer_that = rootjs["timer"][i];
      char timerType_that = rootjs["timerType"][i];
      char act_that = rootjs["act"][i];
      String actBtn_that = rootjs["actBtn"][i];

      int times_that = rootjs["times"][i];
      int bySignal_that = rootjs["bySignal"][i];
      int bySignalPWM_that = rootjs["bySignalPWM"][i];
      int pwmTypeAct_that = rootjs["pwmTypeAct"][i];
      bool En_that = rootjs["En"][i];
      char actOn_that = rootjs["actOn"][i];
      //default
      type_a[thatCondition][i] = 0;
      tID_a[thatCondition][i] = tID_that;
      type_a[thatCondition][i] = type_that;
      timer_a[thatCondition][i] = timer_that;
      timerType_a[thatCondition][i] =  timerType_that;
      act_a[thatCondition][i] = act_that;
      //actBtn_a[thatCondition][i] = actBtn_that;
      strcpy(actBtn_a_ch[thatCondition][i], rootjs["actBtn"][i]);
      actOn_a[thatCondition][i] = actOn_that;

      times[thatCondition][i] = times_that;
      bySignal[thatCondition][i] = (char)bySignal_that;
      bySignalPWM[thatCondition][i] = bySignalPWM_that;
      pwmTypeAct[thatCondition][i] = pwmTypeAct_that;
      En_a[thatCondition][i] = En_that;

      alarm_is_active[thatCondition][i] = alarm_is_active[thatCondition][i] ^ true;
      if (client.connected())  {
        if ((bySignal[thatCondition][i] == 2) || (bySignal[thatCondition][i] == 3)) {
          //Serial.println("POSSIBLE PUBLISH bySignalPWM[c][n]:" + String(bySignalPWM[thatCondition][i], DEC));
          char pubstatus[40];
          char buffer[10];
          deviceID.toCharArray(buffer, sizeof(deviceID));
          sprintf(pubstatus, "%s/PLUS/%d/%d", buffer, thatCondition, i);
          pubStatus(pubstatus, setStatus(bySignalPWM[thatCondition][i]));

        }
      }
    }
    NumberIDs[thatCondition] = Numbers_that;//количество в этом условии (на этой кнопке);
    NumberIDs[thatCondition] > 10 ? NumberIDs[thatCondition] = 0 : true;
    check_if_there_timer_times(thatCondition);
  }
  return true;
}
void check_if_there_timer_once(int idWidget) {//проверка установки таймера

  for (uint8_t i = 0; i < NumberIDs[idWidget]; i++) {
    if (type_a[idWidget][i] == 3) {//таймер//timerType_a
      int multiply = 1;
      switch (timerType_a[idWidget][i]) {
        case 1://минут
          multiply = 60;
          break;
        case 2://часов
          multiply = 3600;
          break;
      }
      idA[idWidget][i] = Alarm.timerOnce( timer_a[idWidget][i] * multiply, OnceOnly);
      Serial.println("установлен таймер:" + String( timer_a[idWidget][i]*multiply, DEC) + " sec" + " idAlarm:" + String(idA[idWidget][i], DEC));
    }

  }
}
void check_if_there_load_times(int thatCondition) {
  String NameFile = "Condition" + String(thatCondition);
  String jsonCondition = readCommonFiletoJson(NameFile);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootjs = jsonBuffer.parseObject(jsonCondition);
  if (!rootjs.success()) {
    Serial.println("PARSE FAIL!!");
  }
  int Numbers_that = rootjs["Numbers"];//1//в одном файле кол-во таймеров
  unsigned int nowsec = (minute()) + (hour() * 60);
  for (char i = 0; i < Numbers_that; i++) {//от всего колимчества таймеров
    int times_that = rootjs["times"][i];
    if (nowsec == times[thatCondition][i]) {
      Serial.println("событие по времени: " + String(times[thatCondition][i], DEC) + " сейчас:" + String(times[thatCondition][i], DEC));
      make_action(thatCondition, i, false);
    }
  }
}
void check_if_there_next_times() {//вызывается каждую минуту
  unsigned int nowsec = (minute()) + (hour() * 60);
  for (int idWidget = 0; idWidget < Condition; idWidget++) {//пробегаемся по всем кнопкам
    for (unsigned char i = 0; i < Numbers; i++) {
      if (En_a[idWidget][i]) {
        if (type_a[idWidget][i] == 1) {//по достижению времени
          if (nowsec == times[idWidget][i]) {
            Serial.println("событие по времени: " + String(times[idWidget][i], DEC) + " сейчас:" + String(times[idWidget][i], DEC));
            make_action(idWidget, i, false);
          }
        }
      }
    }
  }
}
void check_if_there_timer_times(uint8_t idWidget) {//проверка установки буильника
  for (uint8_t i = 0; i < NumberIDs[idWidget]; i++) {
    if (type_a[idWidget][i] == 1) {//по достижению времени
      int *Time_alarm = getTime_min_hour(times[idWidget][i]);
      idA[idWidget][i] = Alarm.alarmRepeat(Time_alarm[0] , Time_alarm[1], 0, OnceOnly); //H:M
      Serial.println("Cond type by time: " + String( Time_alarm[0], DEC) + ":" + String( Time_alarm[1], DEC) + " H:M" + " idAlarm:" + String(idA[idWidget][i], DEC));
      //если текущее время больше установленного
      unsigned int nowsec = (minute()) + (hour() * 60);

      if (times[idWidget][i] < nowsec) {
        Serial.println("уже установлено: " + String(times[idWidget][i], DEC) + "<" + String(nowsec, DEC));
        make_action(idWidget, i, false);
      }
      //}
      //else {//с датой
      // }
    }
  }
}
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
void loop_alarm() {

  //if (alarm_is_active) {
  Alarm.delay(0); // wait one second between clock display
  //}
  if (onesec > check_bySignal_variable + 1 ) {
    check_for_changes();
    check_bySignal_variable = onesec;
    subscr_loop_PLUS();
  }
  if (check_internet) {
    if (onesec > update_alarm_active + 600 ) { //каждые 10 мин
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
}
void CheckInternet(String request) {
  String respond = getHttp(request);
  if (respond == "fail") { //интернета нет
    Serial.println("Интернета нет");
    relayRouter();
  } else { //интернет есть
    DynamicJsonBuffer jsonBuffer;
    JsonObject& rootjs = jsonBuffer.parseObject(respond);

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
      setup_alarm();
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
char subscr_loop_PLUS_i = 0;
char subscr_loop_PLUS_i1 = 0;

void subscr_loop_PLUS() {
  if ((!client.connected()) || (!IOT_Manager_loop))  return;
  if (subscr_loop_PLUS_i <= 3) {
    if (subscr_loop_PLUS_i1 <= NumberIDs[subscr_loop_PLUS_i]) {
      //  if (bySignalPWM[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] != -1) {
      if ((bySignal[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 2) || (bySignal[subscr_loop_PLUS_i][subscr_loop_PLUS_i1] == 3)) {
        String __topic_subscr = deviceID + "/PLUS/" + String(subscr_loop_PLUS_i, DEC) + "/" + String(subscr_loop_PLUS_i1, DEC) + "/status";
        if (!client.subscribe(__topic_subscr.c_str())) {
          Serial.println("Client subscribe FAIL!:" + __topic_subscr);
        }
        else {
          Serial.println("Client subscribe SUCCSESS!:" + __topic_subscr);
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
void check_for_changes() {
  //Serial.println("check");
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



// functions to be called when an alarm triggers:
void MakeIfTrue(uint8_t idWidget, uint8_t i) {
  if (alarm_is_active[idWidget][i]) {
    make_action(idWidget, i, false);
    alarm_is_active[idWidget][i] = false;
  }
}
void MakeIfFalse(char idWidget, char i) {
  if (!alarm_is_active[idWidget][i]) {
    if (act_a[idWidget][i] != 5 ||  7 || 8) { // переключить условие//act_a[i1][i] != 4)(act_a[idWidget][i] != 7)||(act_a[idWidget][i] != 8))
      make_action(idWidget, i, true);
    }
    alarm_is_active[idWidget][i] = true;
    // alarm_is_active[idWidget][i] = true;
    //Serial.println("opposite adc");
  }
}
void OnceOnly() {
  AlarmId idAlarm = Alarm.getTriggeredAlarmId();
  Serial.println ("Alarm Triggered. ID:" + String(idAlarm, DEC));
  uint8_t that_condtion_widget = 255;
  uint8_t that_number_cond = 255;
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
  if ((that_condtion_widget != 255) && (that_number_cond != 255)) {
    make_action(that_condtion_widget, that_number_cond, false);
  }
  else {
    Serial.println("Timer ERROR id:" + String(that_condtion_widget, DEC) + " numberCondtioins:" + String(that_number_cond, DEC));
    return;
  }

  Serial.println("TimerID :" + String(idAlarm, DEC) + " tIDa" + String(tID_a[that_condtion_widget][that_number_cond], DEC) + " type_a:" + String(type_a[that_condtion_widget][that_number_cond], DEC) + " actBtn_a:" + String(actBtn_a_ch[that_condtion_widget][that_number_cond]));
  //alarm_is_active = false;
  // use Alarm.free() to disable a timer and recycle its memory.
  // Alarm.free(alalrmId);
  // optional, but safest to "forget" the ID after memory recycled
  //alalrmId = dtINVALID_ALARM_ID;
  // you can also use Alarm.disable() to turn the timer off, but keep
  // it in memory, to turn back on later with Alarm.enable().
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
  Serial.println("выполняем действие condtion:" + String(that_condtion_widget, DEC) + " number:" + String(that_number_cond, DEC) + " En:" + String(En_a[that_condtion_widget][that_number_cond], DEC) + " opp:" + String(opposite));
  if (En_a[that_condtion_widget][that_number_cond] == true) {//если он включен
    // Serial.println("that_condition:" + String(that_condtion_widget) + " that_number_cond:" + String(that_number_cond));
    if (act_a[that_condtion_widget][that_number_cond] == 2) { ////////////////////////////"нажать кнопку"////////////////////////////////////////////
      uint8_t i = that_condtion_widget;
      uint8_t payload_is = actOn_a[that_condtion_widget][that_number_cond];
      uint8_t id_button = 255;
      for (uint8_t i2 = 0; i2 < nWidgets; i2++) {
        if (strcmp (descr[i2].c_str(), actBtn_a_ch[that_condtion_widget][that_number_cond]) == 0) {
          //if (descr[i2] == actBtn_a[that_condtion_widget][that_number_cond]) {
          id_button = i2;
          Serial.println("найдена кнопка:" + String(actBtn_a_ch[that_condtion_widget][that_number_cond])) ;
          break;
        }
      }
      if (id_button != 255) {
        if (payload_is == 2) {//выкл
          stat[id_button] = 1 ^ opposite;
          stat[id_button] ^= defaultVal[id_button];
          /*
            if (opposite) {
            stat[id_button] = 1;
            }
          */
          digitalWrite(pin[id_button], stat[id_button]);
          Serial.println("выкл:" + String(opposite, DEC)) ;
        } else if (payload_is == 1) { //вкл
          stat[id_button] = 0 ^ opposite;
          stat[id_button] ^= defaultVal[id_button];
          /*
            if (opposite) {
            stat[id_button] = 0;
            }
          */
          digitalWrite(pin[id_button], stat[id_button]);
          Serial.println("вкл:" + String(opposite, DEC)) ;
        }
        else if (payload_is == 3) { //шим
          // int pinNumber = actBtn_a[that_condtion_widget][that_number_cond].toInt();
          stat[id_button] = pwmTypeAct[that_condtion_widget][that_number_cond];
          analogWrite(pin[id_button], stat[id_button]);
          //pwm value
          Serial.println("pwm pin:" + String(pin[id_button], DEC) + "val" + String(stat[id_button], DEC)) ;
        }
        //для вкл или выкл

        switch (pinmode[id_button]) {
          case 2://out
            //digitalWrite(pinNumber, newValue);
            //Serial.println("digitalWrite:" + String(pin[id_button]) + String(newValue)) ;
            break;
          case 6://IR
            Serial.println("IR") ;
            //////////////////////сюда вставить отправку кода IR
            //
            break;
        }
        //pubStatusWS(sTopic[id_button], setStatus(stat[id_button]), true);
      }
      else {
        Serial.println("кнопка не найдена") ;
      }
      //saveSPIFFS_jsonArray(stat);
    }
    else if ((act_a[that_condtion_widget][that_number_cond] == 4) && (!opposite)) { //"отправить Email"//////////////////////////////////////////////////////////
      String buffer;
      buffer += String(actBtn_a_ch[that_condtion_widget][that_number_cond]); //сообщение в условии
      buffer = "сработала тревога на датчике:" + String(descr[that_condtion_widget]) + " топик:" + String(sTopic_ch[that_condtion_widget]) + " на пине:" + String(digitalRead(pin[that_condtion_widget]));
      buffer += "\n";
      buffer += "время на устройстве:" + String(hour()) + ":" + String(minute());
      buffer += "последнее местоположение:";
      buffer += readCommonFiletoJson("ip_gps");
      buffer += "\n";
      Serial.print("Sending Email:");
      Serial.println(sendEmail(buffer));
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 1) { /////////////////////////////установить пин///////////////////////////////////////////
      //int thatpin = actBtn_a[that_condtion_widget][that_number_cond].toInt();
      int thatpin = atoi(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      int new_value = actOn_a[that_condtion_widget][that_number_cond];
      int pwm = pwmTypeAct[that_condtion_widget][that_number_cond];
      if (pwm == -1) {
        int index = -1;
        for (int i = 0; i < nWidgets; i++) {
          if (pin[i] == thatpin) {
            index = i;
          }
        }
        new_value ^= opposite;
        if (index != -1) {
          new_value ^= defaultVal[index];
        }
        digitalWrite(thatpin, new_value);
        Serial.println("digitalWrite:" + String(thatpin, DEC) + "value:" + String(new_value, DEC)) ;
      }
      else {
        if (!opposite) {
          analogWrite(thatpin, 1023 - pwm);
          Serial.println("analogWrite:" + String(thatpin, DEC) + "value:" + String(1023 - pwm)) ;
        } else {
          analogWrite(thatpin, 0);
        }
      }
      //saveSPIFFS_jsonArray(stat);
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 5) { //переключить условие/////////////////////////////////////////////////////////
      uint8_t *StatusEn = getDelimeters(String(actBtn_a_ch[that_condtion_widget][that_number_cond]), ":");
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
      check_if_there_timer_times(that_condtion_widget);
      check_if_there_timer_once(that_condtion_widget);//устанавливаем таймер,если такой есть

      if (type_a[StatusEn[0]][StatusEn[1]] == 3) {//таймер
        if (En_a[StatusEn[0]][StatusEn[1]]) {
          //En_a[StatusEn[0]][StatusEn[1]] = true;
          Serial.println("выполняем обратную установку таймера:");
          make_action(StatusEn[0], StatusEn[1], true);
          idA[StatusEn[0]][StatusEn[1]] = Alarm.timerOnce( timer_a[StatusEn[0]][StatusEn[1]], OnceOnly);
          Serial.println("заводим таймер:" + String( timer_a[StatusEn[0]][StatusEn[1]], DEC) + " sec" + " idAlarm:" + String(idA[StatusEn[0]][StatusEn[1]], DEC));
        }
      }
      //сохранить текущее состояние
      saveConditiontoJson(StatusEn[0]);
    }
    /*
      else if ((act_a[that_condtion_widget][that_number_cond] == 6) && (!opposite)) { //отключить условие
      int *StatusEn = getDelimeters(actBtn_a[that_condtion_widget][that_number_cond], ":");
      En_a[StatusEn[0]][StatusEn[1]] = false;
      //check_if_there_timer_once(that_condtion_widget);
      Serial.println("выключаем условие кнопки:" + String(StatusEn[0]) + " номер по порядку условия:" + String(StatusEn[1]));
      }
    */
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
      DynamicJsonBuffer jsonBuffer;
      String mqtt_parse = String(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      Serial.println(mqtt_parse);
      JsonObject& root = jsonBuffer.parseObject(mqtt_parse);
      if (!root.success()) {
        Serial.println("Parsing fail mqtt");
        return;
      }
      char char_arr[10];
      int value = root["msg"];
      value ^= opposite;
      sprintf(char_arr, "%d", value);

      Serial.println("Publish : (topic:" + root["Topic"].as<String>() + " msg:" + String(char_arr) + ")");
      if (client.connected()) {
        if (client.publish(root["Topic"], char_arr)) {
          Serial.println("Publish SUCCESS! (topic:" + root["Topic"].as<String>() + " msg:" + String(char_arr) + ")");
        } else {
          Serial.println("Publish FAIL! (topic:" + root["Topic"].as<String>() + " msg:" + String(char_arr) + ")");
        }
      }
      else {
        Serial.println("Publish FAIL!client dsiconnected");
      }
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 7) { ///////////////////////////8211/////////////////////////////////////////////////
      //String openPath = "ws8211/" + String(actBtn_a_ch[that_condtion_widget][that_number_cond]);
      char openPath[11] = "ws8211/";
      strcat(openPath, actBtn_a_ch[that_condtion_widget][that_number_cond]);
      String DataLoad_8211 = readCommonFiletoJson(String(openPath));
      LoadData(DataLoad_8211.c_str());
    }
    else if (act_a[that_condtion_widget][that_number_cond] == 8) { /////////////////////////WakeOnLan///////////////////////////
      //char addresWakePC[20];
      //strcpy(addresWakePC,actBtn_a_ch[that_condtion_widget][that_number_cond])
      wakeMyPC(actBtn_a_ch[that_condtion_widget][that_number_cond]);

    }
  }
}
void makeAres_sim(String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  char that_pin;
  float that_val = 0.0F;
  char control = 255;
  char that_stat = 255;
  //char that_nID;
  String String_value = "";
  root.containsKey("pin") ? that_pin = root["pin"] : that_pin = 255;
  root.containsKey("stat") ? that_stat = root["stat"] : that_stat = 255;
  //root.containsKey("read") ? that_stat = root["stat"] : that_stat = 255;
  root.containsKey("val") ? that_val = root["val"] : that_val = -1;
  //root.containsKey("nID") ? that_nID = root["nID"] : that_nID = 255;
  root.containsKey("C") ? control = root["C"] : control = 255;
  root.containsKey("st") ? String_value = root["st"].as<String>() : String_value = "";
  switch (control) {
    case 255: {
        char i = 255;
        for (char i1 = 0; i1 < nWidgets; i1++) {
          if (that_pin == pin[i1])
            i = i1;
          break;
        }

        if (that_stat != 255) {
          if  (root.containsKey("val")) {
            stat[that_stat] = that_val;
          } else {
            that_val = get_new_pin_value(that_stat);//только чтение
          }
        }
        if (i != 255) {
          if ((pinmode[i] == 2) || (pinmode[i] == 1)) {//out, in
            stat[i] = (int)that_val ^ defaultVal[i];
            //send_IR(i);
            digitalWrite(that_pin, stat[i]);
          }
          else if (pinmode[i] == 3) {//pwm
            analogWrite(that_pin, that_val);
          }
        }

        //pubStatusFULLAJAX_String(false);
        that_val = round(that_val * 200) / 200;
        server.send(200, "text / json", String(that_val, DEC));
        break;
      }
    case 1://PLUS Control
      ///aRest?Json={C:0,n:2}
      { //DynamicJsonBuffer jsonBuffer;
        //JsonObject& json = jsonBuffer.createObject();
        //JsonArray& PWM_json = json.createNestedArray("bySignalPWM");
        bySignalPWM[that_pin][that_stat] = that_val;


        //PWM_json.add(bySignalPWM[that_pin][that_nID]);
        //that_pin-это условие
        /*
          for (char i1 = 0; i1 < Condition; i1++) {
          //JsonArray& C = PWM_json.createNestedArray();
          for (char i = 0; i < NumberIDs[i1]; i++) {
            if (bySignalPWM[i1][i] != -1) {
              PWM_json.add(bySignalPWM[i1][i]);
            }
          }
          //PWM_json.add(Condition_json);
          }
        */
        //String buffer;
        //json.printTo(buffer);
        //Serial.println(buffer);
        server.send(200, "text / json",  saveConditiontoJson(that_pin));
        break;
      }
    case 2: { //IR
        send_IR(that_stat);
        break;
      }
    case 3: {
        //irsend.sendNEC(StrToHex(String_value.c_str()), 32);
        break;
      }
    case 4: {
        //        irsend.sendRaw(String_value, String_value.length(), 38);
        break;
      }
  }
}
