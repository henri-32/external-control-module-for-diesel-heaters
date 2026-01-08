#include <Wire.h>

void setup() {
  Wire.begin();        // I2C starten
  Serial.begin(9600);  // Serielle Kommunikation starten
  while (!Serial);     // Warten, bis Serial bereit ist
  Serial.println("I2C Scanner gestartet");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scan läuft...");

  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C Gerät gefunden an Adresse 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      nDevices++;
    } 
    else if (error == 4) {
      Serial.print("Unbekannter Fehler an Adresse 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }    
  }

  if (nDevices == 0)
    Serial.println("Keine I2C Geräte gefunden\n");
  else
    Serial.println("Scan abgeschlossen\n");

  delay(5000);  // Alle 5 Sekunden scannen
}
