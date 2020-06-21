/*
   Motor controller for man or machine

   Based on simpleMotorControlWebPage from Telepresence robot

  20 June 2020 - MS - incorporated into integrated program
  11 June 2020 - MS - Created

*/


/*
   Motor Controller DIP switch (non-lithium battery, simplified serial, single controller, 9600 baud)
   1 - on
   2 - off
   3 - on
   4 - off
   5 - on
   6 - on
*/



/*
   Connector pin order

   Software serial to motor controller:
   1 = GND
   2 = 5V but through jumper so can disconnect when programming
   3 = motorControllerRX (Arduino NC)
   4 = motorControllerTX (Arduino 8) currently A2 white
*/


#include <SoftwareSerial.h>

SoftwareSerial softwareSerial(motorControllerRXPin, motorControllerTXPin); // RX, TX

const int MYFORWARD = 0;
const int MYREVERSE = 1;

int forwardSpeed = 80;

char inChar;  // character we will use for messages from the RPi


void setupMotors() {

  pinMode(LED_BUILTIN, OUTPUT);

  softwareSerial.begin(9600);
  delay(100);

  // stop the motors
  softwareSerial.write((byte)0);

}

void updateMotors() {
  // read the character we receive on the serial port from the RPi
  if (Serial.available()) {
    inChar = (char)Serial.read();
    Serial.print("Arduino: received character ]");
    Serial.print(inChar);
    Serial.print("[");
    Serial.println();

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
        Serial.println("Arduino: LED on");
        digitalWrite(LED_BUILTIN, HIGH);
        break;
      case '0':
        Serial.println("Arduino: LED off");
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case 'f':
      case 'F':
        moveForward (forwardSpeed);
        //delay(800);
        //stopBothMotors ();
        break;
      case 'b':
      case 'B':
        moveBackwards(80);
        //delay(800);
        //stopBothMotors ();
        break;
      case 'l':
      case 'L':
        turnLeft (80);
        //delay(500);
        //stopBothMotors ();
        break;
      case 'r':
      case 'R':
        turnRight (80);
        //delay(500);
        //stopBothMotors ();
        break;
      case '+':
        faster ();
        //delay(500);
        //stopBothMotors ();
        break;
      case '-':
        slower ();
        //delay(500);
        //stopBothMotors ();
        break;
      default:
        stopBothMotors ();
    }
  }
}

/*
   0 == stop
   1 = slow
*/
void moveForward(int speed) {
  Serial.println("Arduino: moveForward");
  controlMotor1(speed, MYFORWARD);
  controlMotor2(speed, MYFORWARD);
}

void moveBackwards(int speed) {
  Serial.println("Arduino: moveBackwards");
  controlMotor1(speed, MYREVERSE);
  controlMotor2(speed, MYREVERSE);
}

void turnLeft(int speed) {
  Serial.println("Arduino: turnLeft");
  controlMotor1(speed, MYFORWARD);
  controlMotor2(speed, MYREVERSE);
}

void turnRight(int speed) {
  Serial.println("Arduino: turnRight");
  controlMotor1(speed, MYREVERSE);
  controlMotor2(speed, MYFORWARD);
}



/*    0 = stop
      1 = slowest
      1-63 = speed
    63-255 = full speed
*/

void controlMotor1(int speed, bool direction) {
  /*
     From the documentation:

     Sending a character between 1 and 127 will control
     motor 1. 1 is full reverse, 64 is stop and 127 is full forward.

     Character 0 (hex 0x00) is a special case.
     Sending this character will shut down both motors.
  */
  if (speed == 0) {
    softwareSerial.write(64);
  }

  if (direction == MYFORWARD) {
    speed = map(speed, 0, 255, 65, 127);
    speed = constrain(speed, 65, 127);
  } else {
    speed = map(speed, 0, 255, 63, 1);
    speed = constrain(speed, 1, 63);
  }

  Serial.print("Arduino: M1 speed = ");
  Serial.println(speed);

  softwareSerial.write(speed);
}

void controlMotor2(int speed, bool direction) {
  /*
     From the documentation:


     Sending a character between 128 and 255 will control motor 2. 128
     is full reverse, 192 is stop and 255 is full forward.

     Character 0 (hex 0x00) is a special case.
     Sending this character will shut down both motors.
  */
  if (speed == 0) {
    softwareSerial.write(192);
  }

  if (direction == MYFORWARD) {
    speed = map(speed, 0, 255, 191, 128);
    speed = constrain(speed, 128, 191);
  } else {
    speed = map(speed, 0, 255, 193, 255);
    speed = constrain(speed, 193, 255);
  }

  Serial.print("Arduino: M2 speed = ");
  Serial.println(speed);

  softwareSerial.write(speed);
}

void stopBothMotors() {
  Serial.println("Arduino: stopBothMotors");
  softwareSerial.write((byte)0);
}

void faster() {
  Serial.print("Arduino: faster: ");
  forwardSpeed += 10;
  Serial.print("Speed now at: ");
  Serial.println(forwardSpeed);
}

void slower() {
  Serial.println("Arduino: slower: ");
  forwardSpeed -= 10;
  Serial.print("Speed now at: ");
  Serial.println(forwardSpeed);
}
