/*
   Simple example using the  library which
	 communicates with the Sabertooth2x12 motor controller from Dimensional
	 Engineering using a software serial port.

   Based on simpleMotorControlWebPage from Telepresence robot

	 Created by Michael Shiloh

	15 July 2020 - MS - created
*/

/* Pin usage */
// Rx pin doesn't need to be connected
const int motorControllerRXPin = 4;
const int motorControllerTXPin = 8;

// Other constants
const int motorTimeOut = 90;


#include <SabertoothMotorController.h>

SabertoothMotorController myMotorController(
  motorControllerRXPin,
  motorControllerTXPin,
  motorTimeOut);

void setup() {
  Serial.begin(9600);
  myMotorController.init();
}

void loop() {

  if (Serial.available()) {
    char inChar = (char)Serial.read();
    updateMotors(inChar);
  }

  myMotorController.tick();
}

void updateMotors(char inChar) {
  /*
    1 = pin 13 LED on
    0 = pin 13 LED  off
    F = move Forward
    B = move Backwards
    L = turn Left
    R = turn Right
    S = Stop
    + = set forward motor speed faster
    - = set forward motor speed slower
  */

  switch (inChar) {

    case '1':
      Serial.println("LED on");
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case '0':
      Serial.println("LED off");
      digitalWrite(LED_BUILTIN, LOW);
      break;
    case 'f':
    case 'F':
      myMotorController.forward(80);
      break;
    case 'b':
    case 'B':
      myMotorController.backward(80);
      break;
    case 'l':
    case 'L':
      myMotorController.left(80);
      break;
    case 'r':
    case 'R':
      myMotorController.right(80);
      break;
   
    default:
      myMotorController.forward(0);
      Serial.println("invalid message");
  }
}
