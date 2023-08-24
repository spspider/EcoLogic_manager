int low_compass_fuel;
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
  //  compass_read = (that_stat * 1.0F / analogDivider * 1.0F ) + analogSubtracter; //adc pin:A0;
  return compass_read ;
}
float set_analogDivider_value() {
  //high_compass_fuel = compass.readHeading();
  analogDivider = (compass.readHeading() - low_compass_fuel) / 40.0F; //adc pin:A0;
  return analogDivider;
}
float set_analogSubtracter_value() {
  low_compass_fuel = compass.readHeading();
  analogSubtracter = compass.readHeading() / analogDivider * (-1.0F) + 0.0F;
  return analogSubtracter;
}
