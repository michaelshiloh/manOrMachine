
/*
    using Arduino Uno for simplicity
    based on the LoRa version

    12 Nov 2023 - ms - no movement; just demo facial expressions
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
const int SENSOR_TRIGGER_PIN = 6;
const int LEFT_DIST_SENSOR = A0;
const int MIDDLE_DIST_SENSOR = A1;
const int RIGHT_DIST_SENSOR = A2;


/*
    Other global variables
*/
const int neoPixelFaceCount = 60;

// reasonable readings of distance sensor (unverified)
const int MIN_DISTANCE = 6;
const int MAX_DISTANCE = 200;

// Control verbosity of messages
const int verboseDistance = 1;
const int reportDistance = 2;
const int verboseMotor = 4;
const int debugMotor = 8;
const int verboseMOTORTIMEOUT = 16;
const int debugSensorReadingTick = 32;
const int debugPrint = 0;

// for controlling the motors
const int MYFORWARD = 0;
const int MYREVERSE = 1;
const int MOTORTIMEOUT = 1000;


/*
   global objects
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);
SoftwareSerial motorControllerSerialPort(MC_RCV_PIN, MC_XMIT_PIN); // RX, TX

SabertoothMotorController myMotorController(
  MC_RCV_PIN,
  MC_XMIT_PIN,
  MOTORTIMEOUT);

void setup() {

  Serial.begin(9600);

  pinMode(SENSOR_TRIGGER_PIN, OUTPUT);
  digitalWrite(SENSOR_TRIGGER_PIN, LOW);

  motorControllerSerialPort.begin(9600);

  robotFace.init();
  myMotorController.init();
}

void loop() {
    robotFace.clear();
    robotFace.smile();
    delay(2000);

    robotFace.clear();
    robotFace.frown();
    delay(2000);
  
    robotFace.clear();
    robotFace.surprised();
    delay(2000);

    robotFace.clear();
    robotFace.angry();
    delay(2000);
  
    robotFace.clear();
    robotFace.eyesLeft();
    delay(2000);
 
    robotFace.clear();
    robotFace.eyesRight();
    delay(2000);
}
