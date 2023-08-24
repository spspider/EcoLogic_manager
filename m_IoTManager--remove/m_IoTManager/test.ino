char incomingByte = 0;   // for incoming serial data
void test_setup() {
}
void test_loop() {
  if (Serial.available() > 0) {
    //incomingByte = Serial.read();
    //String myString = String(char(incomingByte));
    String readStringMy = Serial.readString();
    //if (readStringMy)
      //CheckInternet(readStringMy);
      //CheckInternet(readStringMy);
      // Serial.println(char(incomingByte));

    }


}

