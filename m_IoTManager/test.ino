char incomingByte = 0;   // for incoming serial data
void test_setup() {
}
void test_loop() {
  if (Serial.available() > 0)
  {
    String readStringMy = Serial.readString();
  }
}

