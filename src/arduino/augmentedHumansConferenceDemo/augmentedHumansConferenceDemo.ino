/*
	Author: Michael Shiloh
  Based on conversationTest and conversationInStructure

	Change log

  18 Mar 2025 - ms - initial entry


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

0 
1 
2 RC_CH2_PIN
3 DREQ 
4 CARDCS 
5 
6 SHIELD_DCS  
7 SHIELD_CS  
8 
9 
10 BUTTON_ONE_PIN 
11 BUTTON_TWO_PIN
12 BUTTON_THREE_PIN
13 BUTTON_FOUR_PIN 
14 
15 
16 FACE_NEOPIXEL_PIN 
17 
18 RC_CH3_PIN 
19 RC_CH4_PIN 
20 RC_CH5_PIN 
21 RC_CH1_PIN 
22 
23 
24 
25 
26 
27
28 
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50 MISO The Music Maker Shield uses SPI
51 MOSI The Music Maker Shield uses SPI
52 SCK  The Music Maker Shield uses SPI
53 SS   The Music Maker Shield uses SPI
*/


// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);


// Face
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
const int neoPixelFaceCount = 60;  // How many NeoPixels
const int FACE_NEOPIXEL_PIN = 16;
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);


/*
Front panel buttons
*/
const int FRONT_PANEL_BUTTON_COUNT = 4;
int front_panel_button_states[FRONT_PANEL_BUTTON_COUNT] = { 0, 0, 0, 0 };
const int front_panel_buttons[FRONT_PANEL_BUTTON_COUNT] = { 10, 11, 13, 12 };


/*
Motor Controller
*/
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
// Pins 14 and 15 will be used for Serial port 3
// which should be for the motor controller
//
SabertoothMotorControllerSerial3 myMotorController(MOTORTIMEOUT);


/*
Conversation!
*/
struct Question {
  String audio_file;                      // sound file to play in 8.3 format
  int buttons[FRONT_PANEL_BUTTON_COUNT];  // array of buttons that point to questions
  int face;                               // I really wanted to put the functoin pointer in here
  //void (*faceFunction)(); // pointer to face function void (*funct)(int n);
  // or rewrite the library to take an index and do the case statement in the library
};
// button 1: yes
// button 2: no
Question q0 = { "Hi. Wanna talk?", { 1, 2, 0, 0 }, 0 };
Question q1 = { "Sweet. Do you like rice?", { 3, 4, 0, 0 }, 1 };
Question q2 = { "Goodbye.", { 0, 0, 0, 0 }, 2 };
Question q3 = { "Me too! Wanna talk again?", { 1, 2, 0, 0 }, 3 };
Question q4 = { "Never talk to me again!.. Wanna talk again?", { 1, 2, 0, 0 }, 4 };
const int NUMBER_OF_QUESTIONS = 5;
Question questions[NUMBER_OF_QUESTIONS] = { q0, q1, q2, q3, q4 };
int currentQuestionIndex = 0;


/*
State machine
*/
const int STATE_IDLE = 0;
const int STATE_ASK_QUESTION = 1;
const int STATE_WAITING_SPEAKING_FINISHED = 2;
const int STATE_STARTING_NEW_CONVERSATION = 3;
const int STATE_WAITING_FOR_RESPONSE = 4;
const int STATE_DRIVE = 5;
int currentState;


/* 
Hobby RC receiver
*/
const int RC_CH1_PIN = 21;  // Steering (was set to pin 3) [min=980 max=2036]
const int RC_CH2_PIN = 2;   // Throttle [min=975 max=2036]
const int RC_CH3_PIN = 18;  // Small knob (VR) [min=902 max=1900]
const int RC_CH4_PIN = 19;  // Silver circular button (SWA) [min=976 max=2036] (1ms when not pressed, 2ms when pressed)
const int RC_CH5_PIN = 20;  // Long clear button (SWD) [min=976 max=2036] (1ms when not pressed, 2ms when pressed)
// Indexes into the array of hobby RC radio channels
#define RC_CH1 0
#define RC_CH2 1
#define RC_CH3 2
#define RC_CH4 3
#define RC_CH5 4
const int RC_NUM_CHANNELS = 5;  // How many radio channels
// Arrays for storing hobby RC values
uint16_t rc_values[RC_NUM_CHANNELS] = { 1500, 1500, 1000, 1000, 1000 };
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS] = { 1500, 1500, 1000, 1000, 1000 };
bool rc_valid[RC_NUM_CHANNELS] = { false, false, false, false, false };
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
  if (debug) Serial.println(F("Motor controller initialized"));

  musicMakerShieldInit();
  if (debug) Serial.println(F("Music Maker Shield initialized"));


  // Neopixels for face
  robotFace.init();
  if (debug) Serial.println(F("Face initialized"));


  // Read the hobby RC signals
  hobbyRCInit();
  if (debug) Serial.println(F("Hobby RC initialized"));

  // wait for all channels to be valid
  // this shouldn't take too long
  // assuming the radio transmitter is on
  while (!hobbyRCAreAllChannelsValid()) {
    ;
  }
  // get data once to reset the arrays to reasonable values
  hobbyRCCommand();
  if (debug) Serial.println(F("Received valid Hobby RC signals"));

  for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {
    pinMode(front_panel_buttons[i], INPUT_PULLUP);
  }

  // Initialize previous radio command state
  // so that we can detect a transition
  int previousRadioCommand = RADIO_COMMAND_NONE;

  // Initialize state machine
  // currentState = STATE_IDLE;
  // previousState = STATE_IDLE;
  currentQuestionIndex = 0;
  int currentState = STATE_IDLE;

  if (debug) {
    Serial.println(F("setup finished"));
  }
}

void loop() {

  // keep the motors running
  // this must be called regularly so never use delay()
  myMotorController.tick();

  handleRCSignals();

  myConverse();
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

// How can we determine if we have a valid signal?
// If we catch a rising edge, then we have the start
// of a potentially valid signal
// if the trailing edge is within say 2500 microseconds
// then it's valid

// oh what if we do this:
// anytime we change the face we set a flag saying
// radio signals are invalid, and then
// we set it to valid only after
// all 5 channels have been read
void calc_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
    rc_valid[channel] = true;
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

void hobbyRCInit() {
  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH3_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);
  pinMode(RC_CH5_PIN, INPUT);

  attachInterrupt(digitalPinToInterrupt(RC_CH5_PIN), calc_ch5, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH4_PIN), calc_ch4, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH3_PIN), calc_ch3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH2_PIN), calc_ch2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RC_CH1_PIN), calc_ch1, CHANGE);

  hobbyRCSetAllChannelsInvalid();
}

bool hobbyRCAreAllChannelsValid() {
  // are any channels invalid?
  for (int i = 0; i < RC_NUM_CHANNELS; i++) {
    if (!rc_valid[i]) {
      return false;
    }
  }
  return true;
}

void hobbyRCSetAllChannelsInvalid() {
  for (int i = 0; i < RC_NUM_CHANNELS; i++) {
    rc_valid[i] = false;
  }
}

/* check the hobby RC receiver, and see if we have any commands. 
If there is a command to drive, do so immediately, and return the drive state.
If there is a command to start a conversation, return the start conversation state
Otherwise, return none
*/
int hobbyRCCommand() {
  int retval = RADIO_COMMAND_NONE;  // are we driving, idling, or entering command mode?

  // are any channels invalid?
  if (!hobbyRCAreAllChannelsValid()) {
    return RADIO_COMMAND_NONE;
  }

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

void musicMakerShieldInit() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  if (debug) Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1)
      ;  // don't do anything more
  }
  if (debug) Serial.println(F("SD card found"));

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  Serial.println(F("musicMakerShieldInit finished"));
}

void handleRCSignals() {
  if (0) Serial.println(F("handleRCSignals"));

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
        currentState = STATE_STARTING_NEW_CONVERSATION;
        if (debug) Serial.println("radio requested initiating conversation");
      }
      previousRadioCommand = currentRadioCommand;
      break;

    default:
      if (debug) Serial.println("unexpected return value from hobbyRCCommand()");
      previousRadioCommand = currentRadioCommand;
      break;
  }
}


//====================================================================================
// Ariks method
//



/*
	Conversation

	Notes:
		- before question
		- after question
*/


// is there anything else we need to do?
void resetConversation() {
  // clear face
  currentQuestionIndex = 0;
  currentState = STATE_ASK_QUESTION;
}


// run the conversation as a state machine
void myConverse() {

  switch (currentState) {

    case STATE_IDLE:
      break;

    case STATE_STARTING_NEW_CONVERSATION:
      resetConversation();
      break;

    case STATE_ASK_QUESTION:
      ask();  // this does face and speaking
      currentState = STATE_WAITING_SPEAKING_FINISHED;
      break;

    case STATE_WAITING_SPEAKING_FINISHED:
      if (finishedSpeaking()) {
        currentState = STATE_WAITING_FOR_RESPONSE;
      }
      break;

    case STATE_WAITING_FOR_RESPONSE:
      if (int currentButtonPressed = checkButtonStates()) {
        // Subtract 1 because
        // buttons count from 1
        // while the array of questions is indexed from 0
        int nextQuestionIndex = questions[currentQuestionIndex].buttons[currentButtonPressed - 1];
        if (debug) {
          Serial.print("currentButtonPressed: ");
          Serial.print(currentButtonPressed);
          Serial.print("\tnextQuestionIndex: ");
          Serial.println(nextQuestionIndex);
        }

        if (currentButtonPressed < 0 || currentButtonPressed >= FRONT_PANEL_BUTTON_COUNT) {
          Serial.print("Invalid question - must be a bug");
        }

        if (nextQuestionIndex < 0 || nextQuestionIndex >= NUMBER_OF_QUESTIONS) {
          Serial.print("Invalid question - must be a bug");
        } else {
          // proceed to next question
          currentQuestionIndex = nextQuestionIndex;
          // and go to the appropriate state
          currentState = STATE_ASK_QUESTION;
        }
      }
      break;

    default:
      if (debug) Serial.println("unexpected state in myConverse(); restarting conversation");
      currentState = STATE_STARTING_NEW_CONVERSATION;
      break;

  }  // end of state machine
}  // end of myConverse()

bool finishedSpeaking() {
  return (musicPlayer.stopped());
}

void ask() {
  if (debug) {
    Serial.print("\n in ask function, currentQuestionIndex = ");
    Serial.println(currentQuestionIndex);

    // express the appropriate face
    makeFace();

    // Speak the greeting
    if (debug) Serial.println(questions[currentQuestionIndex].audio_file);
    musicPlayer.startPlayingFile("GREETING.WAV");
    currentState = STATE_WAITING_SPEAKING_FINISHED;
  }
}

void makeFace() {

  if (debug) {
    Serial.print("makeFace: new face is = ");
    Serial.print(newFaceState);
    Serial.print("\tpresent = ");
    Serial.println(presentFaceState);
  }

  newFaceState = questions[currentQuestionIndex].face;
  if (newFaceState != presentFaceState) {
    robotFace.clear();
    robotFace.smile();
    presentFaceState = newFaceState;
    hobbyRCSetAllChannelsInvalid();
  }
}

void endConversation() {
  // clear face?
  // stop audio?
  //having_conversation = false;
}

// Return the number of the button pressed
// button numbering starts at 1
// so that we can return 0
// if no buttons are pressed
int checkButtonStates() {
  for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {

    // read the button
    int current_state = !digitalRead(front_panel_buttons[i]);


    if (0 && current_state) {
      Serial.print("button pressed at index = ");
      Serial.println(i);
    }

    // if button is not set, record the state
    if (!current_state) {
      front_panel_button_states[i] = 0;
      delay(1);  //set higher for better error handling
    }

    // edge detection: button was not pressed previously
    // but now it is pressed
    if (!front_panel_button_states[i] && current_state) {
      // Front panel button pushed
      front_panel_button_states[i] = 1;
      if (debug) {
        Serial.print("pressed switch number: ");
        Serial.println(i + 1);
      }

      // if a button is pressed, return right away
      // with it's number (not index)
      return i + 1;
    }
  }
  // if we've done all buttons and none are pressed
  return 0;
}