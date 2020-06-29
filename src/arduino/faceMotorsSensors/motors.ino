/*
   Motor controller for man or machine

   Based on simpleMotorControlWebPage from Telepresence robot

  20 June 2020 - MS - incorporated into integrated program
  11 June 2020 - MS - Created
  29 June 2020 - MS - add a timeout for each command

  TODO
  - handle millis overflowing (long = ? seconds)
  - all those hard coded numbers in motor control
    should be consts

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

/* // move this to main
   Connector pin order

   Software serial to motor controller:
   1 = GND
   2 = 5V but through jumper so can disconnect when programming
   3 = motorControllerRX (Arduino NC)
   4 = motorControllerTX (Arduino 8) currently A2 white
*/

const long motorTimeOut = 90; // stop after this much time
long motor1OnAt = -1; // -1 means not running
long motor2OnAt = -1; //


void setupMotors() {
  // make these parameters to the constructor:
  // led pin

  pinMode(LED_BUILTIN, OUTPUT);

  // stop the motors
  motorControllerSerialPort.write((byte)0);

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
      Serial.println("updateMotors: LED on");
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case '0':
      Serial.println("updateMotors: LED off");
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


/*
   0 == stop
   1 = slow
*/
void moveForward(int speed) {
  if (debugPrint & verboseMotor) Serial.println("moveForward");
  controlMotor1(speed, MYFORWARD);
  controlMotor2(speed, MYFORWARD);
}

void moveBackwards(int speed) {
  if (debugPrint & verboseMotor) Serial.println("moveBackwards");
  controlMotor1(speed, MYREVERSE);
  controlMotor2(speed, MYREVERSE);
}

void turnLeft(int speed) {
  if (debugPrint & verboseMotor) Serial.println("turnLeft");
  controlMotor1(speed, MYFORWARD);
  controlMotor2(speed, MYREVERSE);
}

void turnRight(int speed) {
  if (debugPrint & verboseMotor) Serial.println("turnRight");
  controlMotor1(speed, MYREVERSE);
  controlMotor2(speed, MYFORWARD);
}

void faster() {
  if (debugPrint & verboseMotor) Serial.print("faster: ");
  forwardSpeed += 10;
  if (debugPrint & verboseMotor) Serial.print("Speed now at: ");
  if (debugPrint & verboseMotor) Serial.println(forwardSpeed);
}

void slower() {
  if (debugPrint & verboseMotor) Serial.println("slower: ");
  forwardSpeed -= 10;
  if (debugPrint & verboseMotor) Serial.print("Speed now at: ");
  if (debugPrint & verboseMotor) Serial.println(forwardSpeed);
}


// If ontime is -1 it's not running
void motorControllerTick() {
  if ((motor1OnAt != -1) && ((millis() - motor1OnAt ) > motorTimeOut )) {
    stopMotor1();
    if (debugPrint & verboseMotorTimeout) Serial.println("motorControllerTick(): motor 1 timeout");
  }
  if ((motor2OnAt != -1) && ((millis() - motor2OnAt ) > motorTimeOut )) {
    stopMotor2();
    if (debugPrint & verboseMotorTimeout) Serial.println("motorControllerTick(): motor 2 timeout");
  }
}


/*
   Only functions below this line
  write to the motor controller serial port
*/

/*    0 = stop
      1 = slowest
      1-63 = speed
    63-255 = full speed
*/

void controlMotor1(int speed, bool direction) {
  if (debugPrint & verboseMotorTimeout) Serial.print("Motor 1, direction = ");
  if (debugPrint & verboseMotorTimeout) Serial.println(direction);

  /*
     From the documentation:

     Sending a character between 1 and 127 will control
     motor 1. 1 is full reverse, 64 is stop and 127 is full forward.

     Character 0 (hex 0x00) is a special case.
     Sending this character will shut down both motors.
  */
  if (speed == 0) {
    stopMotor1();
  }

  if (direction == MYREVERSE) {
    speed = map(speed, 0, 255, 65, 127);
    speed = constrain(speed, 65, 127);
  } else {
    speed = map(speed, 0, 255, 63, 1);
    speed = constrain(speed, 1, 63);
  }

  if (debugPrint & verboseMotor) Serial.print("M1 speed = ");
  if (debugPrint & verboseMotor) Serial.println(speed);

  motorControllerSerialPort.write(speed);
  motor1OnAt = millis();
}

void controlMotor2(int speed, bool direction) {
  if (debugPrint & verboseMotorTimeout) Serial.print("Motor 2, direction = ");
  if (debugPrint & verboseMotorTimeout) Serial.println(direction);
  /*
     From the documentation:


     Sending a character between 128 and 255 will control motor 2. 128
     is full reverse, 192 is stop and 255 is full forward.

     Character 0 (hex 0x00) is a special case.
     Sending this character will shut down both motors.
  */
  if (speed == 0) {
    stopMotor2();
  }

  if (direction == MYREVERSE) {
    speed = map(speed, 0, 255, 191, 128);
    speed = constrain(speed, 128, 191);
  } else {
    speed = map(speed, 0, 255, 193, 255);
    speed = constrain(speed, 193, 255);
  }

  if (debugPrint & verboseMotor) Serial.print("M2 speed = ");
  if (debugPrint & verboseMotor) Serial.println(speed);

  motorControllerSerialPort.write(speed);
  motor2OnAt = millis();

}

void stopBothMotors() {
  if (debugPrint & verboseMotor) Serial.println("stopBothMotors");
  motorControllerSerialPort.write((byte)0);
  motor1OnAt = -1;
  motor2OnAt = -1;
}

void stopMotor1() {
  motorControllerSerialPort.write(64);
  motor1OnAt = -1;
}

void stopMotor2() {
  motorControllerSerialPort.write(192);
  motor2OnAt = -1;
}
