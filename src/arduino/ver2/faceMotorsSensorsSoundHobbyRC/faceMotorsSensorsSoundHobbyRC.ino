
/*
    Based on faceMotorsSensors

    25 Nov 2023 - ms - adding hobby RC receiver and Music Maker Shield.
                       Probably I should switch to Arduino Mega for more
                       memory. Music Maker Shield uses 3, 4, 6, and 7
                       in addition to SPI, so I'll need to move the
                       motor controller pins and the sensor trigger
                       pin. Hobby RC needs 4 which I think can be any
                       I think it will be quicker to make a new shield
                       Stack: Mega - Hobby RC - MMS - local connections
                       so remember to use stacking headers on Hobby RC
                       and MMS
    26 Nov 2023 - ms - Add code for hobby RC and MMS
    26 Nov 2023 - ms - Arduino Mega has 4 UARTs so no need for software
                       serial but must move motor controller to UART3
                       on pins 14 and 15. Rx pin is not used. Tx
                       was on pin 4 (blue) move to pin 14
    26 Nov 2023 - ms - Remove EnableInterrupt.h library and use native
                       attachInterrupt as it conflicts with VS1053 lib
    26 Nov 2023 - ms - Move channels 1 and 2 to pins 2 and 3 for hardware
                       interrupts 0 and 1. this requires moving NeoPixel
                       from pin 2 (yellow) elsewhere; let's put it on pin 16
    26 Nov 2023 - ms - use header strip for motor controller TX and
                       NeoPixel
                       MC TX (blue) = 14
                       NP  (yellow) = 16
    27 Nov 2023 - ms - Problem: Changing NeoPixels messes up the hobby RC
                       readings. Workaround by changing them only when knob
                       rotates to a different position. It still jerks
                       occassionally but at least I can choose when to do this
    14 Dec 2023 - ms - Add 4 momentary switches using internal pullup resistors


  TODO
  1. Music maker shield needs an interrupt pin so remove channel 6
  because my transmitter only has 5 channels anyway
  2. Sending Neopixel commands messes up RC decoding
  3. Faster steering


*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <Face.h>
#include <SabertoothMotorControllerSerial3.h>
#include <Adafruit_VS1053.h> // music maker shield
#include <SD.h> // music maker shield


/*
   Pin usage
*/
//// unused pins
// 5, 8, 9

// Front panel switches
const int BUTTON_ONE_PIN = 10;
const int BUTTON_TWO_PIN = 11;
const int BUTTON_THREE_PIN = 12;
const int BUTTON_FOUR_PIN = 13;

// The Music Maker Shield uses SPI which is on pins
// 50, 51, 52, and 53 on the Mega

const int FACE_NEOPIXEL_PIN = 16;
const int SENSOR_TRIGGER_PIN = 7;
const int LEFT_DIST_SENSOR = A0;
const int MIDDLE_DIST_SENSOR = A1;
const int RIGHT_DIST_SENSOR = A2;
// Pins 14 and 15 will be used for Serial port 3
const int RC_CH1_PIN = 3; // steering
const int RC_CH2_PIN = 2; // throttle
const int RC_CH3_PIN = 18; //
const int RC_CH4_PIN = 19;
const int RC_CH5_PIN = 20;
const int RC_CH6_PIN = 21;

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
//#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
//Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);



/*
    Other global variables
*/


const int RC_NUM_CHANNELS = 6;    // How many radio channels
const int neoPixelFaceCount = 60; // How many NeoPixels

// Hopefully reasonable readings of distance sensor (unverified)
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
const int MOTORTIMEOUT = 100;

// Indexes into the array of hobby RC radio channels
#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3
#define RC_CH5  4
#define RC_CH6  5

// Face expressions

const int FACE_STATE_SURPRISED = 0;
const int FACE_STATE_ANGRY = 1;
const int FACE_STATE_SMILE = 2;
const int FACE_STATE_FROWN = 3;
const int FACE_STATE_EYES_LEFT = 4;
const int FACE_STATE_EYES_RIGHT = 5;
const int FACE_STATE_FLAG = 6;
const int FACE_STATE_FLAG_COLOURS = 7;
int newFaceState = FACE_STATE_SMILE;
int presentFaceState = -1;

/*
   global objects
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);

SabertoothMotorControllerSerial3 myMotorController(MOTORTIMEOUT);

// Arrays for storing hobby RC values
uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

void setup() {

  Serial.begin(9600);

  // Sensors need a trigger to start sending data
  pinMode(SENSOR_TRIGGER_PIN, OUTPUT);
  digitalWrite(SENSOR_TRIGGER_PIN, LOW);

  myMotorController.init();

  // Neopixels for face
  robotFace.init();

  // So the robot can speak
  //setupMusicMakerShield();

  // Read the hobby RC signals
  setupHobbyRC();

  // And finally the front panels switches
  pinMode(BUTTON_ONE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TWO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_THREE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_FOUR_PIN, INPUT_PULLUP);
}

void loop() {
  Serial.print("new = " );
  Serial.print(newFaceState );
  Serial.print("present = " );
  Serial.println(presentFaceState );


  // For the end of semester party, allow the switches to
  // control the face

  if (!digitalRead(BUTTON_ONE_PIN)) {
    Serial.println("BUTTON_ONE_PIN" );
    newFaceState = FACE_STATE_SURPRISED;
  }
  if (!digitalRead(BUTTON_TWO_PIN)) {
    Serial.println("BUTTON_TWO_PIN" );
    newFaceState = FACE_STATE_ANGRY;
  }
  if (!digitalRead(BUTTON_THREE_PIN)) {
    Serial.println("BUTTON_THREE_PIN" );
    newFaceState = FACE_STATE_EYES_LEFT;
  }
  if (!digitalRead(BUTTON_FOUR_PIN)) {
    Serial.println("BUTTON_FOUR_PIN" );
    newFaceState = FACE_STATE_FLAG;
  }

  if (newFaceState != presentFaceState) {
    switch (newFaceState) {
      case FACE_STATE_SURPRISED:
        robotFace.clear();
        robotFace.surprised();
        break;
      case FACE_STATE_ANGRY:
        robotFace.clear();
        robotFace.angry();
        break;
      case FACE_STATE_SMILE:
        robotFace.clear();
        robotFace.smile();
        break;
      case FACE_STATE_FROWN:
        robotFace.clear();
        robotFace.frown();
        break;
      case FACE_STATE_EYES_LEFT:
        robotFace.clear();
        robotFace.eyesLeft();
        break;
      case FACE_STATE_EYES_RIGHT:
        robotFace.clear();
        robotFace.eyesRight();
        break;
      case FACE_STATE_FLAG:
        robotFace.clear();
        robotFace.flag();
        break;
      case FACE_STATE_FLAG_COLOURS:
        robotFace.clear();
        robotFace.flagColours();
        break;
      default:
        Serial.println("Invalid face state");
    }
    presentFaceState = newFaceState;

  }

  // Top priority is given to the hobby RC commands:
  if (hobbyRCCommand()) {
    return;
  }

  // Next priority is given to commands from the
  // Raspberry Pi
  if (raspBerryPiCommand()) {
    return;
  }

  // Normally, just wander aimlessly avoiding obstacles
  //avoidObstacles();

  // and keep the motors running
  // this must be called regularly so never use delay()
  myMotorController.tick();

}


void avoidObstacles() {
  // "To command a range cycle, bring the RX pin high
  // for at least 20us but less than 48 msec. This will
  // start the sensor chain"
  digitalWrite(SENSOR_TRIGGER_PIN, HIGH); // takes about 3 us
  delayMicroseconds(22); // bit of a safety margin
  digitalWrite(SENSOR_TRIGGER_PIN, LOW);

  // The sensors take 49ms to measure
  delay(49); // TODO get rid of delay as this hurts responsiveness

  int leftDistance = analogRead(LEFT_DIST_SENSOR);
  delay(1);
  int frontDistance = analogRead(MIDDLE_DIST_SENSOR);
  delay(1);
  int rightDistance = analogRead(RIGHT_DIST_SENSOR);
  delay(1);

  //  Serial.print("left ");
  //  Serial.print(leftDistance);
  //  Serial.print("\t");
  //  Serial.print(frontDistance);
  //  Serial.print("\t");
  //  Serial.print("right ");
  //  Serial.print(rightDistance);
  //  Serial.println();

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
    //    Serial.println("forward");
    myMotorController.forward(150);
    robotFace.clear();
    robotFace.smile();
  }

  // Otherwise, figure out which way to turn
  // This seems to favor right turns. Is that due
  // to logic or different sensor behavior?
  else if (leftDistance > rightDistance) { // more room on the left so turn that way
    //    Serial.println("left");
    myMotorController.left(150);
    robotFace.clear();
    robotFace.eyesLeft();
  }
  else {
    //    Serial.println("right");
    myMotorController.right(150);
    robotFace.clear();
    robotFace.eyesRight();
  }
}



// Commands from the Raspberry Pi or the serial monitor
bool raspBerryPiCommand() {
  // read the character we receive on the serial port from the RPi
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (debugPrint & verboseMotor) Serial.print("listenHardwareSerialPort: received character ]");
    if (debugPrint & verboseMotor) Serial.print(inChar);
    if (debugPrint & verboseMotor) Serial.print("[");
    if (debugPrint & verboseMotor) Serial.println();

    updateMotors(inChar);
    return (true);
  }
  return (false);
}

bool hobbyRCCommand() {

  rc_read_values();

  // Right now just print them out
  Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t\t");
  Serial.print("CH2:"); Serial.print(rc_values[RC_CH2]); Serial.print("\t\t");
  Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t\t");
  Serial.print("CH4:"); Serial.print(rc_values[RC_CH4]); Serial.print("\t\t");
  Serial.print("CH5:"); Serial.print(rc_values[RC_CH5]); Serial.println("\t\t");
  //Serial.print("CH6:"); Serial.println(rc_values[RC_CH6]);
  /*
    // This is a bug, as it doesn't happen with the raw decoding example
    // but until I find that bug, this hack:
    // If either the throttle or the steering wheel are outside of a reasonable
    // range set them to 1500
    if (rc_values[RC_CH1] > 2100 || rc_values[RC_CH1] < 900) {
      rc_values[RC_CH1] = 1500;
      //while(1);
    }
    if (rc_values[RC_CH2] > 2100 || rc_values[RC_CH2] < 900) {
      rc_values[RC_CH2] = 1500;
      //while(1);
    }
    if (rc_values[RC_CH3] > 2100 || rc_values[RC_CH3] < 900) {
      rc_values[RC_CH3] = 1500;
      //while(1);
    }
    if (rc_values[RC_CH4] > 2100 || rc_values[RC_CH4] < 900) {
      rc_values[RC_CH4] = 1500;
      //while(1);
    }
    if (rc_values[RC_CH5] > 2100 || rc_values[RC_CH5] < 900) {
      rc_values[RC_CH5] = 1500;
      //while(1);
    }
  */
  // Channel 1 is the steering wheel
  if (rc_values[RC_CH1] > 1600) {
    Serial.print("right!");
    myMotorController.right(180);
  } else if (rc_values[RC_CH1] < 1400) {
    Serial.print("left!");
    myMotorController.left(180);
  }

  // Channel 2 is the throttle
  if (rc_values[RC_CH2] < 1400) {
    Serial.print("forward!");
    int go = map(constrain( rc_values[RC_CH2], 900, 1400), 1400, 900, 0, 255);
    myMotorController.forward(go);
  } else if (rc_values[RC_CH2] > 1600) {
    Serial.print("backward!");
    int go = map(constrain( rc_values[RC_CH2], 1600, 2000), 1600, 2000, 0, 255);
    myMotorController.backward(go);
  }
/*
  // Channel 3 is the knob
  if (rc_values[RC_CH3] > 900rc_values[RC_CH3] < 1100) {
    Serial.print("smile!");
    newFaceState = FACE_STATE_ANGRY;
  }
  if (rc_values[RC_CH3] > 1100 && rc_values[RC_CH3] < 1250) {
    Serial.print("frown!");
    newFaceState = FACE_STATE_SMILE;
  }
  if (rc_values[RC_CH3] > 1250 && rc_values[RC_CH3] < 1400) {
    Serial.print("flag!");
    newFaceState = FACE_STATE_FROWN;
  }
  if (rc_values[RC_CH3] > 1400 && rc_values[RC_CH3] < 1550) {
    Serial.print("frown!");
    newFaceState = FACE_STATE_EYES_LEFT;
  }
  if (rc_values[RC_CH3] > 1550 && rc_values[RC_CH3] < 1700) {
    Serial.print("frown!");
    newFaceState = FACE_STATE_EYES_RIGHT;
  }
  if (rc_values[RC_CH3] > 1700 && rc_values[RC_CH3] < 1850) {
    Serial.print("flag!");
    newFaceState = FACE_STATE_FLAG;
  }
  if (rc_values[RC_CH3] > 1850) {
    Serial.print("flag!");
    newFaceState = FACE_STATE_FLAG_COLOURS;
  }

  if (newFaceState != presentFaceState) {
    switch (newFaceState) {
      case FACE_STATE_ANGRY:
        robotFace.clear();
        robotFace.angry();
        break;
      case FACE_STATE_SMILE:
        robotFace.clear();
        robotFace.smile();
        break;
      case FACE_STATE_FROWN:
        robotFace.clear();
        robotFace.frown();
        break;
      case FACE_STATE_EYES_LEFT:
        robotFace.clear();
        robotFace.eyesLeft();
        break;
      case FACE_STATE_EYES_RIGHT:
        robotFace.clear();
        robotFace.eyesRight();
        break;
      case FACE_STATE_FLAG:
        robotFace.clear();
        robotFace.flag();
        break;
      case FACE_STATE_FLAG_COLOURS:
        robotFace.clear();
        robotFace.flagColours();
        break;
      default:
        Serial.println("Invalid face state");
    }
    presentFaceState = newFaceState;
  }
*/
  return (false); // later will override if needed

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

void setupMusicMakerShield() {
  //Commented out for now until I fix the interrupt vector redefinition error

  //  if (!musicPlayer.begin()) {  // initialise the music player
  //    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
  //    while (1)
  //      ;
  //  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  //  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  //  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

// Hobby RC functions
void rc_read_values() {
  noInterrupts();
  memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
  interrupts();
}

void calc_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
  }
}

void calc_ch1() {
  calc_input(RC_CH1, RC_CH1_PIN);
}
void calc_ch2() {
  calc_input(RC_CH2, RC_CH2_PIN);
}
void calc_ch3() {
  calc_input(RC_CH3, RC_CH3_PIN);
}
void calc_ch4() {
  calc_input(RC_CH4, RC_CH4_PIN);
}
void calc_ch5() {
  calc_input(RC_CH5, RC_CH5_PIN);
}
void calc_ch6() {
  calc_input(RC_CH6, RC_CH6_PIN);
}

void setupHobbyRC() {
  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH3_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);
  pinMode(RC_CH5_PIN, INPUT);
  pinMode(RC_CH6_PIN, INPUT);

  //  enableInterrupt(RC_CH1_PIN, calc_ch1, CHANGE);
  //  enableInterrupt(RC_CH2_PIN, calc_ch2, CHANGE);
  //  enableInterrupt(RC_CH3_PIN, calc_ch3, CHANGE);
  //  enableInterrupt(RC_CH4_PIN, calc_ch4, CHANGE);
  //  enableInterrupt(RC_CH5_PIN, calc_ch5, CHANGE);
  //  enableInterrupt(RC_CH6_PIN, calc_ch6, CHANGE);

  attachInterrupt(digitalPinToInterrupt(RC_CH6_PIN), calc_ch6, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH5_PIN), calc_ch5, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH4_PIN), calc_ch4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH3_PIN), calc_ch3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH2_PIN), calc_ch2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH1_PIN), calc_ch1, CHANGE);
}
