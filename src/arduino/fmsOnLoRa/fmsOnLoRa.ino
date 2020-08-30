
/*
    Finally try combining all three features

    19 jun 2020 - ms - initial integration
    20 jun 2020 - ms - library for face
    21 jun 2020 - ms - maxsonic sensor
    25 jun 2020 - ms - start adding control panel
    17 jul 2020 - ms - use the new motor controller library
    12 aug 2020 - ms - transition to Arduino MKRWAN1310: hardware serial, lora radio
    26 aug 2020 - ms - first test MKR - motor controller, sensor works! Next: radio

    TODO
    - full description
    - classes for sensors?
    - remove delays in sensors
    - put pins in order
    - emergency stop
    - currently, motors continue turning until there is a command to stop.
      This is useful for testing but dangerous.


  MKRWAN1310 is based on SAMD21 which uses SERCOMs for serial ports. Leaving the
  USB serial port as it is, for debugging and uploading, plan is this:

  - Motor controller on SERCOM 2: D2-TX, D3-RX (only Tx is used)
    - circuit: black wire = ground, white wire = pin 2
  - Sensors on SERCOM 3, RX=1 TX=0 (only Rx is used)
  - Neopixel unchanged (I think; haven't tried it)
  - LoRa for control panel


*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <Face.h>
//#include <SabertoothMotorControllerSercom.h>
#include <LoRa.h>
#include "wiring_private.h" // pinPeripheral() function


/*
   Pin assignments
*/

const int distanceSensorTxPin = 0;
const int distanceSensorRxPin = 1;
const int motorControllerTxPin = 2;
const int motorControllerRxPin = 3;
const int neoPixelFacePin = 4; // might as well go in order

/*
    Other global variables
*/
const int neoPixelFaceCount = 60;
const long motorTimeOut = 100;
long motor1OnAt = -1; // -1 means not running
long motor2OnAt = -1; //


// reasonable readings of distance sensor (unverified)
const int MIN_DISTANCE = 6;
const int MAX_DISTANCE = 200;

// Control verbosity of messages
const int verboseDistance = 1 << 0;
const int reportDistance = 1 << 2;
const int verboseMotor = 1 << 3;
const int debugMotor = 1 << 4;
const int verboseMotorTimeout = 1 << 5;
const int debugSensorReadingTick = 1 << 6;
const int verboseHardwareSerial = 1 << 7;

const int debugPrint = 0 ;

// for controlling the motors
const int MYFORWARD = 0;
const int MYREVERSE = 1;
int forwardSpeed = 80;

// For keeping track of what we are doing
char lastChar;

/*
   global objects
*/
Face robotFace(neoPixelFaceCount, neoPixelFacePin);
//SabertoothMotorController myMotorController(motorTimeOut);
/*
    SERCOM functions
*/

// Motor controller on SERCOM 2: D2-TX, D3-RX (only Tx is used)
Uart motorControllerSerial (
  &sercom2,
  motorControllerRxPin, motorControllerTxPin,
  SERCOM_RX_PAD_3, UART_TX_PAD_2);
void SERCOM2_Handler()
{
  motorControllerSerial.IrqHandler();
}

// Sensors on SERCOM 3, RX=1 TX=0 (only Rx is used)
Uart sensorSerial (
  &sercom3,
  distanceSensorRxPin, distanceSensorTxPin,
  SERCOM_RX_PAD_1, UART_TX_PAD_0);
void SERCOM3_Handler()
{
  sensorSerial.IrqHandler();
}


void setup() {

  Serial.begin(9600);
 // while (!Serial);
  pinMode(LED_BUILTIN, OUTPUT);

  robotFace.init();

  // This would be in motor controller init function
  motorControllerSerial.begin(9600);
  pinPeripheral(motorControllerTxPin, PIO_SERCOM_ALT);
  pinPeripheral(motorControllerRxPin, PIO_SERCOM_ALT);
  stopBothMotors();

  sensorsInit();

  //  updateMotors();?? perhaps I meant motorInit?

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.beginPacket(); // open a packet
  Serial.println("fmsOn LoRa ready");
}
void loop() {

  motorControllerTick(); // this must be called regularly so all other functions here must not use delay()

  listenHardwareSerialPort();

  listenLoRa();

  // super simple sensor read
  // with added LoRa send
  while (sensorSerial.available()) {
    char c = sensorSerial.read();
    if (c == 'R') {
      if (debugPrint & reportDistance) Serial.println(); // end the line
      // LoRa.endPacket(); // end the packet
      // LoRa.beginPacket(); // begin the next packet
    }
    else {
      if (debugPrint & reportDistance) Serial.print(c);
      //  LoRa.write(c); // write this character
    }
  }

}

void listenLoRa() {

  // Anything from radio?
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet ");
    while (LoRa.available()) {
      char inChar = (char)LoRa.read();
      Serial.print(inChar);
      updateMotors(inChar);
    }
    Serial.println();
  }
}

void listenHardwareSerialPort() {
  // read the character we receive on the serial port from the RPi
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (debugPrint & verboseHardwareSerial) Serial.print("listenHardwareSerialPort: received character ]");
    if (debugPrint & verboseHardwareSerial) Serial.print(inChar);
    if (debugPrint & verboseHardwareSerial) Serial.print("[");
    if (debugPrint & verboseHardwareSerial) Serial.println();
    updateMotors(inChar);
  }
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
      moveForward(80);
      break;
    case 'b':
    case 'B':
      moveBackwards(80);
      break;
    case 'l':
    case 'L':
      turnLeft(80);
      break;
    case 'r':
    case 'R':
      turnRight(80);
      break;

    // Is this smart? Might be a good emergency stop
    case '\r':
    case '\n':
      break;

    default:
      // moveForward(0);
      Serial.println("invalid message");
  }
}



//// motor controller stuff, until i figure out how to make this a library


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
    if (debugPrint & verboseMotorTimeout) {
      Serial.print("motorControllerTick(): motor 2 timeout at ");
      Serial.print(millis());
      Serial.println();
    }
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
  if ((debugPrint & verboseMotor) || (debugPrint & verboseMotorTimeout)) Serial.print("Motor 1 ");
  if (debugPrint & verboseMotor) Serial.print("direction = ");

  if (debugPrint & verboseMotor) Serial.print(direction);

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

  if ((debugPrint & verboseMotor) || (debugPrint & verboseMotorTimeout)) Serial.print("M1 speed = ");
  if ((debugPrint & verboseMotor) || (debugPrint & verboseMotorTimeout)) Serial.print(speed);

  motorControllerSerial.write(speed);
  motor1OnAt = millis();

  if (debugPrint & verboseMotorTimeout) Serial.print(" at time = ");
  if (debugPrint & verboseMotorTimeout) Serial.print(motor1OnAt);
  if (debugPrint ) Serial.println();
}

void controlMotor2(int speed, bool direction) {
  if ((debugPrint & verboseMotor) || (debugPrint & verboseMotorTimeout)) Serial.print("Motor 2 ");
  if (debugPrint & verboseMotor) Serial.print("direction = ");
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

  if (debugPrint & verboseMotorTimeout) Serial.print("M2 speed = ");
  if (debugPrint & verboseMotorTimeout) Serial.print(speed);

  motorControllerSerial.write(speed);
  motor2OnAt = millis();

  if (debugPrint & verboseMotorTimeout) Serial.print(" at time = ");
  if (debugPrint & verboseMotorTimeout) Serial.print(motor2OnAt);
  if (debugPrint ) Serial.println();
}

void stopBothMotors() {
  if (debugPrint & verboseMotor) Serial.println("stopBothMotors");
  motorControllerSerial.write((byte)0);
  motor1OnAt = -1;
  motor2OnAt = -1;
}

void stopMotor1() {
  motorControllerSerial.write(64);
  motor1OnAt = -1;
}

void stopMotor2() {
  motorControllerSerial.write(192);
  motor2OnAt = -1;
}
