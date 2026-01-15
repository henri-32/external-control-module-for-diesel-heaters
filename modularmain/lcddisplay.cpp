#include "lcddisplay.h"
#include <string.h>
#include "variables.h"

// Display Status Variable wird auf passive gesetzt damit das Display im Startmodus ausgeschaltet ist
DISPLAYSTATUS displaystatus = DISPLAYSTATUS::passive;

// In dieser Datei globale Arrays für den Displayinhalt (content) definiert
char content[4][21];
char stringsOfStates[2][21];

// LCD Adresse und Größe anpassen, Objekt erstellen
static LiquidCrystal_I2C lcd(0x27, 20, 4);

// Hilfsfunktion: Zeile löschen
static void clearLine(uint8_t line) {
  lcd.setCursor(0, line);
  lcd.print("                    ");  // 20 Leerzeichen
  lcd.setCursor(0, line);
}

// Initialisierungsfunktion für main.cpp setup()  
void display_init() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
}



// Update-Funktion für display_update_wrapper(), die Displayinhalt physisch aktualisiert
void display_update(unsigned long now, char content[4][21]) {
  static char lastLine[4][21] = { "", "", "", "" }; //nötige für Vergleich

  for (uint8_t i = 0; i < 4; i++) {
    if (strcmp(content[i], lastLine[i]) != 0) {  // Nur schreiben, wenn sich etwas geändert hat
      clearLine(i);
      lcd.print(content[i]);
      strncpy(lastLine[i], content[i], 21);
      lastLine[i][20] = '\0';
    }
  }
}

// Hilfsfunktion: Zustände werden in Strings umgewandelt um sie auf dem Display anzuzeigen zu können
void statesToStrings() {
  switch (Heizungszustand) {
    case HEIZUNGSZUSTAND::AN:
      strncpy(stringsOfStates[0], "ON", 21);
      stringsOfStates[0][20] = '\0';
      break;
    case HEIZUNGSZUSTAND::AUS:
      strncpy(stringsOfStates[0], "OFF", 21);
      stringsOfStates[0][20] = '\0';
      break;
  }
  switch (Heizungsmode) {
    case HEIZUNGSMODE::TEMP:
      strncpy(stringsOfStates[1], "TEMP", 21);
      stringsOfStates[0][20] = '\0';
      break;
    case HEIZUNGSMODE::POWER:
      strncpy(stringsOfStates[1], "POWER", 21);
      stringsOfStates[0][20] = '\0';
      break;
  }
}
//Inhaltsfunktion die berechnet, was angezeigt wird (content)
void createDisplayContent() {
  statesToStrings(); // Es werden die aktuellen globalen Zustände eingelesen und in Strings umgewandelt
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
        snprintf(content[2], 21, "Zustand:   %s", stringsOfStates[0]);
        snprintf(content[3], 21, "Mode:      %s", stringsOfStates[1]);
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


//Wrapper Funktion für main.cpp loop()
void lcdDisplay(unsigned long now) {
  createDisplayContent();
  display_update(now, content);
}
