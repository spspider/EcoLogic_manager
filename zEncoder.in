////////////////////zEncoder
//#define readA bitRead(PIND,4)//faster than digitalRead()
//#define readB bitRead(PIND,5)//faster than digitalRead()


///////////////////////////


ICACHE_RAM_ATTR void doEncoderA() {
  boolean EncoderA = 0;
  boolean EncoderB = 0;
  if (EncoderIB == 255 && EncoderIA == 255)return;
  EncoderA = digitalRead(EncoderIA);
  EncoderB = digitalRead(EncoderIB);
  if (EncoderB != EncoderA) {
    no_internet_timer ++;
  } else {
    no_internet_timer --;
  }

}
ICACHE_RAM_ATTR void doEncoderB() {
  boolean EncoderA = 0;
  boolean EncoderB = 0;
  if (EncoderIB == 255 && EncoderIA == 255)return;
  EncoderA = digitalRead(EncoderIA);
  EncoderB = digitalRead(EncoderIB);
  if (EncoderA == EncoderB) {
    no_internet_timer ++;
  } else {
    no_internet_timer --;
  }
}
void EncoderCalc() {
  //unsigned char FirstLadder = 50;
  //unsigned char SecondLadder = 500;

  if (engineA != 255 && engineB != 255) {
    int speed_Enc_int = speed_Enc * 4 + 4;
    //int speed_Enc_int = no_internet_timer/1024;
    if (no_internet_timer > 200) {
      speed_Enc_int = (300 < no_internet_timer && no_internet_timer < 1000)  ?  no_internet_timer : speed_Enc_int ;
      speed_Enc_int = (0 < no_internet_timer && no_internet_timer < 300)  ?  300 : speed_Enc_int ;
      unsigned char topic_is;
      callback_scoket(engineB, speed_Enc_int);
      callback_scoket(engineA, 0);
    } else if (no_internet_timer < -200) {
      speed_Enc_int = (-300 > no_internet_timer && no_internet_timer > -1000) ? (no_internet_timer * -1) : speed_Enc_int ;
      speed_Enc_int = (-0 > no_internet_timer && no_internet_timer > -300) ? 300 : speed_Enc_int ;
      callback_scoket(engineA, speed_Enc_int);
      callback_scoket(engineB, 0);
    }
    else if (200 > no_internet_timer && no_internet_timer > -200) {
      callback_scoket(engineA, 0);
      callback_scoket(engineB, 0);
      engineA = engineB = 255;
    }
  }
}
