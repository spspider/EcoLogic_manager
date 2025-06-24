#ifdef USE_PLAY_AUDIO_FROM_URL
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2SNoDAC.h>

WiFiUDP udp;
const unsigned int localUdpPort = 12345; // Port to listen on
const char* audioFilePath = "/audio.wav";
File audioFile;

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceLittleFS *file = nullptr;
AudioOutputI2SNoDAC *out = nullptr;

void setup_player_from_url() {
  udp.begin(localUdpPort);
  Serial.print("[AUDIO] UDP listening on port: ");
  Serial.println(localUdpPort);
}

void loop_player_from_url() {
  static bool receiving = false;
  static bool playbackStarted = false;
  char incomingPacket[512];
  int packetSize = udp.parsePacket();

  if (packetSize) {
    int len = udp.read(incomingPacket, sizeof(incomingPacket));
    if (len > 0) {
      // Check for end of transmission marker
      if (len == 3 && incomingPacket[0] == 'E' && incomingPacket[1] == 'N' && incomingPacket[2] == 'D') {
        Serial.println("[AUDIO] End of audio stream received.");
        if (audioFile) {
          audioFile.close();
          receiving = false;
          playbackStarted = false;
        }
      } else {
        // Start new file if not already open
        if (!receiving) {
          Serial.println("[AUDIO] Starting new audio file for UDP stream...");
          audioFile = LittleFS.open(audioFilePath, "w");
          receiving = true;
        }
        if (audioFile) {
          audioFile.write((uint8_t*)incomingPacket, len);
          Serial.print("[AUDIO] Received audio data chunk, bytes: ");
          Serial.println(len);
        }
      }
    }
  }

  // Start playback after receiving is done and not already started
  if (!receiving && !playbackStarted && LittleFS.exists(audioFilePath)) {
    Serial.println("[AUDIO] Starting playback of received audio file...");
    if (file) delete file;
    if (wav) delete wav;
    if (out) delete out;
    file = new AudioFileSourceLittleFS(audioFilePath);
    out = new AudioOutputI2SNoDAC();
    wav = new AudioGeneratorWAV();
    wav->begin(file, out);
    playbackStarted = true;
  }

  // Continue playback
  if (wav && wav->isRunning()) {
    wav->loop();
  } else if (playbackStarted && wav && !wav->isRunning()) {
    Serial.println("[AUDIO] Playback finished.");
    playbackStarted = false;
  }
}
#endif