
/*
    Finally try combining all three features

    19 jun 2020 - ms - initial integration
    20 jun 2020 - ms - library for face
    21 jun 2020 - ms - maxsonic sensor
    25 jun 2020 - ms - start adding control panel

    TODO
    - full description
    - classes for motors, sensors?
    - remove delays in sensors
    - put pins in order
    - emergency stop
    - currently, motors continue turning until there is a command to stop.
      This is useful for testing but dangerous.
*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Face.h>

/*
   Pin assignments
*/

// unused: 3, 5, 6
const int distanceSensorTxPin = 2; // actually not used but need it to construct object
const int motorControllerRXPin = 4; // actually not used but need it to construct object
const int neoPixelFacePin = 7;
const int motorControllerTXPin = 8;
const int controlPanelTxPin = 9; // yellow wire
const int distanceSensorRxPin = 10;
const int maxsonic = 11;
const int controlPanelRxPin = 12; // red wire


/*
    Other global variables
*/
const int neoPixelFaceCount = 60;

// reasonable readings of distance sensor (unverified)
const int MIN_DISTANCE = 6;
const int MAX_DISTANCE = 200;

// Control verbosity of messages
const int debugPrint = 12;
const int verboseDistance = 1;
const int reportDistance = 2;
const int verboseMotor = 4;
const int debugMotor = 8;

// for controlling the motors
const int MYFORWARD = 0;
const int MYREVERSE = 1;
int forwardSpeed = 80;


/*
   global objects
*/
Face robotFace(neoPixelFaceCount, neoPixelFacePin);
SoftwareSerial motorControllerSerialPort(motorControllerRXPin, motorControllerTXPin); // RX, TX
SoftwareSerial controlPanelSerial(controlPanelRxPin, controlPanelTxPin); // RX, TX

void controlPanelInit() {
}


void setup() {

  Serial.begin(9600);
  controlPanelSerial.begin(9600);

  motorControllerSerialPort.begin(9600);
  delay(100); // why?

  robotFace.init();
  setupMotors();
  sensorsInit();
}

void loop() {

  //  robotFace.clear();
  //  robotFace.smile();
  //  delay(1000);
  //  robotFace.clear();
  //  robotFace.frown();
  //  delay(1000);

  listenHardwareSerialPort();
  listenControlPanelSerialPort();
  // if there were other forms of control e.g. wireless could add that here

  // Serial.println("measure distance");
  int distance;
  distance = doReadingMyWay();

  // Only move if the reading is valid
  if ( distance == constrain( distance, MIN_DISTANCE, MAX_DISTANCE)) {

    //    if (distance > 50) {
    //
    //      moveForward(80);
    //      delay(1000);
    //    } else {
    //      turnLeft(80);
    //      delay(1000);
    //    }
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


// Listen for commands from the control panel

void listenControlPanelSerialPort() {
  // Serial.println("checking port");
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it:
  controlPanelSerial.listen();
  delay(50);
  if (int foo = controlPanelSerial.available()) {
    Serial.print("listenControlPanelSerialPort: ");
    Serial.print(foo);
    Serial.print(" bytes available\t");
    char inChar = (char) controlPanelSerial.read();
    if (inChar) {
      Serial.print(" received character ]");

      Serial.print((int)inChar);
      Serial.print("[");
      Serial.println();
    } else {
      Serial.println("null byte received");
    }

    switch (inChar) {
      case 4:
        updateMotors('L');
        break;
      case 1:
        updateMotors('R');
        break;
      case 2:
        updateMotors('F');
        break;
      case 0:
        updateMotors('S');
        break;
    }
  }
}
