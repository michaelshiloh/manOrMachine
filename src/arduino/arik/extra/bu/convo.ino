/*
	Convo [WIP]
	Arik Rosenthal
	2025-03-17

	Notes:
		? silver_button = start new conversation
		? clear_button = end conversation
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
	Debug
	- Set to `true` for Serial output
*/
const bool debug = false;


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
const int SENSOR_TRIGGER_PIN = 7;
// Pins 14 and 15 will be used for Serial port 3
const int RC_CH1_PIN = 21; // Steering (was set to pin 3) [min=980 max=2036]
const int RC_CH2_PIN = 2;  // Throttle [min=975 max=2036]
const int RC_CH3_PIN = 18; // Small knob (VR) [min=902 max=1900]
const int RC_CH4_PIN = 19; // Silver circular button (SWA) [min=976 max=2036]
const int RC_CH5_PIN = 20; // Long clear button (SWD) [min=976 max=2036]

//TODO
// Adafruit music maker shield


/*
	Other global variables
*/
const int RC_NUM_CHANNELS = 5;    // How many radio channels
const int neoPixelFaceCount = 60; // How many NeoPixels

// Indexes into the array of hobby RC radio channels
#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3
#define RC_CH5  4

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

// Misc.
bool having_conversation = false
bool starting_new_conversation = false
bool person_responded = false
int red_button_index = 0;
int current_question = 0;
int next_question_index = 0;
int RC_CH4_THRESHOLD = 1500;

//array red_buttons = [BUTTON_ONE_PIN, BUTTON_TWO_PIN, BUTTON_THREE_PIN, BUTTON_FOUR_PIN]
//array red_buttons = [0, 1, 2, 3]
array red_buttons = 4;

class Question {
Public:
	string audio_file;
	array buttons;
	int face;
//	int button_0;
//	int button_1;
//	int button_2;
//	int button_3;
}


//TODO
// Control verbosity of messages


/*
	global objects
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);

// Arrays for storing hobby RC values
uint16_t rc_values[RC_NUM_CHANNELS] = {1500, 1500, 1000, 1000, 1000};
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];


/*
	Create question objects
*/
Question q1;
Question q2;
Question q3;
Question q4;
Question q5;
Question q6;
Question q7;
//...

q1.audio_file = "Hi. Wanna talk?";
q1.buttons = [2, 3, 0, 0];
q1.face = 2;
//q1.button_0 = 2; // Goodbye [end]
//q1.button_1 = 3; // Fave animal
//q1.button_2 = 0;
//q1.button_3 = 0;

q2.audio_file = "Goodbye";
q2.buttons = [0, 0, 0, 0];
q2.face = 1;
//q2.button_0 = 0;
//q2.button_1 = 0;
//q2.button_2 = 0;
//q2.button_3 = 0;

q3.audio_file = "Fave animal, cat, dog, avocado, other?";
q3.button = [4, 5, 6, 7]
q3.face = 3;
//q3.button_0 = 4; // Cat
//q3.button_1 = 5; // Dog
//q3.button_2 = 6; // Avocado
//q3.button_3 = 7; // Other

q4.audio_file = "Cats rule! Wanna talk again?";
q4.buttons = [1, 2, 0, 0];
q4.face = 3;
//q4.button_0 = 1; // Q1
//q4.button_1 = 2; // Goodbye [end]
//q4.button_2 = 0;
//q4.button_3 = 0;

q5.audio_file = "Dogs drool! Wanna talk again?";
q5.buttons = [1, 2, 0, 0];
q5.face = 3;
//q5.button_0 = 1; // Q1
//q5.button_1 = 2; // Goodbye [end]
//q5.button_2 = 0;
//q5.button_3 = 0;

q6.audio_file = "Avocados are green! Wanna talk again?";
q6.buttons = [1, 2, 0, 0];
q6.face = 3;
//q6.button_0 = 1; // Q1
//q6.button_1 = 2; // Goodbye [end]
//q6.button_2 = 0;
//q6.button_3 = 0;

q7.audio_file = "That's not an animal! Wanna talk again?";
q7.buttons = [1, 2, 0, 0];
q7.face = 3;
//q7.button_0 = 1; // Q1
//q7.button_1 = 2; // Goodbye [end]
//q7.button_2 = 0;
//q7.button_3 = 0;

array questions = [q1, q2, q3, q4, q5, q6, q7];

void setup() {
	// Begin serial transmission if in debug mode
	if (debug) {
		Serial.begin(9600);
	}

	// Neopixels for face
	robotFace.init();

	// Read the hobby RC signals
	setupHobbyRC();

	// Read the red button (front panels switches) signals
	setupRedButtons();
}

void loop() {
	// Get signals
	hobbyRCCommand()

	handleRCSignals();
	handleRedButtons();

	// Update states?

	// Drive
	//drive();

	// Converse
	converse();

	// Top priority is given to the hobby RC commands:
}


/*
	Hobby RC functions
*/
void RCReadValues() {
	noInterrupts();
	memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
	interrupts();
}

void calcInput(uint8_t channel, uint8_t input_pin) {
	if (digitalRead(input_pin) == HIGH) {
		rc_start[channel] = micros();
	} else {
		uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
		rc_shared[channel] = rc_compare;
	}
}

void calcCH1() {
	calcInput(RC_CH1, RC_CH1_PIN);
}
void calcCH2() {
	calcInput(RC_CH2, RC_CH2_PIN);
}
void calcCH3() {
	calcInput(RC_CH3, RC_CH3_PIN);
}
void calcCH4() {
	calcInput(RC_CH4, RC_CH4_PIN);
}
void calcCH5() {
	calcInput(RC_CH5, RC_CH5_PIN);
}

void setupHobbyRC() {
	pinMode(RC_CH1_PIN, INPUT);
	pinMode(RC_CH2_PIN, INPUT);
	pinMode(RC_CH3_PIN, INPUT);
	pinMode(RC_CH4_PIN, INPUT);
	pinMode(RC_CH5_PIN, INPUT);

	//	enableInterrupt(RC_CH1_PIN, calcCH1, CHANGE);
	//	enableInterrupt(RC_CH2_PIN, calcCH2, CHANGE);
	//	enableInterrupt(RC_CH3_PIN, calcCH3, CHANGE);
	//	enableInterrupt(RC_CH4_PIN, calcCH4, CHANGE);
	//	enableInterrupt(RC_CH5_PIN, calcCH5, CHANGE);

	attachInterrupt(digitalPinToInterrupt(RC_CH5_PIN), calcCH5, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RC_CH4_PIN), calcCH4, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RC_CH3_PIN), calcCH3, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RC_CH2_PIN), calcCH2, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RC_CH1_PIN), calcCH1, CHANGE);
}


/*
	Setup red buttons
*/
void setupRedButtons() {
	pinMode(BUTTON_ONE_PIN, INPUT_PULLUP);
	pinMode(BUTTON_TWO_PIN, INPUT_PULLUP);
	pinMode(BUTTON_THREE_PIN, INPUT_PULLUP);
	pinMode(BUTTON_FOUR_PIN, INPUT_PULLUP);
}


/*
	Run hobby RC command
*/
bool hobbyRCCommand() {
	RCReadValues();

	if (debug) {
		// Right now just print them out
		Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t\t");
		Serial.print("CH2:"); Serial.print(rc_values[RC_CH2]); Serial.print("\t\t");
		Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t\t");
		Serial.print("CH4:"); Serial.print(rc_values[RC_CH4]); Serial.print("\t\t");
		Serial.print("CH5:"); Serial.print(rc_values[RC_CH5]); Serial.println("\t\t");
	}
	return (false); // later will override if needed
}


// misc.
void handleRCSignals() {
	if (silverButtonPushed() && !having_conversation) {
		starting_new_conversation = true;
	}
}

int silverButtonPushed() {
	return rc_values[RC_CH4] > RC_CH4_THRESHOLD;
}

void handleRedButtons() {
	if (redButtonPushed()) {
		person_responded = true;
	}
}

int redButtonPushed() {
	// Check this logic
	for (i = 0; i < red_buttons; i++) {
		if (!digitalRead(i) {
			red_button_index = i;
			return 1;
		}
	}
	return 0;
}

void drive() {
	return;
}


/*
	Conversation

	Notes:
		- before question
		- after question
*/
void converse() {
	if (starting_new_conversation) {
		current_question = q1;
		having_conversation = true;
		starting_new_conversation = false;
		ask();
	}

	if (having_conversation) {
		if (person_responded) {
			next_question_index = current_question.buttons[red_button_index];
			current_question = questions[next_question_index];
			ask();
		}
	}

	return;
}

void ask() {
	// Face
	makeFace();

	// Speak
	//play(question.audio_file);
	if (debug) {
		Serial.println(question.audio_file);
	}

	// Set buttons??
	//question.button_0
	//question.button_1
	//question.button_2
	//question.button_3

	person_responded = false;
}

void makeFace() {
	//current_question.face(); // could be method in class

	if (debug) {
		Serial.print("new = " );
		Serial.print(newFaceState );
		Serial.print("present = " );
		Serial.println(presentFaceState );
	}

	//TODO verify face logic
	newFaceState = current_question.face;
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
				if (debug) {
					Serial.println("Invalid face state");
				}
		}
		presentFaceState = newFaceState;
	}

void endConversation() {
	// clear face?
	// stop audio?
	having_conversation = false;
}
