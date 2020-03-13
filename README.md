# OpenWaterData

Prototyp für Wassermessstationen.

## Was ist hier drin?

- **nodemcu** Code für den NodeMCU, alles was mit Internetverbindung zu tun hat
- **arduino** Code für den Arduino Uno/ Nano der alle Messdaten aufnimmt und an den nodeMCU weiterleitet
- **sinatraserver** ein kleiner Ruby/ Sinatra server um POST requests von nodeMCU zu debuggen
- **rocketship** Rustserver der als Endpunkt für Messdaten und Enduser-API fungiert
- **sketches** verschiedene Sketches um die Wassersensoen zu testen

Alles work-in-progress, mehr Dokumentation kommt.
