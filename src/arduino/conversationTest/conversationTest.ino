/*
	Author: Michael Shiloh
  Based on ver3

	Change log

  16 Mar 2025 - ms - initial entry


	TODO
	1. Sending Neopixel commands messes up RC decoding


	Notes for future people hoping to make this work:

	Copy from the `lib/` directory the `Face/`, and `SabertoothMotorControllerSerial3/`
	to your Arduino libraries directory. Install any others using Arduino's interface,
	This should include `Adafruit_NeoPixel` and `Adafruit_VS1053` (SD will be downloaded
	as a dependency).
*/

/*
	Libraries
*/
#include <Adafruit_NeoPixel.h>
#include <Face.h>
#include <SabertoothMotorControllerSerial3.h>
#include <Adafruit_VS1053.h>  // music maker shield
#include <SD.h>               // music maker shield


/*
	Debug
	- Set to `true` for Serial output
*/
const bool debug = true;


/*
	Pin usage
*/

// Front panel switches
const int BUTTON_ONE_PIN = 10;
const int BUTTON_TWO_PIN = 11;
const int BUTTON_THREE_PIN = 12;
const int BUTTON_FOUR_PIN = 13;

// The Music Maker Shield uses SPI which is on pins
// 50, 51, 52, and 53 on the Mega

const int FACE_NEOPIXEL_PIN = 16;


// Pins 14 and 15 will be used for Serial port 3
const int RC_CH1_PIN = 21;  // Steering (was set to pin 3) [min=980 max=2036]
const int RC_CH2_PIN = 2;   // Throttle [min=975 max=2036]
const int RC_CH3_PIN = 18;  // Small knob (VR) [min=902 max=1900]
const int RC_CH4_PIN = 19;  // Silver circular button (SWA) [min=976 max=2036] (1ms when not pressed, 2ms when pressed)
const int RC_CH5_PIN = 20;  // Long clear button (SWD) [min=976 max=2036] (1ms when not pressed, 2ms when pressed)

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
const int RC_NUM_CHANNELS = 5;     // How many radio channels
const int neoPixelFaceCount = 60;  // How many NeoPixels

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
#define RC_CH1 0
#define RC_CH2 1
#define RC_CH3 2
#define RC_CH4 3
#define RC_CH5 4

// Face expressions
const int FACE_STATE_SURPRISED = 0;
const int FACE_STATE_ANGRY = 1;
const int FACE_STATE_SMILE = 2;
const int FACE_STATE_HAPPY = 2;
const int FACE_STATE_FROWN = 3;
const int FACE_STATE_SAD = 3;
const int FACE_STATE_EYES_LEFT = 4;
const int FACE_STATE_EYES_RIGHT = 5;
const int FACE_STATE_FLAG = 6;
const int FACE_STATE_FLAG_COLOURS = 7;
const int FACE_STATE_WAITING = 8;
const int FACE_STATE_GREETING = 9;
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

// Define states for state machine
//
//

const int STATE_IDLE = 0;
const int STATE_RESET = 90;
const int STATE_DRIVE = 99;
const int STATE_GREETING = 1;
const int STATE_WAITING_AFTER_GREETING = 2;
const int STATE_GREETING_ANSWER_YES = 3;
const int STATE_GREETING_ANSWER_NO = 4;
const int STATE_WAITING_AFTER_QUESTION1 = 5;
const int STATE_WAITING_AFTER_QUESTION2 = 6;
int currentState;
int previousState;

// Define radio commands
const int RADIO_COMMAND_NONE = 0;
const int RADIO_COMMAND_DRIVE = 1;
const int RADIO_COMMAND_INIT_CONV = 2;
int previousRadioCommand;

void setup() {

  if (debug) {
    Serial.begin(9600);
  }

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

  // Initialize previous radio command state
  // so that we can detect a transition
  int previousRadioCommand = RADIO_COMMAND_NONE;

  // Initialize state machine
  currentState = STATE_IDLE;
  previousState = STATE_IDLE;

  if (debug) {
    Serial.println("setup finished");
  }
}

void loop() {

  // Check if the radio is sending any command
  switch (int currentRadioCommand = hobbyRCCommand()) {

    case RADIO_COMMAND_NONE:
      // don't change state
      previousRadioCommand = currentRadioCommand;
      break;

    case RADIO_COMMAND_DRIVE:
      currentState = STATE_DRIVE;
      previousRadioCommand = currentRadioCommand;
      break;

    case RADIO_COMMAND_INIT_CONV:
      // first make sure this is a transition
      if (previousRadioCommand != currentRadioCommand) {
        currentState = STATE_GREETING;
        if (debug) Serial.println("radio requested initiating conversation");
      }
      previousRadioCommand = currentRadioCommand;
      break;

    default:
      if (debug) Serial.println("unexpected return value from hobbyRCCommand()");
      previousRadioCommand = currentRadioCommand;
      break;
  }

  switch (currentState) {

    case STATE_IDLE:
      // do nothing
      break;

    case STATE_DRIVE:
      // nothing to do since it's already driving
      break;

    case STATE_GREETING:
      // express the greeting face
      newFaceState = FACE_STATE_GREETING;

      // Speak the greeting
      Serial.println("Hello, I am a robot! Can I ask you some questions?");

      // Speak the instructions
      Serial.println("to answer the question, press button 1 for yes, 2 for no, or 3 to repeat the question");

      // proceed to next state
      currentState = STATE_WAITING_AFTER_GREETING;

      if (0) {
        Serial.print("initiating conversation, setting state to ");
        Serial.println(currentState);
      }

      break;

    case STATE_WAITING_AFTER_GREETING:
      // we have spoken greeting, now check for an answer
      //
      // express the waiting face
      newFaceState = FACE_STATE_WAITING;


      // see if we have an answer
      if (!digitalRead(BUTTON_ONE_PIN)) {
        currentState = STATE_GREETING_ANSWER_YES;
        break;
      }
      if (!digitalRead(BUTTON_TWO_PIN)) {
        currentState = STATE_GREETING_ANSWER_NO;
        break;
      }

      break;


    case STATE_GREETING_ANSWER_YES:
      // express the greeting face
      newFaceState = FACE_STATE_HAPPY;

      // Speak the greeting
      Serial.println("I am so excited! Are you excited to be speaking with a robot?");

      // Speak the instructions
      Serial.println("to answer the question, press button 1 for yes, 2 for no, or 3 to repeat the question");

      // proceed to next state
      currentState = STATE_WAITING_AFTER_QUESTION1;

      break;

    case STATE_GREETING_ANSWER_NO:
      // express the greeting face
      newFaceState = FACE_STATE_SAD;

      // Speak the greeting
      Serial.println("I am sorry that you are not excited. Are you frightened by robots?");

      // Speak the instructions
      Serial.println("to answer the question, press button 1 for yes, 2 for no, or 3 to repeat the question");

      // proceed to next state
      currentState = STATE_WAITING_AFTER_QUESTION2;

      break;

     
      case STATE_WAITING_AFTER_QUESTION2:
        Serial.println("not implemented yet");
        break;

    case STATE_WAITING_AFTER_QUESTION1:
      // we have spoken greeting, now check for an answer
      //
      // express the waiting face
      newFaceState = FACE_STATE_WAITING;

      // see if we have an answer
      if (!digitalRead(BUTTON_ONE_PIN)) {
        currentState = STATE_GREETING_ANSWER_YES;
        break;
      }
      if (!digitalRead(BUTTON_TWO_PIN)) {
        currentState = STATE_GREETING_ANSWER_NO;
        break;
      }

      break;

    case STATE_RESET:
      // do any necessary resetting here
      // return to idle state
      currentState = STATE_IDLE;


    default:
      // indicate an error
      Serial.println("Unexpected state; resetting");
      currentState = STATE_RESET;
      break;
  }
}


// check the channels of the hobby RC receiver
// return TRUE if a new value is found
//
int getRadioCommand() {

  // Read all 5 radio channels

  // Decide whether we are doing nothing, driving, or starting a conversation

  // return appropriate

  return;
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

void setupHobbyRC() {
  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH3_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);
  pinMode(RC_CH5_PIN, INPUT);

  //	enableInterrupt(RC_CH1_PIN, calc_ch1, CHANGE);
  //	enableInterrupt(RC_CH2_PIN, calc_ch2, CHANGE);
  //	enableInterrupt(RC_CH3_PIN, calc_ch3, CHANGE);
  //	enableInterrupt(RC_CH4_PIN, calc_ch4, CHANGE);
  //	enableInterrupt(RC_CH5_PIN, calc_ch5, CHANGE);

  attachInterrupt(digitalPinToInterrupt(RC_CH5_PIN), calc_ch5, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH4_PIN), calc_ch4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH3_PIN), calc_ch3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH2_PIN), calc_ch2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH1_PIN), calc_ch1, CHANGE);
}


/* check the hobby RC receiver, and see if we have any commands. 
If there is a command to drive, do so immediately, and return the drive state.
If there is a command to start a conversation, return the start conversation state
Otherwise, return none
*/
int hobbyRCCommand() {
  int retval = RADIO_COMMAND_NONE;  // are we driving, idling, or entering command mode?

  rc_read_values();

  if (0) {
    // Right now just print them out
    Serial.print("CH1:");
    Serial.print(rc_values[RC_CH1]);
    Serial.print("\t\t");
    Serial.print("CH2:");
    Serial.print(rc_values[RC_CH2]);
    Serial.print("\t\t");
    Serial.print("CH3:");
    Serial.print(rc_values[RC_CH3]);
    Serial.print("\t\t");
    Serial.print("CH4:");
    Serial.print(rc_values[RC_CH4]);
    Serial.print("\t\t");
    Serial.print("CH5:");
    Serial.print(rc_values[RC_CH5]);
    Serial.println("\t\t");
  }

  // Channel 1 is the steering wheel
  if (rc_values[RC_CH1] > 1600) {
    if (debug) {
      Serial.println("right!");
    }
    myMotorController.right(180);
    retval = RADIO_COMMAND_DRIVE;
  } else if (rc_values[RC_CH1] < 1400) {
    if (debug) {
      Serial.println("left!");
    }
    myMotorController.left(180);
    retval = RADIO_COMMAND_DRIVE;
  }

  // Channel 2 is the throttle
  if (rc_values[RC_CH2] < 1400) {
    if (debug) {
      Serial.println("forward!");
    }
    int go = map(constrain(rc_values[RC_CH2], 900, 1400), 1400, 900, 0, 255);
    myMotorController.forward(go);
    retval = RADIO_COMMAND_DRIVE;
  } else if (rc_values[RC_CH2] > 1600) {
    if (debug) {
      Serial.println("backward!");
    }
    int go = map(constrain(rc_values[RC_CH2], 1600, 2000), 1600, 2000, 0, 255);
    myMotorController.backward(go);
    retval = RADIO_COMMAND_DRIVE;
  }

  if (rc_values[RC_CH4] > 1600) {
    retval = RADIO_COMMAND_INIT_CONV;
  }


  return retval;
}
