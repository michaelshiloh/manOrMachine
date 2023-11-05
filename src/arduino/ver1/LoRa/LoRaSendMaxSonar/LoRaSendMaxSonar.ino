//#include <SPI.h>
#include <LoRa.h>

#include "wiring_private.h" // pinPeripheral() function

// Use Sercom 2 to make a new serial port
// D2 Tx
// D3 Rx
Uart mySerial (&sercom2, 3, 2, SERCOM_RX_PAD_3, UART_TX_PAD_2);
void SERCOM2_Handler()
{
  mySerial.IrqHandler();
}

int counter;

void setup() {
  Serial.begin(9600);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Start the extra serial port on SERCOM 2
  mySerial.begin(9600);

  pinPeripheral(2, PIO_SERCOM_ALT);
  pinPeripheral(3, PIO_SERCOM_ALT);

  delay(1000);  // Not sure what this is for
  LoRa.beginPacket(); // open a packet
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {

  while (mySerial.available()) {
    char c = mySerial.read();
    if (c == 'R') {
      Serial.println();
      LoRa.endPacket(); // 
      digitalWrite(LED_BUILTIN, counter++ % 2);
     // delay(500);
      LoRa.beginPacket();
    }
    else {
      Serial.print(c);
      LoRa.write(c);
    }
  }
}
