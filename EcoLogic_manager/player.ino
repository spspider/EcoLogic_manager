#ifdef USE_PLAY_AUDIO_WAV
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2SNoDAC.h>


extern ESP8266WebServer server;

AudioGeneratorWAV *wav;
AudioFileSourceLittleFS *file;
AudioOutputI2SNoDAC *out;

void setup_player() {

}

void playAudioWAV(const char *filename) {
   // Check if file exists
  if (!LittleFS.exists(filename)) {
    Serial.printf("ERROR: %s not found in LittleFS!\n", filename);
    return;
  }
    File test = LittleFS.open(filename, "r");
  if (!test) {
    Serial.println("File open failed!");
    return;
  } else {
    Serial.printf("File opened: %s, size: %u bytes\n", filename, test.size());
    test.close();
  }

  // Set up audio playback
  file = new AudioFileSourceLittleFS(filename);
  out = new AudioOutputI2SNoDAC();
  out->SetGain(0.8);  // Adjust volume (0.0–1.0)

  wav = new AudioGeneratorWAV();
  if (!wav->begin(file, out)) {
    Serial.println("Failed to start WAV playback");
  } else {
    Serial.println("WAV playback started");
  }
}


void loop_player() {

  if (wav && wav->isRunning()) {
    wav->loop();
  } else if (wav) {
    Serial.println("Playback finished");
    wav->stop();
    delete wav;
    delete file;
    delete out;
    wav = nullptr;
  }
}
#endif
