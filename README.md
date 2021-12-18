# hgdo
**H**örmann **G**arage **D**oor **O**pener für Supramatic E3 und P3

Die Hörmann-Antriebe der Supramatic-3-Serie lassen sich über die Universaladapterplatine UAP1 steuern. Diese bietet aber kein WLAN-Interface, sondern nur Relais-Ausgänge und Eingänge.  
hdgo nutzt die gleiche BUS-Schnittstelle wie die UAP1, läuft aber auf einem ESP8266 und bietet daher WLAN.  
  
## Funktionen
- Web-Interface (JSON)
- Websocket-Interface
- Bedienung per Taster (abhängig von Uhrzeit)
- Automatisches Schließen (abhängig von Uhrzeit) mit Vorwarnung
- Anbindung von entweder 4x4-Tastenfeld oder RFID-Leser
- Paketdienst-Funktion (Fahrt auf Lüftungsposition per Code, 15s später automatisches Schließen) --> nur zulässig mit Lichtschranke! (noch in Erprobung)
- Aufzeichnung der letzten Fahrten (mit Code von Tastenfeld bzw. RFID)
- Trace der Buskommunikation (nur für Debugging)
- Anzeige der Öffnungsposition, z.B. "30% offen" (geplant)

Details zu den Funktionen werde ich nach und nach auch im [Wiki](https://github.com/steff393/hgdo/wiki) beschreiben.
  
## Beispiele
<img src="https://i.ibb.co/7WxjjMV/Web-Interface.png">  

```c++
http://x.x.x.x/json
  
{
  "hgdo": {
    "version": "v0.0.1",
    "bldDate": "2021-10-02",
    "timeNow": "15:22:11",
    "millis": 2705270
  },
  "door": {
    "open": true,
    "closed": false,
    "error": false,
    "opening": false,
    "closing": false,
    "venting": false
  },
  "wifi": {
    "mac": "8C:AA:B5:7A:7F:55",
    "rssi": -66,
    "signal": 68,
    "channel": 7
  }
}

http://x.x.x.x/json?act=0     --> Stop
http://x.x.x.x/json?act=1     --> Open
http://x.x.x.x/json?act=2     --> Close
http://x.x.x.x/json?act=3     --> Venting Position
http://x.x.x.x/json?act=4     --> Toggle Light

http://x.x.x.x/edit           --> LittleFS Editor
http://x.x.x.x/update         --> Software Update Over-The-Air
http://x.x.x.x/reset          --> Reset
```
  
## Hardware
<img src="https://i.ibb.co/xCXz35Q/PCB-Schema.png">  
  
Über den RJ12-Stecker erfolgt die Spannungsversorgung und Buskommunikation. Die Anschlüsse für RFID, Tastenfeld und externen Taster sind unterhalb der NodeMCU platziert. Der externe Taster wird per Schraubklemmen angeschlossen. RFID und Tastenfeld über 8- bzw. 4-polige Steckerleisten.

## Kontakt
Bei Fragen oder wenn ihr Unterstützung braucht gerne einfach eine Mail schicken (wbec393@gmail.com)     
  
## Credits
Das Projekt wurde stark inspiriert von den folgenden Projekten:  
https://github.com/stephan192/hoermann_door/  
https://github.com/raintonr/hormann-hcp/  
https://blog.bouni.de/posts/2018/hoerrmann-uap1/  
  
Folgende Libraries wurden genutzt:
- [ESP Async WebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- [MFRC522](https://github.com/miguelbalboa/MFRC522)
- [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [Web Interface](https://RandomNerdTutorials.com)
- [A Beginner's Guide to the ESP8266 - article](https://github.com/tttapa/ESP8266)
- [Keypad library for Arduino](https://github.com/chris--a/Keypad)
- [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)

Vielen Dank!  

## Hinweise
Das Projekt ermöglicht eine Ansteuerung des Torantriebes aus der Ferne: ***Nutzung auf eigene Gefahr!***  
Empfehlung: Das Passwort des WLAN Access Points (cfgApPass) sollte nach Erstinbetriebnahme verändert werden.  
  
Sobald die UAP1 bzw. hgdo einmalig auf die Anfrage des Torantriebes geantwortet hat, erwartet dieser dauerhaft eine Kommunikation. Nach Entfernen von UAP1 oder hgdo muss daher der Torantrieb auf Werkseinstellungen zurückgesetzt werden (s. [Wiki](https://github.com/steff393/hgdo/wiki/Informationen-zum-Torantrieb#r%C3%BCcksetzen-des-torantriebs)), andernfalls ist keine manuelle Bedienung mehr möglich.  

## Unterstützung des Projektes
hgdo gefällt dir? [Star this project on GitHub](https://github.com/steff393/wbec/stargazers)!  
