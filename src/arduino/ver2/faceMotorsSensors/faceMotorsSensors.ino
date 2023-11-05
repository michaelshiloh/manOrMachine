
/*
    using Arduino Uno for simplicity
    based on the LoRa version

    05 Nov 2023 - ms - copied from LoRa and removed a ton of stuff
*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Face.h>

#include <SabertoothMotorController.h>

/*
   Pin usage
*/

const int FACE_NEOPIXEL_PIN = 2;
const int MC_RCV_PIN = 3; // actually unused
const int MC_XMIT_PIN = 4; // transmit to motor controller
const int DIST_SENSOR_0 = A0;
const int DIST_SENSOR_1 = A1;
const int DIST_SENSOR_2 = A2;


/*
    Other global variables
*/
const int neoPixelFaceCount = 60;
const int motorTimeOut = 90;

// reasonable readings of distance sensor (unverified)
const int MIN_DISTANCE = 6;
const int MAX_DISTANCE = 200;

// Control verbosity of messages
const int verboseDistance = 1;
const int reportDistance = 2;
const int verboseMotor = 4;
const int debugMotor = 8;
const int verboseMotorTimeout = 16;
const int debugSensorReadingTick = 32;
const int debugPrint = debugSensorReadingTick;

// for controlling the motors
const int MYFORWARD = 0;
const int MYREVERSE = 1;
int forwardSpeed = 80;

/*
   global objects
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);
SoftwareSerial motorControllerSerialPort(MC_RCV_PIN, MC_XMIT_PIN); // RX, TX

SabertoothMotorController myMotorController(
  MC_RCV_PIN,
  MC_XMIT_PIN,
  motorTimeOut);

void setup() {

  Serial.begin(9600);

  motorControllerSerialPort.begin(9600);
  //delay(100); // why?

  robotFace.init();
  myMotorController.init();
}

void loop() {


    delay(1000);
    robotFace.clear();
    robotFace.frown();
    delay(1000);

  myMotorController.tick(); // this must be called regularly so all other functions here must not use delay()

  // Serial.println("measure distance");
  int distance;
  //  distance = doReadingMyWay();

  

  // Only move if the reading is valid
  if ( distance == constrain( distance, MIN_DISTANCE, MAX_DISTANCE)) {

    //    if (distance > 50) {
    //
    //      moveForward(80);
    //      delay(1000);
    //    } else {
    //      turnLeft(80);
    //      delay(1000);
    //    }while (inChar != 'R') {
  }
}

void listenHardwareSerialPort() {
  // read the character we receive on the serial port from the RPi
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (debugPrint & verboseMotor) Serial.print("listenHardwareSerialPort: received character ]");
    if (debugPrint & verboseMotor) Serial.print(inChar);
    if (debugPrint & verboseMotor) Serial.print("[");
    if (debugPrint & verboseMotor) Serial.println();

    updateMotors(inChar);
  }
}




void updateMotors(char inChar) {
  

  
}
