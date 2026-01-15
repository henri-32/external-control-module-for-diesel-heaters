#line 1 "/home/henri-32/Softwareprojekte/Arduinoprojekte/Heizungssteuerung_git/modularmain/lcddisplay.cpp"
#include "lcddisplay.h"
#include <string.h>
#include "variables.h"
#include "hardwarefunctions.h"

DISPLAYSTATUS displaystatus = DISPLAYSTATUS::passive;

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

  for (uint8_t i = 0; i < 4; i++) {
    if (strcmp(content[i], lastLine[i]) != 0) {  // Nur schreiben, wenn sich etwas geändert hat
      clearLine(i);
      lcd.print(content[i]);
      strncpy(lastLine[i], content[i], 21);
      lastLine[i][20] = '\0';
    }
  }
}

//Inhaltsfunktion
void display_create_content() {
  switch (displaystatus) {
    case DISPLAYSTATUS::standard:
      {
        lcd.backlight();
        lcd.display();
        int t_int = int(tempC);                    // Hier ist eine manuelle Zerlegung der Variablen nötig
        int t_frac = abs((int)(tempC * 10) % 10);  // da Arduino die fragmentierung von snprintf() nicht akzeptiert
        int s_int = int(Solltemperatur);
        int s_frac = abs((int)(Solltemperatur * 10) % 10);

        snprintf(content[0], 21, "Temp.:     %d.%d C", t_int, t_frac);
        snprintf(content[1], 21, "Solltemp.: %d.%d C", s_int, s_frac);
        snprintf(content[2], 21, "Zustand:   %s", zustandsstrings[0]);
        snprintf(content[3], 21, "Mode:      %s", zustandsstrings[1]);
        break;
      }

    case DISPLAYSTATUS::passive:
      {
        lcd.noBacklight();
        lcd.noDisplay();
        break;
      }
  }
}

void ZustaendetoString() {
  switch (Heizungszustand) {
    case HEIZUNGSZUSTAND::AN:
      strncpy(zustandsstrings[0], "ON", 21);
      zustandsstrings[0][20] = '\0';
      break;
    case HEIZUNGSZUSTAND::AUS:
      strncpy(zustandsstrings[0], "OFF", 21);
      zustandsstrings[0][20] = '\0';
      break;
  }
  switch (Heizungsmode) {
    case HEIZUNGSMODE::TEMP:
      strncpy(zustandsstrings[1], "TEMP", 21);
      zustandsstrings[0][20] = '\0';
      break;
    case HEIZUNGSMODE::POWER:
      strncpy(zustandsstrings[1], "POWER", 21);
      zustandsstrings[0][20] = '\0';
      break;
  }
}
//Update_Wrapper Funktion
void display_update_wrapper(unsigned long now) {
  ZustaendetoString();
  display_create_content();
  display_update(now, content);
}
