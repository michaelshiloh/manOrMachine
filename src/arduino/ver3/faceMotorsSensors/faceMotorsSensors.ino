
/*
    using Arduino Uno for simplicity
    based on the LoRa version

    05 Nov 2023 - ms - copied from LoRa and removed a ton of stuff
                       face and motors work with commands from
                       serial port; no sensors yet
    06 Nov 2023 - ms - add sensors
    11 Nov 2023 - ms - modified sensors to require triggering, to prevent interference
    12 Nov 2023 - ms - now add some facial expressions
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

  // "To command a range cycle, bring the RX pin high
  // for at least 20us but less than 48 msec. This will
  // start the sensor chain"
  digitalWrite(SENSOR_TRIGGER_PIN, HIGH); // takes about 3 us
  delayMicroseconds(22); // bit of a safety margin
  digitalWrite(SENSOR_TRIGGER_PIN, LOW);

  // Although the sensors take 49ms to measure, since
  // it's buffered I can read one after the other
  int leftDistance = analogRead(LEFT_DIST_SENSOR);
  delay(1);
  int frontDistance = analogRead(MIDDLE_DIST_SENSOR);
  delay(1);
  int rightDistance = analogRead(RIGHT_DIST_SENSOR);
  delay(1);

  Serial.print("left ");
  Serial.print(leftDistance);
  Serial.print("\t");
  Serial.print(frontDistance);
  Serial.print("\t");
  Serial.print("right ");
  Serial.print(rightDistance);
  Serial.println();

  // First check to see if we're stuck
  if (leftDistance < 20 && frontDistance < 20 && rightDistance < 20 ) {
    Serial.println("Nowhere to go. I'm stuck");
    myMotorController.forward(0);
    robotFace.clear();
    robotFace.frown();
  }

  // Is there room to go forward?
  // Only if there is room in front and the right and left distances are about the same
  else if (frontDistance > 20 ) { //&& abs(leftDistance - rightDistance) < 20) {
    Serial.println("forward");
    myMotorController.forward(150);
    robotFace.clear();
    robotFace.smile();
  }

  // Otherwise, figure out which way to turn
  // This seems to favor right turns. Is that due
  // to logic or different sensor behavior?
  else if (leftDistance > rightDistance) { // more room on the left so turn that way
    Serial.println("left");
    myMotorController.left(150);
    robotFace.clear();
    robotFace.eyesLeft();
  }
  else {
    Serial.println("right");
    myMotorController.right(150);
    robotFace.clear();
    robotFace.eyesRight();
  }

  if (Serial.available()) {
    char inChar = (char)Serial.read();
    updateMotors(inChar);
  }

  myMotorController.tick(); // this must be called regularly so never use delay()

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
      robotFace.clear();
      robotFace.smile();
      break;
    case 'b':
    case 'B':
      myMotorController.backward(80);
      robotFace.clear();
      robotFace.frown();
      break;
    case 'l':
    case 'L':
      myMotorController.left(80);
      robotFace.clear();
      robotFace.eyesLeft();
      break;
    case 'r':
    case 'R':
      myMotorController.right(80);
      robotFace.clear();
      robotFace.eyesRight();
      break;

    default:
      myMotorController.forward(0);
      Serial.println("invalid message");
  }
}
