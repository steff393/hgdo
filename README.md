# hgdo
**H**örmann **G**arage **D**oor **O**pener für Supramatic E3 und P3

Die Hörmann-Antriebe der Supramatic-3-Serie lassen sich über die Universaladapterplatine UAP1 steuern. Diese bietet aber kein WLAN-Interface, sondern nur Relais-Ausgänge und Eingänge.  
hdgo nutzt die gleiche BUS-Schnittstelle wie die UAP1, läuft aber auf einem ESP8266 und bietet daher WLAN.  
  
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
Die verwendete Hardware ist noch nicht final. Pin-Belegungen können sich daher künftig ändern.  
Fragen dazu bitte per Mail an wbec393@gmail.com    

## Credits
Das Projekt wurde stark inspiriert von den folgenden Projekten:  
https://github.com/stephan192/hoermann_door/  
https://github.com/raintonr/hormann-hcp/  
https://blog.bouni.de/posts/2018/hoerrmann-uap1/  
  
Vielen Dank!  

## Hinweise
Das Projekt ermöglicht eine Ansteuerung des Torantriebes aus der Ferne: ***Nutzung auf eigene Gefahr!***  
Empfehlung: Das Passwort des WLAN Access Points (cfgApPass) sollte nach Erstinbetriebnahme verändert werden.  
  
Sobald die UAP1 bzw. hgdo einmalig auf die Anfrage des Torantriebes geantwortet hat, erwartet dieser dauerhaft eine Kommunikation. Nach Entfernen von UAP1 oder hgdo muss daher der Torantrieb auf Werkseinstellungen zurückgesetzt werden, andernfalls ist keine manuelle Bedienung mehr möglich.  
