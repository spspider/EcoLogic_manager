#ifdef USE_PLAY_AUDIO
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2SNoDAC.h>

extern ESP8266WebServer server;

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceLittleFS *file = nullptr;
AudioOutputI2SNoDAC *out = nullptr;

void setup_player() {

}

void playAudioMP3(const char *filename) {
  if (mp3 && mp3->isRunning()) {
    mp3->stop();
    delete mp3;
    delete file;
    delete out;
  }

  file = new AudioFileSourceLittleFS(filename);
  out = new AudioOutputI2SNoDAC();
  out->begin();

  mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);
}


void loop_player() {
  server.handleClient();

  if (mp3 && mp3->isRunning()) {
    mp3->loop();
  }
}
#endif
