char incomingByte = 0;   // for incoming serial data
void test_setup() {
}
void test_loop() {
  if (Serial.available() > 0) {
    //incomingByte = Serial.read();
    //String myString = String(char(incomingByte));
    String readStringMy = Serial.readString();
    if (readStringMy) {
      test_action = true;
      //check_if_there_next_times();
      
    }
    //CheckInternet(readStringMy);
    //CheckInternet(readStringMy);
    // Serial.println(char(incomingByte));

  }


}

