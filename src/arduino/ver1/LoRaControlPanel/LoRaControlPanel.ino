#include <SPI.h>
#include <LoRa.h>

// based on LoRaTransmit

const int forwardButtonPin = 3;
const int backwardButtonPin = 4;
const int leftButtonPin = 2;
const int rightButtonPin = 5;
const int dataButtonPin = 6;

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
  pinMode(dataButtonPin, INPUT);
}

void loop() {

  checkButtons();


}

int checkLoRa() {

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
    return 1;
  } else {
    return 0;
  }
}

long timeLastSent = 0;
const long interval = 0; // send as often as possible

void checkButtons() {

  // don't send too often
  if (millis() - timeLastSent < interval) {
    Serial.println("too soon");
    return;
  }

  if ( digitalRead(forwardButtonPin)) {
    Serial.println(" sending f");
    LoRa.beginPacket();
    LoRa.print('f');
    LoRa.endPacket();
    timeLastSent = millis();
  }

  if ( digitalRead(backwardButtonPin)) {
    Serial.println(" sending b");
    LoRa.beginPacket();
    LoRa.print('b');
    LoRa.endPacket();
    timeLastSent = millis();
  }

  if ( digitalRead(leftButtonPin)) {
    Serial.println(" sending l");
    LoRa.beginPacket();
    LoRa.print('l');
    LoRa.endPacket();
    timeLastSent = millis();
  }

  if ( digitalRead(rightButtonPin)) {
    Serial.println(" sending r");
    LoRa.beginPacket();
    LoRa.print('r');
    LoRa.endPacket();
    timeLastSent = millis();
  }

  if ( digitalRead(dataButtonPin)) {
    Serial.println(" sending d");
    LoRa.beginPacket();
    LoRa.print('d');
    LoRa.endPacket();
    timeLastSent = millis();

    checkLoRa();
      
  }

}
