#ifdef USE_PLAY_AUDIO
#include <AudioFileSourceLittleFS.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2SNoDAC.h>
#include <ESP8266WebServer.h>

extern ESP8266WebServer server;

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceLittleFS *file = nullptr;
AudioOutputI2SNoDAC *out = nullptr;

void setup_player() {

}

void handlePlay() {
  play_wav_i2s_nodac();
  server.send(200, "text/plain", "Playing audio.wav via I2SNoDAC");
}

void play_wav_i2s_nodac() {
  if (wav && wav->isRunning()) {
    return; // Уже играет
  }
  if (file) delete file;
  if (wav) delete wav;
  if (out) delete out;

  file = new AudioFileSourceLittleFS("/audio.wav");
  out = new AudioOutputI2SNoDAC(); // программный дельта-сигма DAC
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
}

void loop_player() {
  if (wav && wav->isRunning()) {
    wav->loop();
  }
}
#endif
