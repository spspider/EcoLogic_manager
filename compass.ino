int low_compass_fuel;
void setup_compass() {

}
/*
  void setup_compass() {
  ///////////////compass/////////////

  Wire.begin();
  compass.init();
  compass.setSamplingRate(50);
  ///////////////////////////////
  //pinMode(pin[i], OUTPUT);
  compass.reset();
  Serial.println("Compass Ready");

  }

  float get_fuel_value() {

  float compass_read = 0.0F;
  compass_read = compass.readHeading();
  if (compass_read == 0) {
  compass.reset();
  } else {
   //compass_read = (compass_read * 1.0F / analogDivider * 1.0F ) - analogSubtracter; //adc pin:A0;
  // compass_read = ((compass_read  - analogSubtracter) / analogDivider);

  }
  return compass_read ;
  }
*/
float setFUllFuel(uint8_t full_fuel) {
  //high_compass_fuel = compass.readHeading();

 // analogDivider = (encoder.getAngle()  - analogSubtracter) / (full_fuel * 1.00F);

  savePinSetup();

  return analogDivider;
}
float setZeroFuel() {

 // analogSubtracter = encoder.getAngle() ;
  savePinSetup();

  return analogSubtracter;
}

bool savePinSetup() {

  String buffer_read = readCommonFiletoJson("pin_setup");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buffer_read);
  json["aDiv"] = analogDivider;
  json["aSusbt"] = analogSubtracter;
  String buffer;
  json.printTo(buffer);
  //Serial.println(buffer);
  saveCommonFiletoJson("pin_setup", buffer, 1);

}

