#include "lcddisplay.h"
#include <string.h>

enum class DISPLAYSTATUS { standard,
                           passive };
DISPLAYSTATUS displaystatus = DISPLAYSTATUS::standard;

char content[4][21];
char zustandsstrings[2][21];


// LCD Adresse und Größe anpassen
static LiquidCrystal_I2C lcd(0x27, 20, 4);


void display_init() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

// Hilfsfunktion: Zeile löschen
static void clearLine(uint8_t line) {
  lcd.setCursor(0, line);
  lcd.print("                    ");  // 20 Leerzeichen
  lcd.setCursor(0, line);
}


// Update-Funktion
void display_update(unsigned long now,
                    char content[4][21]) {

  static char lastLine[4][21] = { "", "", "", "" };

  static char lines[4][21];
  for (uint8_t i = 0; i < 4; i++) {
    strncpy(lines[i], content[i], 21);
  }
  for (uint8_t i = 0; i < 4; i++) {
    if (strcmp(lines[i], lastLine[i]) != 0) {  // Nur schreiben, wenn sich etwas geändert hat
      clearLine(i);
      lcd.print(lines[i]);
      strncpy(lastLine[i], lines[i], 21);
      lastLine[i][20] = '\0';
    }
  }
}

//Inhaltsfunktion
void display_content() {
  switch (displaystatus) {
    case DISPLAYSTATUS::standard:
      snprintf(content[0], 21, "Temperatur %.1f", TempC);
      snprintf(content[1], 21, "Solltemperatur %.1f", Solltemperatur);
      snprintf(content[2], 21, "%s", zustandsstring[0]);
      snprintf(content[3], 21, "%s", zustandsstrings[1]);
      break;
  }
}

const char ZustaendetoString() {
  switch (Heizungszustand) {
    case HEIZUNGSZUSTAND::AN:
      zustandsstrings[0] = "AN";
      break;
    case HEIZUNGSZUSTAND::AUS:
      zustandsstrings[0] = "AUS"
  }
  switch (Heizungsmode) {
    case HEIZUNGSMODE::TEMP:
      zustandsstrings[1] = "TEMP";
      break;
    case HEIZUNGSMODE::POWER:
      zustandsstrings[1] = "POWER";
      break;
  }
}
//Update_Wrapper Funktion
void display_update_wrapper(unsigned long now) {

  display_update(now, content);
}
