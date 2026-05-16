# Externe Steuerung für Dieselheizungen (LF Bros Fokus)

Minimal-invasive Zusatzsteuerung für Dieselheizungen: Die reguläre
Steuerungssoftware bleibt unangetastet, nur der vorhandene An/Aus-Eingang wird
über ein Relais betätigt. Damit wird temperaturabhängiges vollständiges
Ein-/Ausschalten ermöglicht.

<p align="center"> 
  <img src=images/assembledController.jpeg" width="500"
</p>

## Projekt

### Anforderungsbeschreibung
Ich nutze eine "LF Bros" Dieselheizung für den Innenraum meines Bootes.
Viele günstige Dieselheizungen können die Heizleistung regeln, aber nicht
selbstständig vollständig an- und ausschalten, wenn die Solltemperatur über-
oder unterschritten wird.

Ein hartes Abschalten über die Stromversorgung ist keine Option, weil die
Heizung zum Schutz kontrolliert nachlüften und abkühlen muss.

Das originale Steuergerät ist proprietär. Deshalb erweitert dieses Projekt die
Funktionalität extern und minimal-invasiv: Es greift nicht in die interne
Steuerungssoftware ein, sondern betätigt nur den vorhandenen An-/Aus-Eingang
über ein Relais.

Dadurch bleibt die restliche Funktionalität des originalen Steuergeräts
erhalten. Ein- und Ausschalten kann weiter manuell oder automatisch über den
Arduino-Controller erfolgen.

Um Entscheidungen über das Ein-/Ausschalten zu ermöglichen, wird der Status
der originalen Steuerung emuliert.

### Scope und Annahmen
- Primär entwickelt für eine LF Bros Dieselheizung.
- Der Ansatz ist auf ähnliche Heizungen übertragbar.
- Es wird nicht in die proprietäre interne Kommunikationslogik des
  Heizungscontrollers eingegriffen.

### Hardware
- Arduino UNO
- DS18B20 Temperatursensor
- Arduino-kompatibles Relais (AZDelivery KF-301)
- 20x4 I2C-fähiges LCD-Display (Arceli 20x4 2004 LCD Display Modul)
- 1x Encoder mit Push-Button (GIAK KY-040)
- 2x Kippschalter
- Netzteil 12V auf 5V für Arduino

### Hardware-Eingriff
TODO: Foto verlinken

### Pin-Belegungen
Aus `includes/config.h`:

- `D2`: `powerSwitch`
- `D3`: `modeSwitch`
- `D4`: `relais`
- `D5`: `tempSensor` (DS18B20 OneWire)
- `D6`: Encoder A (`myEncoder[0]`)
- `D7`: Encoder B (`myEncoder[1]`)
- `D8`: `displayButton`
- `I2C (A4/A5 beim UNO)`: LCD (`LCDAdapter{0x27, 20, 4}`)

### Build
Die `.hex`-Datei zum Flashen kann über das Make-Target gebaut werden:

make all

### Quick Start (Bedienung)
1. System einschalten und Startzustand am Display prüfen.
2. Mit `modeSwitch` zwischen `POWER` und `TEMP` umschalten.
3. Im `TEMP`-Modus mit dem Encoder die Solltemperatur einstellen.
4. Mit gedrücktem `displayButton` und Encoder die LCD-Seiten wechseln.
5. Mit `powerSwitch` die Heizung manuell ein-/ausschalten.

### Steuerlogik (Laufzeitverhalten)
Die Hauptschleife ruft pro Zyklus `controller()` auf. Intern passiert:

1. Eingänge lesen (`inputDevices.update()`)
2. Eingaben anwenden (`applyInputdata()`)
3. Heizlogik ausführen (`applyHeatingLogic()`)
4. Ausgabe-Intent schreiben (`writeOutputIntent()`)
5. Hardware aktualisieren (`outputDevices.update()`)

Steuerverhalten:

- `powerSwitch` Änderung:
  - Normal: Relais `Long`-Puls (1500 ms), Heizungszustand toggeln (`ON/OFF`)
  - Bei gedrücktem `displayButton`: nur internen Zustand toggeln, kein Relais
- `modeSwitch` Änderung:
  - Normal: Relais `Short`-Puls (200 ms), Modus toggeln (`POWER/TEMP`)
  - Bei gedrücktem `displayButton`: nur internen Modus toggeln, kein Relais
- `displayButton` bei Loslassen:
  - ohne Kombiaktion: LCD zwischen `OFF` und `Page1` umschalten
  - nach Kombiaktion: keine zusätzliche Display-Umschaltung
- Encoder drehen:
  - ohne gedrückten `displayButton`: Solltemperatur anpassen
    (`target_tempC += encoder_steps * 0.5`, begrenzt auf 5..30 °C)
  - mit gedrücktem `displayButton`: LCD-Seiten zyklisch wechseln

Temperaturregelung (`TEMP`-Modus):

- Wenn `sensor_tempC <= target_tempC - 1.5` und Heizung `OFF`:
  Relais `Long`-Puls, Zustand auf `ON`
- Wenn `sensor_tempC >= target_tempC + 1.5` und Heizung `ON`:
  Relais `Long`-Puls, Zustand auf `OFF`

### Konfigurationswerte
Aus `includes/config.h`:

| Parameter | Wert | Bedeutung |
| --- | --- | --- |
| `defaultTemp` | `15` | Start-Solltemperatur in °C |
| `tempStep` | `0.5` | Schrittweite pro Encoder-Impuls in °C |
| `tempMin` | `5` | Untere Solltemperaturgrenze in °C |
| `tempMax` | `30` | Obere Solltemperaturgrenze in °C |
| `tolerance` | `1.5` | Hysterese um die Solltemperatur in °C |
| `encoderValCutoff` | `6` | Grenzwert zur Interpretation von Encoder-Schritten |
| `RelaisLongPulse_ms` | `1500` | Langer Relais-Puls (Power-Toggle) |
| `RelaisShortPulse_ms` | `200` | Kurzer Relais-Puls (Mode-Toggle) |

### Praktische Nutzung
Hierdurch wird folgende Nutzung ermöglicht:
Controller und Heizung können über eine Stromquelle versorgt werden.
Bei Einschalten der Stromversorgung startet der Arduino-Controller
und überwacht die Raumtemperatur. Weicht diese über die konfigurierte
Toleranz hinaus von der Solltemperatur ab, hat der Controller ohne weitere
Handlungen sofort die Möglichkeit, die Heizung ein- und auszuschalten.

Mit dem Mode-Switch kann zwischen Temperaturregelung und Powerregelung
gewechselt werden. Im `POWER`-Modus ist die Temperaturregelung nicht aktiv.
Das ermöglicht unter anderem das Freibrennen der Heizung unabhängig vom
Verhältnis Soll-/Isttemperatur.

Wird der Power-Switch betätigt, schaltet sich die Heizung manuell ein bzw.
aus. Die Heizung wechselt automatisch in den `POWER`-Modus, um ein sofortiges
Eingreifen der Temperaturregelung zu verhindern.

Da der Zustand der originalen Steuerung (AN/AUS und POWER/TEMP)
nur antizipiert wird, kann es (mehrfach im Entwicklungsprozess, 
mittlerweile hoffentlich nur noch theoretisch) 
passieren, dass der Zustand von Controller und originaler Steuerung nicht 
übereinstimmt.
Deshalb kann bei gedrücktem Display-Knopf der Mode- oder Power-Switch
betätigt werden. Dann wird nur der interne Controllerzustand ohne
Relaisbetätigung geändert.


### Artefakte
Die Memory-Funktionen sollten eine statistische Auswertung des
Laufzeitverhaltens ermöglichen, um effiziente Konfigurationswerte zu ermitteln.
Aktuell beeinflussen sie die Kernfunktionalität nicht negativ, liefern aber
noch keine interpretierbaren Anzeigen.


### Zukünftige Aufgaben 
- Korrektur der Memory Funktionen zur statistischen Auswertung
- Zusätzliche Anbingung an die Drehbetätigung des originalen Encoders, 
  um vollständig über den eigenen Controller steuern zu können. 

### Sicherheitshinweis
- Heizung nicht hart über Wegnahme der Versorgung abschalten, wenn ein
  geregeltes Nachlauf-/Abkühlverhalten erforderlich ist.
- Die Verantwortung für sicheren Einbau und Betrieb liegt selbstverstänglich
  beim Anwender.


## Lizenz
Der eigene Projektcode steht unter der MIT-Lizenz. Details siehe
[`LICENSE`](LICENSE).

Dieses Repository enthält außerdem Drittanbieter-Bibliotheken mit eigenen
Lizenzen. Details und Quellen siehe [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
