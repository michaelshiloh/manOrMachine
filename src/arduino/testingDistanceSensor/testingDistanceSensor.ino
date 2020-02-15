/*

	Testing the Maxbotix distance measuring sensor, also includes code
	for the motor as that will be the next step

	based on the simple web controlled robot from my telepresence project

  Modification log

	Feb 15 2020 - Michael Shiloh - written by Michael Shiloh

*/

/* Pin usage */

const int distanceSensorRxPin = 10;
const int distanceSensorTxPin = 7; // actually not used
const int motorControllerRxPin = 8; // actually not used
const int motorControllerTxPin = 12;

#include <SoftwareSerial.h>

/* old
  SoftwareSerial motorControllerPort(10, 11); // RX, TX
*/
// new
SoftwareSerial motorControllerPort(8, 12); // RX, TX
SoftwareSerial distanceSensorPort(10, 7, true); // RX, TX, inverted logic

const int MYFORWARD = 0;
const int MYREVERSE = 1;

int forwardSpeed = 80;

void setup() {

  Serial.begin(9600);
  motorControllerPort.begin(9600);
  delay(100); // Why? Maybe to let motors stop?

  distanceSensorPort.begin(9600);
  delay(250); // needs 250mSec before it's ready to receive anything
  delay(100); // first reading will take an additional ~100mS.


  // stop the motors
  motorControllerPort.write((byte)0);

}
boolean stringComplete = false;

void loop() {
  int distance = doReadingMyWay();

  if (distance > 36){a
    moveForward(200);
    delay(1000);
  } else {
    turnLeft(100);
    delay(1000);
  }
  
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
}

/*
  from https://forum.arduino.cc/index.php?topic=114808.0

  seems like a good way to do it but the reading is always 59
*/

int EZread()
{
  int result;
  char inData[4];                                          //char array to read data into
  int index = 0;


  distanceSensorPort.flush();                                     // Clear cache ready for next reading

  while (stringComplete == false) {
    Serial.print("reading ");    //debug line

    if (distanceSensorPort.available())
    {
      char rByte = distanceSensorPort.read();                     //read serial input for "R" to mark start of data
      if (rByte == 'R')
      {
        Serial.println("rByte set");
        while (index < 3)                                  //read next three character for range from sensor
        {
          if (distanceSensorPort.available())
          {
            inData[index] = distanceSensorPort.read();
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


int doReadingMyWay() {
  char inData[4];
  char inChar = 0; // anything but R
  // read the character we receive on the serial port from the RPi
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it:
  distanceSensorPort.listen();

  // Wait for the leading R
  while (inChar != 'R') {
    if (distanceSensorPort.available()) {
      //      Serial.print("received ");
      //      Serial.println( inChar );
      inChar = (char)distanceSensorPort.read();
    }
  }


  // Now read three characters
  while (!distanceSensorPort.available()) {
  }
  inChar = (char)distanceSensorPort.read();
  Serial.print(inChar);
  inData[0] = inChar;

  while (!distanceSensorPort.available()) {
  }
  inChar = (char)distanceSensorPort.read();
  Serial.print(inChar);
  inData[1] = inChar;

  while (!distanceSensorPort.available()) {
  }
  inChar = (char)distanceSensorPort.read();
  Serial.print(inChar);
  inData[2] = inChar;

  Serial.print(" ascii ");


  inData[03] = 0;
  int result = atoi(inData);
  Serial.print(" or ");
  Serial.print(result);
  Serial.println(" integer");


  delay (49); // Subsequent readings will take 49mS.
  return result;
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
    motorControllerPort.write(64);
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

  motorControllerPort.write(speed);
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
    motorControllerPort.write(192);
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

  motorControllerPort.write(speed);
}

void stopBothMotors() {
  Serial.println("Arduino: stopBothMotors");
  motorControllerPort.write((byte)0);
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
