# Bienenstock Sensordaten
Gewicht, Temperatur und Luftfeuchtigkeit in einen Bienenstock werden gemessen. Dazu wird ein Arduino Controller und das TTN LoRaWAN-Netzwerk verwendet.\
[English text](./README.md).

![Arduino compile test](https://github.com/joergkeller/beehive-sensor/workflows/Arduino%20compile%20test/badge.svg)

Um Bienenzüchter bei der Überwachung ihrer Bienenvölker zu unterstützen und die Bienen möglichst wenig zu stören, werden mit Sensoren die wichtigsten Werte in kurzen Intervallen erfasst und übermittelt.
Ziele des Designs:
* Energieeffizienz. Die Geräte sollen mit einer kleinen Solarzelle und einem Bufferakku unbeschränkt laufen (auch im Winter und bei schlechtem Wetter).
* Strahlungsarm. Ich bin kein Experte was den Einfluss von Strahlung auf Bienen betrifft. Mit einer energie- und strahlungsarmen Technologie wie LoRa statt WLAN oder GSM wird der Einfluss auf jeden Fall minimiert.
* Günstig. Kleine Microcontroller und öffentliches LoRaWAN sind sehr günstig und für den Zweck und die Datenmenge ausreichend.

## Systemübersicht
- **Arduino** (entweder Uno + Dragino LoRa Shield, Dragino LoRa Mini Dev oder CubeCell LoRa Dev) \
  Ein kleiner Microcontroller hat genug Leistung für die gelegentlichen Messungen. 
  Eine wiederaufladbare Batterie dient als Buffer während der Nacht und bei schlechtem Wetter. \
  Das Gerät läuft ohne manuelle Interaktion ausser einem Knopf, um es in in den 'Manuellen Modus' zu setzen während am Bienenstock gearbeitet wird.
    - Mehrere (z.B. 5) DS18B20 Temperatursensoren mit dem 1-wire-Protokoll
    - DHT11/22 Temperatur- und Feuchtigkeitssensor
    - Wägezelle(n) mit HX711 Konverter
    - LoRaWan Verbindung (TTN)
    - Solarzelle und wiederaufladbarer Batterie (LiPo)
    - Push-Button mit LED für temporäre Ausschaltung (Manueller Modus)
    - Evtl. weitere Geräte mit RS-485 Schnittstelle?
    
- **LoRaWAN** TTN-Gateway/Netzwerk/Applikation \
  Bienenstöcke haben normalerweise keinen Kabel-Internetanschluss. Hochgeschwindigkeitsverbindungen mit GSM und WLAN sind nicht nötig und werden wegen der Strahlenbelastung vermieden. \
    [TheThingsNetwork](https://www.thethingsnetwork.org/) (TTN) ist ein globales, offenes LoRa Netzwerk. Wenn kein Gateway innerhalb einiger km verfügbar ist, kann ein zusätzlicher eigener Gateway einfach installiert werden.
    - Übermittelt empfangene Nachrichten vom LoRa Gateway ins Internet, prüft die Authorisierung (OTAA/ABP) und kodiert/dekodiert Nachrichten 
    - Erlaubt direkte ThingSpeak-Integration (jedoch nur 1 Gerät pro Applikation mit 1 Kanal)
    - HTTP-Integration erlaubt jedes beliebige Internet-Backend anzusprechen (z.B. eigene Verarbeitung, siehe unten)
    - siehe [TTN application](./docs/ttn-application.md) (english)
    
- Übersicht und Visualisierung mit **ThingSpeak-Kanal** \
  Das ist der schnelle und einfache Weg um Sensordaten anzuzeigen.
    - Einfaches Aufsetzen und Teilen von [ThingSpeak](https://thingspeak.com/)
    - Mobile App ist verfügbar
    - Matlab Analyse ist möglich
    - Beschränkt auf 8 Felder/Kanal
    - Keine Verlinkung von mehreren Kanälen (z.B. Bienenstöcken desselben Imkers)
    - Beschränkte Interaktion

- **Verarbeitung und Weiterleitung** von Sensordaten z.B. AWS (in Bearbeitung) \
  Alternative oder Ergänzung zu ThingSpeak.
  Eine eigene Backend-Anwendung kann verwendet werden um Sensordaten zu sammeln und eine eigene Webanwendung zu betreiben.
    - siehe [AWS serverless](./docs/aws-serverless.md) (english)
    - Weiterleitung von ThingSpeak-Kanälen wie oben, aber hier können mehrere Geräte/Kanäle pro TTN-Applikation verwendet werden.
    - Zusätzliche Speicherung von Sensordaten in DynamoDB, siehe [AWS recording](./docs/aws-recorder.md)
    - Keine Einschränkungen der Anzahl Felder oder wie Sensordaten visualisiert werden (eigene Web-Applikation)
       
## Geräte-Hardware
Drei unterschiedliche Boards wurden getestet:
- Arduino Uno + [Dragino LoRa Shield](https://www.dragino.com/products/lora/item/102-lora-shield.html)
    - Benötigt 5V Betriebsspannung
    - Viele digitale Pins sind vom Shield bereits besetzt
    - Klassische Uno-Grösse und MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Dragino LoRa Mini Dev](https://www.dragino.com/products/lora/item/126-lora-mini-dev.html)
    - 3.3V Betriebsspannung ermöglicht LiPo (nicht getestet) 
    - Kleines Board, LoRa Transmitter bereits integriert
    - Klassischer Arduino MCU (16MHz ATMega328P, 32KB FLASH, 2KB SRAM)
- [Heltec CubeCell LoRa Dev](https://heltec.org/project/htcc-ab01/). Das ist mein Favorit!
    - 3.3V ermöglicht LiPo, Ladeschaltung für Solarzelle bereits integriert
    - Energieeffizienz (1W Solarzelle mit 230mAh LiPo wird an einem Sonnentag aufgeladen und kann fast 2 Wochen überbrücken)
    - Kleines Board, LoRa Transmitter bereits integriert
    - Stärkere MCU (48 MHz ARM M0+, 128KB FLASH, 16KB SRAM)
    
Verwendete Pins:

| Pin Funktion | Verdrahtung | Uno+Shield / Dragino | CubeCell |
| ------------ | ------ | ----------------- | -------- |
| 1-wire Temp Sensor | pull-up Widerstand 4k7 auf VCC | D5 | GPIO5 |
| DHT-11/22 | pull-up Widerstand 4k7 auf VCC | D4 | GPIO4 |
| HX711 Dout | | A0 | GPIO2 |
| HX711 Sck | | A1 | GPIO3 |
| Push-Button | auf GND (active low, triggert Interrupt) | D3 | GPIO7 (auch Batterie Test Control) |
| LED | auf VCC (active low) | A2 | GPIO1 |
       
## Übermittelte LoRa Nachricht (Binär kodiert)
- Das Gerät misst ca. alle 5 min
- Messungen werden bei grösseren Änderungen oder alle 30 min übermittelt (es kann jedoch passieren, dass Nachrichten verloren gehen)
- Aktuell keine Uplink-Nachrichten
- Fixe Nachrichtengrösse und Reihenfolge der Sensorwerte für kompakte/effiziente Übermittlung
- Werte werden als short integer mit 2 Nachkommastellen übermittelt (-327.67 .. 327.67)
- Spezieller Wert für null (-327.68) 
~~~
 "sensor": {
   "version": 0,  // command id or version
   "battery": 3.92,
   "weight": 0.37,
   "humidity": {
     "roof": 47.5
   },
   "temperature": {
     "drop": 20.68,
     "lower": 19.5,
     "middle": 19.93,
     "outer": 20.62,
     "roof": 22.2,
     "upper": 19.37
   }
 }
~~~   
   
## Checkout von diesem Projekt
Lokale Installation von `git` und `Arduino IDE` vorausgesetzt.\
Installation des CubeCell Software: https://heltec-automation-docs.readthedocs.io/en/latest/cubecell/quick_start.html
(aktuelle Version 1.1.0)
~~~
git clone https://github.com/joergkeller/beehive-sensor.git
cd beehive-sensor
git submodule init
git submodule update
~~~
Spätere Aktualisierung auf die neueste Version:
~~~
cd beehive-sensor
git stash
git pull
git submodule update
git stash pop
~~~

| Arduino Settings      | Value |
| ----------------------|-------|
| File > Preferences > Settings > Board Manager URLs | `http://resource.heltec.cn/download/package_CubeCell_index.json`
| File > Preferences > Settings > Sketchbook location | `C:\...\beehive-sensor` |
| File > Open... | `C:\...\beehive-sensor\arduino_beehive_sensor_lora\arduino_beehive_sensor_lora.ino` |
| Tools > Board | `CubeCell-Board` |
| Tools > LORAWAN_REGION | `REGION_EU868` |
| Tools > LORAWAN_CLASS | `CLASS_A` |
| Tools > LORAWAN_NETMODE | `OTAA` |
| Tools > LORAWAN_ADR | `ON` |
| Tools > LORAWAN_UPLINKMODE | `UNCONFIRMED` |
| Tools > LORAWAN_NET_RESERVATION | `OFF` |
| Tools > LORAWAN_AT_SUPPORT | `OFF` |
| Tools > LORAWAN_RGB | `ACTIVE` or `DEACTIVE` |
| Monitor baud rate | `115200` |

Im Projekt sind alle verwendeten Bibliotheken bereits enthalten (das ist der Grund für das submodule-Kommando und die Einstellung des Sketchbook location).

Dann
1. Erstelle gratis einen TTN account/application/device und übertrage die OTAA Authorisierungscodes in `credentials.h`
1. Erstelle gratis einen ThingSpeak account/channel und eine TTN ThingSpeak-Integration mit den entsprechenden Werten 
1. Sketch kompilieren/laden
1. Aktivierung mit OTAA benötigt einige Zeit (je nach Verbindungsqualität auch ca. eine halbe Stunde)
1. Drücke den `USR` Knopf für den 'Manuellen Modus':
    - es werden keine LoRa Nachrichten verschickt, die LED blinkt
    - kontinuierliche Gewichtsmessung für Kalibration, daraus kann Offset/Scale berechnet werden (z.B. [Excel sheet](./docs/Gewicht%20Eichung%20Loadcell.xlsx))
    - für Temperaturkompensation sollten die Gewichte zusätzlich bei unterschiedlichen Temperaturen gemessen werden
    - 1-wire Temperatursensoren nacheinander einlesen, IDs aufschreiben 
    - Knopf nochmals drücken um die LoRa Aktivierung wieder zu starten
1. Eingabe der 1-wire Sensor IDs und Gewichtskalibration in `calibration.h`
1. Sketch nochmals kompilieren/laden
    - Abwarten der OTAA-Aktivierung
    - Erster Messwert sollte im lokalen Monitor, als TTN-Daten und in der ThingSpeak-Anwendung erscheinen