#if defined(ws2811_include)

#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//#define CLK_PIN   4
//#define DATA_PIN 15
#define DATA_PIN 15 //lock2,any 15
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    88//40//53 - двери: 52 ванна 88, компьютер 8
//#if defined(ws2811_include)
CRGB leds[NUM_LEDS];

//CRGB leds_prep[NUM_LEDS];
#define NUM_CHARTS    1//2
#include <ArduinoJson.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_D1_PIN_ORDER


#define BRIGHTNESS          100
#define FRAMES_PER_SECOND  500

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

unsigned char from_ws8211[NUM_CHARTS];
unsigned char to_ws8211[NUM_CHARTS];
unsigned char type_ws8211[NUM_CHARTS];
unsigned char dir_ws8211[NUM_CHARTS];
unsigned char sp_ws8211;
unsigned char duration = 255;
unsigned char fade = 20;
unsigned char bright = 255;
unsigned char fadetype = 3;
bool running_led = true;

unsigned char fade_sunrise[NUM_LEDS];

unsigned  char col_ws8211[NUM_CHARTS];
unsigned  char white_ws8211[NUM_CHARTS];
unsigned  char br__ws8211[NUM_CHARTS];
unsigned char delay_loop_8211 = 0;
//unsigned  char white_white_col_[NUM_CHARTS];

char num_ws8211 = 0;


short int pos[NUM_CHARTS];
long millis_strart;
//String Previous_set = "";
//uint8_t Previous_set_char;
bool reverse_set;
boolean inv;

void loadLimits() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["NUM_LEDS"] = NUM_LEDS;
  json["NUM_CHARTS"] = NUM_CHARTS;
  json["DATA_PIN"] = DATA_PIN;
  String buffer;
  json.printTo(buffer);
  server.send(200, "text/json", buffer);
}
void setup_ws2811() {

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //String dataLoad = "{\"from\":[0,20],\"to\":[20,40],\"type\":[4,4],\"dir\":[0,0],\"col\":[0,1],\"num\":2}";
  //String dataLoad = "{\"from\":[0,20],\"to\":[20,40],\"type\":[5,5],\"dir\":[0,10],\"col\":[0,1],\"num\":2}";
  //String dataLoad = "{\"from\":[0,21],\"to\":[21,40],\"type\":[1,6],\"dir\":[0,1],\"col\":[0,1],\"num\":1,\"sp\":10,\"dr\":255}";
  //String dataLoad = "{\"from\":[0,10],\"to\":[10,20],\"type\":[1,1],\"dir\":[0,1],\"col\":[0,0],\"num\":2,\"sp\":50,\"fd\":200}";
  //String dataLoad = "{\"from\":[0],\"to\":[40],\"type\":[13],\"dir\":[0],\"col\":[0],\"num\":1,\"sp\":10,\"fd\":50,\"br\":5,\"r\":1}";
  //LoadData(dataLoad);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  String DataLoad_8211 = readCommonFiletoJson("ws8211/0");
  char buffer[200];
  DataLoad_8211.toCharArray(buffer, sizeof buffer);
  LoadData(buffer);

}
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool LoadData_set_leds(char json[400]) {
  Serial.println(json);
  //running_led = true;
  DynamicJsonBuffer jsonBuffer;
  //char json[] =
  //ws2811AJAXset?json={%22g1%22:[192,192],%22g3%22:[192,192],%22num%22:2,%22br%22:255,%22wh%22:0}
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed LoadData:");
    Serial.println(json);
    return false;
  }
  unsigned char num_ = root["num"];
  unsigned char wh = root["wh"];
  unsigned char g1 = 255;
  for (unsigned char i = 0; i < NUM_LEDS; i++) {
    // unsigned char g1 = root["g1"][i];
    //unsigned char g2 = root["g2"][i];//от

    unsigned char g3 = root["g3"][i];//от
    if (i > num_) {
      leds[i] = CHSV( 0, 0, 0);//hue,white,bright
    } else {
      leds[i] = CHSV( g1, wh, g3);//hue,white,bright
    }

  }

  FastLED.show();
}

bool LoadData(char json[200]) {
  Serial.println(json);
  running_led = true;
  DynamicJsonBuffer jsonBuffer;
  //char json[] =
  //"{\"from\":[0,10],\"to\":[10,20],\"type\":[4,3],\"dir\":[1,0],\"col\":[0,1],\"num\":2}";
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed LoadData:");
    Serial.println(json);
    return false;
  }


  //if (root["num"] <= NUM_CHARTS) {
  unsigned char num_ = root["num"];
  num_ws8211 = num_ <= NUM_CHARTS ? num_ : NUM_CHARTS;
  //num_ = root["num"] > NUM_CHARTS ? NUM_CHARTS : = root["num"];
  if ( root["num"] > NUM_CHARTS) {
    Serial.print("ERROR NUM_CHARTS:");
    Serial.println(num_, DEC);
  }

  sp_ws8211 = root.containsKey("sp") ? root["sp"] : 10;
  //sp_ws8211 = ws8211_loop == false ? 0 : sp_ws8211 ;
  duration = root.containsKey("dr") ? root["dr"] : 255;
  fade = root.containsKey("fd") ? root["fd"] : 200;
  bright = root.containsKey("br") ? root["br"] : 255;
  reverse_set = root.containsKey("r") ? root["r"] : 0;
  fadetype = root.containsKey("fdt") ? root["fdt"] : 3;
  inv = root.containsKey("inv") ? root["inv"] : 0;
  //reverse_set = root["r"];
  if (!reverse_set) {
    //Previous_set = String(json);
    //Previous_set_char=
    //Serial.println("set return to this");
  }


  FastLED.setBrightness(bright);
  //}
  for (char i = 0; i < num_ws8211; i++) {
    unsigned char from_ = root["from"][i];//от
    unsigned char to_ = root["to"][i];//к чему
    unsigned char type_ = root["type"][i];//тип
    unsigned char dir_ = root["dir"][i];//направление
    unsigned char col_ = root["col"][i];//цвет
    unsigned char white_col_ = root["wh"][i];//интенсивность белого
    unsigned char br_ = root["br_"][i];//яркость

    white_ws8211[i] = white_col_ == 0 ? 255 : white_col_;
    br__ws8211[i] = br_ == 0 ? 192 : br_;
    //Serial.print("white_col:");
    //Serial.println(white_ws8211[i], DEC);
    pos[i] = dir_ == 1 ? to_ : from_;
    //if (from_ <= NUM_LEDS) {
    from_ws8211[i] = from_ <= NUM_LEDS ? from_ : NUM_LEDS ;
    if ( from_ > NUM_LEDS) {
      Serial.print("ERROR from_:");
      Serial.println(from_, DEC);
    }
    //}
    //if (to_ws8211[i] <= NUM_LEDS) {
    to_ws8211[i] = to_ <= NUM_LEDS ? to_ : NUM_LEDS;
    if ( to_ > NUM_LEDS) {
      Serial.print("ERROR to_:");
      Serial.println(to_, DEC);
    }
    //}
    type_ws8211[i] = type_;
    dir_ws8211[i] = dir_;
    col_ws8211[i] = col_;
    //white_white_col_[i]=white_col_;
    /*
        Serial.print("from_ws8211:");
        Serial.println(from_, DEC);
        Serial.print("to_:");
        Serial.println(to_, DEC);
        Serial.print("type_:");
        Serial.println(type_, DEC);
        Serial.print("dir_:");
        Serial.println(dir_, DEC);
        Serial.print("col_ws8211:");
        Serial.println(col_ws8211[i], DEC);
    */
  }
  if (!ws8211_loop) {
    loop_ws2811();
    //delay_loop_8211 = 2;
    //ws8211_loop = true;
  }
  return true;
}
uint8_t one_min, one_hour;//, new_value_count;


void one_sec() {
  if (timer_alarm_action_switch) {
    if (timer_alarm_action < timer_alarm_action_max) {
      timer_alarm_action++;
    } else {
      timer_alarm_action_switch = 0;
      timer_alarm_action = 0;
    }
  }

  if ((duration > 0) && (duration != 255)) {
    running_led = true;
    //    alarm_is_active[idWidget][i] = true;
    //Serial.print("Duration:");
    //Serial.println(duration, DEC);
    duration--;
  } else if (duration == 0)
  {
    running_led = false;
    if (reverse_set) {
      String DataLoad_8211 = readCommonFiletoJson("ws8211/0");
      char buffer[200];
      DataLoad_8211.toCharArray(buffer, sizeof buffer);
      LoadData(buffer);
      //Serial.println("Load reverse set:");
      //if (Previous_set.indexOf("\"r\":1") == -1) {
      //LoadData(Previous_set.c_str());
      //reverse_set = 0;
      // } else {
      //  reverse_set = 0;
      // }
    }
    //duration=255;
  }
  //  one_sec++;
  one_min++;

  /*
    new_value_count++;

    if (new_value_count > 1) {
    //new_value_count = 0;
    get_new_pin_value_ = true;
    }
    if (new_value_count > 9) {
    new_value_count = 0;
    get_new_pin_value_ = false;
    }
  */
  if (one_min >= 60) {

    one_min = 0;
    one_hour++;
    //check_for_license();
  }
  if (one_hour >= 24) {

  }
  if (delay_loop_8211 > 1) {
    delay_loop_8211--;
  }
}
void check_for_license() {
  if (one_hour > 1) {
    // license = getEEPROM_char(0);
    if (license != 1) {
      FileDelete("pin_setup.txt");
    }
  }
}
void load_pattern()
{

  // a colored dot sweeping back and forth, with fading trails

  switch (fadetype) {
    case 0:
      nscale8_video( leds, NUM_LEDS, fade);
      break;
    case 1:
      fade_video( leds, NUM_LEDS, fade);
      break;
    case 2:
      fadeLightBy( leds, NUM_LEDS, fade);
      break;
    case 3:
      fadeToBlackBy( leds, NUM_LEDS, fade);
      break;
    case 4:
      fade_raw( leds, NUM_LEDS, fade);
      break;
    case 5:
      //nscale8_raw( leds, NUM_LEDS, fade);
      break;
    case 6:
      nscale8( leds, NUM_LEDS, fade);
      break;
    case 7:
      //fadeUsingColor( leds, NUM_LEDS, fade);
      break;
    case 8:
      blur1d( leds, NUM_LEDS, fade);
      break;

  }

  //fade_video(leds, NUM_LEDS, fade);//чем меньше, тем медленее)

  if (!running_led) return;
  //Serial.println(divide1,DEC);

  for (char i = 0; i < num_ws8211; i++) {

    if ( col_ws8211[i] != 0) {
      gHue = col_ws8211[i];
    }

    char mid = (to_ws8211[i] - from_ws8211[i]) / 2;
    char maxLight = 192 / mid;
    char i_on = 0, i_off = 0;
    char i_div = 1;
    char divide1 = (to_ws8211[i] - from_ws8211[i]) / 3;

    switch (type_ws8211[i]) {
      case 0://с одной стороны
        leds[pos[i]] += CHSV( gHue, white_ws8211[i], br__ws8211[i]);//
        //fill_solid(leds, NUM_LEDS, CHSV(hue, white_ws8211[i], 192));
        //leds[pos[i]].maximizeBrightness(1);
        pos[i] = reverseDirection(i);
        //leds[i].setRGB(0,255,250);  // Set Color HERE!!!
        //leds[i].fadeLightBy(brightness);
        //
        break;
      case 1://с двух сторон
        // case 5:
        pos[i] = reverseDirection(i);
        leds[pos[i]] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//бегающий огонь

        unsigned char  calc_pos;
        calc_pos = (to_ws8211[i] - pos[i] + from_ws8211[i]);
        leds[calc_pos] = CHSV( gHue, white_ws8211[i], br__ws8211[i]); //бегающий огонь
        break;
      case 2://полностью в цвете
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          leds[i1] = CHSV(  gHue, white_ws8211[i], br__ws8211[i]);//простая линия света
        }
        break;
      case 3://середина в цвете
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (i1 < mid) {
            leds[i1] = CHSV(  gHue, white_ws8211[i], maxLight * i1);
          }
          else {
            leds[i1] = CHSV(  gHue, white_ws8211[i], maxLight * (to_ws8211[i] - i1));
          }
        }
        break;
      case 4://не работает
        leds[random(from_ws8211[i], to_ws8211[i])] += CHSV( gHue, white_ws8211[i], br__ws8211[i]); //бегающий огонь
        break;
      case 5://мигание через одну
        if (pos[i] >= to_ws8211[i]) {
          dir_ws8211[i] = 1;
        } else if (pos[i] <= from_ws8211[i]) {
          dir_ws8211[i] = 0;
        }
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (pos[i] % 2 == 0) {
            if ( (i1 % 2) == 0) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//бегающий огонь
            }
          } else {
            if ( (i1 % 2) != 0) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);//бегающий огонь
            }
          }
        }
        break;
      case 6://деление на три части
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {
          if (i1 < (divide1 * i_div) + from_ws8211[i]) {
            if ((i_div & 0x01) == (pos[i] & 0x01)) {
              leds[i1] = CHSV( gHue, white_ws8211[i], br__ws8211[i]);
            }
          } else {
            i_div++;
          }
        }
        break;
      case 7:
        rainbow();
        break;
      case 8:
        rainbowWithGlitter();
        break;
      case 9:
        confetti();
        break;
      case 10:
        sinelon();
        break;
      case 11:
        bpm();
        break;
      case 12:
        juggle();
        break;
      case 13://glow_to_max
        for (char i1 = from_ws8211[i]; i1 < to_ws8211[i]; i1++) {

          unsigned char bright_pos = pos[i] * (255 / (to_ws8211[i] - from_ws8211[i]));
          leds[i1] = CHSV( 0, 0, bright_pos);//бегающий огонь
        }
        break;
      case 14://случайно
        type_ws8211[i] = random(0, 13);
        break;
      case 15://цветомузыка

        break;
    }
  }


  //Serial.println(pos[i]);

  //leds[num_leds_fade - pos] += CHSV( gHue, white_ws8211[i], br__ws8211[i]);

    if (inv) {
    for ( unsigned char i = 0; i < NUM_LEDS; i++) { //9948
      leds[i] = CHSV(0,0,255)-leds[i];    }
  }
  
}
unsigned char reverseDirection(unsigned char i) {

  switch (dir_ws8211[i]) {
    case 0:
      pos[i] = pos[i] >= to_ws8211[i] ? from_ws8211[i] : pos[i];
      break;
    case 1:
      pos[i] = pos[i] <= from_ws8211[i] ?  to_ws8211[i] : pos[i];
      break;
  }
  return  pos[i];
}
void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  //addGlitter(80);
}
/*
  void addGlitter(fract8 chanceOfGlitter)
  {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
  }
*/
void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}



void loop_ws2811()
{
  // Call the current pattern function once, updating the 'leds' array

  // send the 'leds' array out to the actual LED strip
  //gPatterns[gCurrentPatternNumber]();
  load_pattern();

  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(sp_ws8211);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    //nextPattern();  // change patterns periodically
  }

  //unsigned char one_period_speed = 2;//((sp_ws8211 * 16) / (192 / 2));
  if (millis() > (sp_ws8211)+ millis_strart) {
    for (char i = 0; i < num_ws8211; i++) {
      if (pos[0] == 0) {
        //check_for_changes();
      }
      switch (dir_ws8211[i]) {
        case 0:
          pos[i] = pos[i] < to_ws8211[i] ?  pos[i] + 1 : to_ws8211[i];
          break;
        case 1:
          pos[i] =  pos[i] > from_ws8211[i] ? pos[i] - 1 : from_ws8211[i];
          break;
        case 3:
          pos[i] = random16( from_ws8211[i], to_ws8211[i]);
          break;
        case 4:
          pos[i] = 0;
          break;
      }

    }
    millis_strart = GET_MILLIS();
  }


  if (delay_loop_8211 == 1) {
    ws8211_loop = false;
  }
}
#endif
