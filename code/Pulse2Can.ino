#include <ArduinoBLE.h>
#include <EEPROM.h>
#include <mcp_can.h>
#include <SPI.h>

const int pulsePinA = 2; // Pin für Signal A des Inkrementalgebers
const int pulsePinB = 3; // Pin für Signal B des Inkrementalgebers
volatile int pulseCount = 0;
volatile bool direction = true; // true = vorwärts, false = rückwärts
unsigned long lastMillis = 0;
int teethCount = 80; // Standard-Zähnezahl
unsigned long sendInterval = 1000; // Standard-Sendeintervall in ms
unsigned long lastSendMillis = 0;
unsigned long canID = 0x100; // Standard-CAN-ID
unsigned long calcInterval = 1000; // Standard-Zeitintervall für Drehzahlberechnung in ms

BLEService configService("180A"); // BLE Service
BLEIntCharacteristic teethCharacteristic("2A29", BLERead | BLEWrite); // BLE Charakteristik für die Zähnezahl
BLEUnsignedLongCharacteristic intervalCharacteristic("2A2A", BLERead | BLEWrite); // BLE Charakteristik für das Sendeintervall
BLEUnsignedLongCharacteristic canIDCharacteristic("2A2B", BLERead | BLEWrite); // BLE Charakteristik für die CAN-ID
BLEUnsignedLongCharacteristic calcIntervalCharacteristic("2A2C", BLERead | BLEWrite); // BLE Charakteristik für das Zeitintervall der Drehzahlberechnung

MCP_CAN CAN0(10); // Set CS to pin 10

void setup() {
  Serial.begin(9600);
  pinMode(pulsePinA, INPUT);
  pinMode(pulsePinB, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsePinA), countPulseA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pulsePinB), countPulseB, CHANGE);

  // EEPROM initialisieren und gespeicherte Werte laden
  EEPROM.begin();
  teethCount = EEPROM.read(0);
  if (teethCount == 0xFF) { // Wenn EEPROM leer ist, Standardwert setzen
    teethCount = 80;
  }
  sendInterval = EEPROM.read(1);
  if (sendInterval == 0xFFFFFFFF) { // Wenn EEPROM leer ist, Standardwert setzen
    sendInterval = 1000;
  }
  canID = EEPROM.read(2);
  if (canID == 0xFFFFFFFF) { // Wenn EEPROM leer ist, Standardwert setzen
    canID = 0x100;
  }
  calcInterval = EEPROM.read(3);
  if (calcInterval == 0xFFFFFFFF) { // Wenn EEPROM leer ist, Standardwert setzen
    calcInterval = 1000;
  }

  // Bluetooth initialisieren
  if (!BLE.begin()) {
    Serial.println("Bluetooth initialisieren fehlgeschlagen!");
    while (1);
  }

  BLE.setLocalName("TeethCounter");
  BLE.setAdvertisedService(configService);
  configService.addCharacteristic(teethCharacteristic);
  configService.addCharacteristic(intervalCharacteristic);
  configService.addCharacteristic(canIDCharacteristic);
  configService.addCharacteristic(calcIntervalCharacteristic);
  BLE.addService(configService);

  teethCharacteristic.writeValue(teethCount);
  intervalCharacteristic.writeValue(sendInterval);
  canIDCharacteristic.writeValue(canID);
  calcIntervalCharacteristic.writeValue(calcInterval);

  BLE.advertise();
  Serial.println("Bluetooth gestartet!");

  // CAN initialisieren
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN initialisiert!");
  } else {
    Serial.println("CAN initialisieren fehlgeschlagen!");
    while (1);
  }
  CAN0.setMode(MCP_NORMAL);
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Verbunden mit: ");
    Serial.println(central.address());
  }

  if (teethCharacteristic.written()) {
    teethCount = teethCharacteristic.value();
    EEPROM.write(0, teethCount); // Neue Zähnezahl im EEPROM speichern
    EEPROM.commit();
    Serial.print("Neue Zähnezahl: ");
    Serial.println(teethCount);
  }

  if (intervalCharacteristic.written()) {
    sendInterval = intervalCharacteristic.value();
    EEPROM.write(1, sendInterval); // Neues Sendeintervall im EEPROM speichern
    EEPROM.commit();
    Serial.print("Neues Sendeintervall: ");
    Serial.println(sendInterval);
  }

  if (canIDCharacteristic.written()) {
    canID = canIDCharacteristic.value();
    EEPROM.write(2, canID); // Neue CAN-ID im EEPROM speichern
    EEPROM.commit();
    Serial.print("Neue CAN-ID: ");
    Serial.println(canID);
  }

  if (calcIntervalCharacteristic.written()) {
    calcInterval = calcIntervalCharacteristic.value();
    EEPROM.write(3, calcInterval); // Neues Zeitintervall im EEPROM speichern
    EEPROM.commit();
    Serial.print("Neues Zeitintervall für Drehzahlberechnung: ");
    Serial.println(calcInterval);
  }

  if (millis() - lastMillis >= calcInterval) { // Zeitintervall für Drehzahlberechnung erreicht
    noInterrupts(); // Interrupts deaktivieren, um die Pulse sicher zu zählen
    int pulses = pulseCount;
    pulseCount = 0; // Zähler zurücksetzen
    interrupts(); // Interrupts wieder aktivieren

    // Drehzahl in Hz berechnen
    float frequency = pulses / (float)teethCount;

    Serial.print("Drehzahl (Hz): ");
    Serial.println(frequency);

    lastMillis = millis();
  }

  if (millis() - lastSendMillis >= sendInterval) { // Sendeintervall erreicht
    float frequency = pulseCount / (float)teethCount;
    byte data[5];
    memcpy(data, &frequency, sizeof(frequency));
    data[4] = direction ? 1 : 0; // Drehrichtung hinzufügen (1 = vorwärts, 0 = rückwärts)
    CAN0.sendMsgBuf(canID, 0, 5, data); // Drehzahl und Drehrichtung über CAN senden
    lastSendMillis = millis();
  }

  if (central) {
    Serial.print("Verbindung getrennt: ");
    Serial.println(central.address());
  }
}

void countPulseA() {
  if (digitalRead(pulsePinA) == digitalRead(pulsePinB)) {
    direction = true; // vorwärts
  } else {
    direction = false; // rückwärts
  }
  pulseCount++;
}

void countPulseB() {
  if (digitalRead(pulsePinA) != digitalRead(pulsePinB)) {
    direction = true; // vorwärts
  } else {
    direction = false; // rückwärts
  }
  pulseCount++;
}
