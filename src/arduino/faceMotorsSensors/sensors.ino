/*

	Maxbotix distance measuring sensor

	based on the simple web controlled robot from my telepresence project

  Modification log

	Feb 15 2020 - Michael Shiloh - written by Michael Shiloh
  20 June 2020 - MS - incorporated into integrated program


*/

#include <SoftwareSerial.h>


SoftwareSerial distanceSensorSerialPort(distanceSensorRxPin, distanceSensorTxPin, true); // RX, TX, inverted logic

void sensorsInit() {

  distanceSensorSerialPort.begin(9600);
  delay(250); // needs 250mSec before it's ready to receive anything
  delay(100); // first reading will take an additional ~100mS.
}

boolean stringComplete = false;

/* example from Maxbotix

  //
  //  int range = EZread();
  //  if (stringComplete)
  //  {
  //    stringComplete = false;                                //reset sringComplete ready for next reading
  //
  //    Serial.print("Range ");
  //    Serial.println(range);
  //    //delay(500);                                          //delay for debugging
  //  }
*/

/*
  from https://forum.arduino.cc/index.php?topic=114808.0

  seems like a good way to do it but the reading is always 59
*/

/*
  int EZread()
  {
  int result;
  char inData[4];                                          //char array to read data into
  int index = 0;


  distanceSensorSerialPort.flush();                                     // Clear cache ready for next reading

  while (stringComplete == false) {
    Serial.print("reading ");    //debug line

    if (distanceSensorSerialPort.available())
    {
      char rByte = distanceSensorSerialPort.read();                     //read serial input for "R" to mark start of data
      if (rByte == 'R')
      {
        Serial.println("rByte set");
        while (index < 3)                                  //read next three character for range from sensor
        {
          if (distanceSensorSerialPort.available())
          {
            inData[index] = distanceSensorSerialPort.read();
            Serial.println(inData[index]);               //Debug line

            index++;                                       // Increment where to write next
          }
        }
        inData[index] = 0x00;                              //add a padding byte at end for atoi() function
      }

      rByte = 0;                                           //reset the rByte ready for next reading

      index = 0;                                           // Reset index ready for next reading
      stringComplete = true;                               // Set completion of read to true
      result = atoi(inData);                               // Changes string data into an integer for use
    }
  }
  delay (49); // Subsequent readings will take 49mS.

  return result;
  }
*/

int doReadingMyWay() {
  char inData[4];
  char inChar = 0; // anything but R
  // read the character we receive on the serial port from the RPi
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it:
  distanceSensorSerialPort.listen();

  // Wait for the leading R
  while (inChar != 'R') {
    if (debugPrint & verboseDistance) Serial.println("waiting for R");
    if (distanceSensorSerialPort.available()) {
      //      Serial.print("received ");
      //      Serial.println( inChar );
      inChar = (char)distanceSensorSerialPort.read();
    }
  }


  // Now read three characters
  while (!distanceSensorSerialPort.available()) {
    if (debugPrint & verboseDistance) Serial.println("waiting for first digit");
  }
  inChar = (char)distanceSensorSerialPort.read();
  if (debugPrint & reportDistance) Serial.print(inChar);
  inData[0] = inChar;

  while (!distanceSensorSerialPort.available()) {
    if (debugPrint & verboseDistance) Serial.println("waiting for second digit");
  }
  inChar = (char)distanceSensorSerialPort.read();
  if (debugPrint & reportDistance)  Serial.print(inChar);
  inData[1] = inChar;

  while (!distanceSensorSerialPort.available()) {
    if (debugPrint & reportDistance) Serial.println("waiting for third digit");
  }
  inChar = (char)distanceSensorSerialPort.read();
  if (debugPrint & reportDistance)  Serial.print(inChar);
  inData[2] = inChar;

  if (debugPrint & reportDistance)  Serial.print(" ascii ");


  inData[03] = 0;
  int result = atoi(inData);
  if (debugPrint & reportDistance)  Serial.print(" or ");
  if (debugPrint & reportDistance) Serial.print(result);
  if (debugPrint & reportDistance)  Serial.println(" integer");


  delay (49); // Subsequent readings will take 49mS.
  return result;
}
