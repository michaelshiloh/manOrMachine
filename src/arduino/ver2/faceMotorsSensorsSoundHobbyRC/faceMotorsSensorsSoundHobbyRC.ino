
/*
    using Arduino Uno for simplicity
    based on the LoRa version

    05 Nov 2023 - ms - copied from LoRa and removed a ton of stuff
                       face and motors work with commands from
                       serial port; no sensors yet
    06 Nov 2023 - ms - add sensors
    11 Nov 2023 - ms - modified sensors to require triggering,
                       to prevent interference
    12 Nov 2023 - ms - now add some facial expressions
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
                       on pins 14 and 15
*/

/*
   Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <Face.h>
#include <SabertoothMotorController.h>
#include <SoftwareSerial.h> // motor controller comms
#include <Adafruit_VS1053.h> // music maker shield
#include <SD.h> // music maker shield
// The hobby RC decoding needs this
// library by Mike "GreyGnome" Schwager
#include <EnableInterrupt.h>

/*
   Pin usage
*/

const int FACE_NEOPIXEL_PIN = 2;
const int MC_RCV_PIN = 8; // prior 3; // actually unused
const int MC_XMIT_PIN = 9; // prior 4; // transmit to motor controller
const int SENSOR_TRIGGER_PIN = 10; // prior 6;
const int LEFT_DIST_SENSOR = A0;
const int MIDDLE_DIST_SENSOR = A1;
const int RIGHT_DIST_SENSOR = A2;
const int RC_CH1_PIN = 16;
const int RC_CH2_PIN = 17;
const int RC_CH3_PIN = 18;
const int RC_CH4_PIN = 19;
const int RC_CH5_PIN = 20;
const int RC_CH6_PIN = 21;
const int RC_NUM_CHANNELS = 6;

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);



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

// Indexes into the array of hobby RC radio channels
#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3
#define RC_CH5  4
#define RC_CH6  5
/*
   global objects
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);
SoftwareSerial motorControllerSerialPort(MC_RCV_PIN, MC_XMIT_PIN); // RX, TX

SabertoothMotorController myMotorController(
  MC_RCV_PIN,
  MC_XMIT_PIN,
  MOTORTIMEOUT);

// Arrays for storing hobby RC values
uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

void setup() {

  Serial.begin(9600);

  // Sensors need a trigger to start sending data
  pinMode(SENSOR_TRIGGER_PIN, OUTPUT);
  digitalWrite(SENSOR_TRIGGER_PIN, LOW);

  // Motor controller needs a serial port
  motorControllerSerialPort.begin(9600);
  myMotorController.init();

  // Neopixels for face
  robotFace.init();

  // So the robot can speak
  setupMusicMakerShield();
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

void setupMusicMakerShield() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
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

  enableInterrupt(RC_CH1_PIN, calc_ch1, CHANGE);
  enableInterrupt(RC_CH2_PIN, calc_ch2, CHANGE);
  enableInterrupt(RC_CH3_PIN, calc_ch3, CHANGE);
  enableInterrupt(RC_CH4_PIN, calc_ch4, CHANGE);
  enableInterrupt(RC_CH5_PIN, calc_ch5, CHANGE);
  enableInterrupt(RC_CH6_PIN, calc_ch6, CHANGE);
}
