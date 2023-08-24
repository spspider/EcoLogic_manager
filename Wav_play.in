
char buffer[1024];

void setup_wav() {
  Serial.begin(115200);
  pinMode(10, OUTPUT);
  //SPIFFS.begin();
  analogWriteRange(256);
  analogWriteFreq(32000);
}

void loop_wav() {
  Serial.println(F("Starting loop..."));
  File   f = SPIFFS.open("/ImpTwBlu.wav", "r");
  if (!f) {
    Serial.println("file open failed");
  } //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=8194#sthash.u5P6kDr6.ycI23aTr.dpuf
  analogWriteRange(256);
  analogWriteFreq(32000);
  while (f.position() < (f.size() - 1)) {
    int numBytes = _min(1024, f.size() - f.position() - 1);
    f.readBytes(buffer, numBytes);
    for (int i = 0; i < numBytes; i++) {
      int old = micros();
      analogWrite(5, buffer[i]);
      while (micros() - old < 120); //125usec = 1sec/8000 and assume 5us for overhead like wifi
    }
    Serial.print("Position "); Serial.println(f.position());
  }
  f.close();
}
