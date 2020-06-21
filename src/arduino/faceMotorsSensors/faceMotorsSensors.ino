
/*
    Finally try combining all three features

    TODO
    - full description
    - classes for face, motors, sensors
    - remove delays in sensors
    - put pins in order
*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <Face.h>

/*
   Pin assignments
*/

const int distanceSensorTxPin = 2; // actually not used but need it to construct object
const int motorControllerRXPin = 4; // actually not used but need it to construct object
const int neoPixelFacePin = 7;
const int motorControllerTXPin = 8;
const int distanceSensorRxPin = 10;
const int maxsonic = 11; // should i save this for pwm


/*
    Other global variables
*/
const int neoPixelFaceCount = 60;

/*
   global objects
*/
Face robotFace(neoPixelFaceCount, neoPixelFacePin);

void setup() {

  Serial.begin(9600);
  robotFace.init();
  setupMotors();
}

void loop() {


  Serial.println("top of loop");
  robotFace.clear();
  robotFace.smile();
  delay(1000);
  robotFace.clear();
  robotFace.frown();
  delay(1000);
  updateMotors();
  int distance;
  // distance = doReadingMyWay();

  if (distance > 36) {

    moveForward(200);
    delay(1000);
  } else {
    turnLeft(100);
    delay(1000);
  }
}