#include <SPI.h>
#include <LoRa.h>

// based on LoRaTransmit

const int forwardButtonPin = 3;
const int backwardButtonPin = 4;
const int leftButtonPin = 2;
const int rightButtonPin = 5;

void setup() {
  Serial.begin(9600);
  // while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  pinMode(forwardButtonPin, INPUT);
  pinMode(backwardButtonPin, INPUT);
  pinMode(leftButtonPin, INPUT);
  pinMode(rightButtonPin, INPUT);
}

void loop() {

  checkButtons();

  checkLoRa();
}

void checkLoRa() {

  // Anything from radio?
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");
    while (LoRa.available()) {
      char inChar = (char)LoRa.read();
      Serial.print(inChar);
    }
    Serial.println();
  }
}

void checkButtons() {

  if ( digitalRead(forwardButtonPin)) {
    Serial.println(" sending f");
    LoRa.beginPacket();
    LoRa.print('f');
    LoRa.endPacket();
  }

  if ( digitalRead(backwardButtonPin)) {
    Serial.println(" sending b");
    LoRa.beginPacket();
    LoRa.print('b');
    LoRa.endPacket();
  }

  if ( digitalRead(leftButtonPin)) {
    Serial.println(" sending l");
    LoRa.beginPacket();
    LoRa.print('l');
    LoRa.endPacket();
  }

  if ( digitalRead(rightButtonPin)) {
    Serial.println(" sending r");
    LoRa.beginPacket();
    LoRa.print('r');
    LoRa.endPacket();
  }

}
