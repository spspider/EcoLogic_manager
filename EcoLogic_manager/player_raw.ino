#ifdef USE_PLAY_AUDIO_RAW
#define AUDIO_PIN 3 // TX pin (can be changed to D1 or other GPIO)
void setup_player() {
  pinMode(AUDIO_PIN, OUTPUT);
  analogWriteRange(255);
  analogWriteFreq(8000);  // 8 kHz playback rate
}

void playAudio(const char *filename) {
  File audio = LittleFS.open(filename, "r");
  if (!audio) {
    Serial.println("Failed to open audio file");
    return;
  }

  while (audio.available()) {
    uint8_t sample = audio.read();
    analogWrite(AUDIO_PIN, sample);
    delayMicroseconds(125); // ~8kHz = 125us
  }

  analogWrite(AUDIO_PIN, 0); // silence
  audio.close();
}
#endif