#include "lcddisplay.h"

// LCD Adresse und Größe anpassen
static LiquidCrystal_I2C lcd(0x27, 20, 4);

// Zuletzt angezeigte Zeilen für Dirty-Flag
static String lastLine[4] = {"", "", "", ""};

void display_init() {
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

// Hilfsfunktion: Zeile löschen
static void clearLine(uint8_t line) {
    lcd.setCursor(0, line);
    lcd.print("                    "); // 20 Leerzeichen
    lcd.setCursor(0, line);
}

// Update-Funktion
void display_update(const String &line0, const String &line1, const String &line2, const String &line3) {
    String lines[4] = {line0, line1, line2, line3};

    for (uint8_t i = 0; i < 4; i++) {
        if (lines[i] != lastLine[i]) {  // Nur schreiben, wenn sich etwas geändert hat
            clearLine(i);
            lcd.print(lines[i]);
            lastLine[i] = lines[i];
        }
    }
}
