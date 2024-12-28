#if defined(as5600)
void setup_compass()
{
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
#endif
  float setFUllFuel(uint8_t full_fuel)
  {
    // high_compass_fuel = compass.readHeading();
#if defined(as5600)
    analogDivider = (encoder.getAngle() - analogSubtracter) / (full_fuel * 1.00F);
#endif
    savePinSetup();

    return analogDivider;
  }
  float setZeroFuel()
  {
#if defined(as5600)
    analogSubtracter = encoder.getAngle();
#endif
    savePinSetup();

    return analogSubtracter;
  }

  bool savePinSetup()
  {
    File buffer_read = fileSystem->open("/pin_setup.txt", "r");
    DynamicJsonDocument jsonDocument(1024); // Adjust the capacity as needed
    DeserializationError error = deserializeJson(jsonDocument, buffer_read);

    if (error)
    {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return false;
    }

    JsonObject json = jsonDocument.as<JsonObject>();
    json["aDiv"] = analogDivider;
    json["aSusbt"] = analogSubtracter;

    String buffer;
    serializeJson(json, buffer);
    // Serial.println(buffer);
    saveCommonFiletoJson("pin_setup", buffer, 1);

    return true;
  }
