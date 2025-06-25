#ifdef USE_PLAY_AUDIO_MP3
#include <Arduino.h>
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

  mp3 = new AudioGeneratorMP3();
  if (mp3->begin(file, out)) {
    Serial.println("MP3 playback started");
  } else {
    Serial.println("Failed to start MP3 playback");
  }
}


void loop_player() {

  if (mp3 && mp3->isRunning()) {
    mp3->loop();
  } else if (mp3) {
    Serial.println("Playback finished");
    mp3->stop();
    delete mp3;
    delete file;
    delete out;
    mp3 = nullptr;
  }
}
#endif
