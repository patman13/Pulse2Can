# Pulse2Can

## Übersicht
Dieses Projekt verwendet einen Arduino Nano 33 IoT, um Pulse von einem Geber zu zählen, die Drehzahl in Hertz zu berechnen und die Ergebnisse über CAN zu senden.  
Die Einstellungen können über Bluetooth (BLE) verändert und im EEPROM gespeichert werden.

## Hardware
- Arduino Nano 33 IoT
- MCP2515 CAN-Modul
- Geber
- Bluetooth-fähiges Gerät (z.B. Smartphone oder Tablet)

## Schaltplan
Verbinde den Geber mit dem Pin 2 des Arduino Nano 33 IoT.  
Verbinde das MCP2515 CAN-Modul mit dem Arduino Nano 33 IoT:  
CS: Pin 10  
SI: Pin 11  
SO: Pin 12  
SCK: Pin 13  
VCC: 3.3V  
GND: GND  

## Bibliotheken
ArduinoBLE: Für die Bluetooth-Kommunikation  
EEPROM: Zum Speichern von Einstellungen im EEPROM  
mcp_can: Für die CAN-Kommunikation  
SPI: Für die Kommunikation mit dem MCP2515 CAN-Modul  




![image](https://github.com/user-attachments/assets/60dc3366-5a1d-4c1a-997b-ad4f601f1701)
![image](https://github.com/user-attachments/assets/ea37aa70-00be-40f5-b0d0-e5960fdf4fff)

![image](https://github.com/user-attachments/assets/ca9e3302-192e-4845-a533-e03d92017f2f)

