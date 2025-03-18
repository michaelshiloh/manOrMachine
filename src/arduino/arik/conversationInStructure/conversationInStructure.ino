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
const bool talk = true;


/*
	Pin usage
*/
// Front panel buttons
const int FRONT_PANEL_BUTTON_COUNT = 4;
const int front_panel_buttons[FRONT_PANEL_BUTTON_COUNT] = {10, 11, 13, 12}; // Wrong {10, 11, 12, 13};
int front_panel_button_states[FRONT_PANEL_BUTTON_COUNT] = {0, 0, 0, 0};
int current_front_panel_button = 0;
int silver_button_pushed = 0;

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


struct Question {
	String audio_file; // sound to play
	int buttons[FRONT_PANEL_BUTTON_COUNT]; // array of buttons that point to questions
	int face; // face to make
//	int button_0;
//	int button_1;
//	int button_2;
//	int button_3;
};


//TODO
// Control verbosity of messages


// Arrays for storing hobby RC values
//uint16_t rc_values[RC_NUM_CHANNELS] = {1500, 1500, 1000, 1000, 1000};
uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

/*
	global objects

	TODO seems to need to be under the array declarations above...
*/
Face robotFace(neoPixelFaceCount, FACE_NEOPIXEL_PIN);


/*
	Create question objects
*/
// button 1: yes
// button 2: no
Question q1 = {"Hi. Wanna talk?", {2, 3, 0, 0}, 2};
Question q2 = {"Sweet. Do you like rice?", {4, 5, 0, 0}, 3};
Question q3 = {"Goodbye.", {0, 0, 0, 0}, 1};
Question q4 = {"Me too! Wanna talk again?", {2, 3, 0, 0}, 3};
Question q5 = {"Never talk to me again!.. Wanna talk again?", {2, 3, 0, 0}, 3};
//Question q2;
//Question q3;
//Question q4;
//Question q5;
//Question q6;
//Question q7;
//...

//q1.audio_file = "Hi. Wanna talk?";
//q1.buttons = [2, 3, 0, 0];
//q1.face = 2;
////q1.button_0 = 2; // Goodbye [end]
////q1.button_1 = 3; // Fave animal
////q1.button_2 = 0;
////q1.button_3 = 0;

//q2.audio_file = "Goodbye";
//q2.buttons = [0, 0, 0, 0];
//q2.face = 1;
////q2.button_0 = 0;
////q2.button_1 = 0;
////q2.button_2 = 0;
////q2.button_3 = 0;

//q3.audio_file = "Fave animal, cat, dog, avocado, other?";
//q3.button = [4, 5, 6, 7]
//q3.face = 3;
////q3.button_0 = 4; // Cat
////q3.button_1 = 5; // Dog
////q3.button_2 = 6; // Avocado
////q3.button_3 = 7; // Other

//q4.audio_file = "Cats rule! Wanna talk again?";
//q4.buttons = [1, 2, 0, 0];
//q4.face = 3;
////q4.button_0 = 1; // Q1
////q4.button_1 = 2; // Goodbye [end]
////q4.button_2 = 0;
////q4.button_3 = 0;

//q5.audio_file = "Dogs drool! Wanna talk again?";
//q5.buttons = [1, 2, 0, 0];
//q5.face = 3;
////q5.button_0 = 1; // Q1
////q5.button_1 = 2; // Goodbye [end]
////q5.button_2 = 0;
////q5.button_3 = 0;

//q6.audio_file = "Avocados are green! Wanna talk again?";
//q6.buttons = [1, 2, 0, 0];
//q6.face = 3;
////q6.button_0 = 1; // Q1
////q6.button_1 = 2; // Goodbye [end]
////q6.button_2 = 0;
////q6.button_3 = 0;

//q7.audio_file = "That's not an animal! Wanna talk again?";
//q7.buttons = [1, 2, 0, 0];
//q7.face = 3;
////q7.button_0 = 1; // Q1
////q7.button_1 = 2; // Goodbye [end]
////q7.button_2 = 0;
////q7.button_3 = 0;

// Misc.
bool having_conversation = false;
bool starting_new_conversation = false;
bool person_responded = false;
int next_question_index = 0;
int RC_CH4_THRESHOLD = 1500;
Question questions[] = {q1, q2, q3, q4, q5};
Question current_question = q1;


void setup() {
	// Begin serial transmission if in debug mode
	if (debug || talk) {
		Serial.begin(9600);
	}

	// Neopixels for face
	robotFace.init();

	// Read the hobby RC signals
	setupHobbyRC();

	// Wait for values to be copied
	delay(100);

	// Setup front panel buttons
	setupFrontPanelButtons();
}

void loop() {
	// Get signals
	hobbyRCCommand();

	handleRCSignals();

	// Check front panel buttons
	getFrontPanelButtons();

	// Check front panel buttons
	getSilverButton();

	// Update states?

	// Drive
	//drive();

	// Converse
	converse();
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
	Setup front panel buttons
*/
void setupFrontPanelButtons() {
	for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {
		pinMode(front_panel_buttons[i], INPUT_PULLUP);
		if (debug) {
			Serial.println(front_panel_buttons[i]);
		}
	}
}

void getFrontPanelButtons() {
	for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {
		int current_state = !digitalRead(front_panel_buttons[i]);
		if (!current_state) {
			front_panel_button_states[i] = 0;
			delay(1); //set higher for better error handling
		}
		if (!front_panel_button_states[i] && current_state) {
			// Front panel button pushed
			person_responded = true;
			current_front_panel_button = i;
			front_panel_button_states[i] = 1;
			if (debug) {
				Serial.print("pressed: ");
				Serial.println(i);
			}
		}
	}
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
	//if (silverButtonPushed() && !having_conversation) {
}

void getSilverButton() {
	int current_state = rc_values[RC_CH4] > RC_CH4_THRESHOLD;
	if (!current_state) {
		silver_button_pushed = 0;
		//delay(1); //set higher for better error handling
	}
	if (!silver_button_pushed && current_state) {
		starting_new_conversation = true;
		silver_button_pushed = 1;
		//Serial.print("pressed: silver button");
	}
	//return rc_values[RC_CH4] > RC_CH4_THRESHOLD;
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
		startNewConversation();
	} else if (having_conversation && person_responded) {
	//} else if (having_conversation) {
		// Subtract 1 because we start counting questions at 1 not 0 like the buttons
		next_question_index = current_question.buttons[current_front_panel_button] - 1;
		Serial.print("next_question_index: ");
		Serial.println(next_question_index);
		if (next_question_index == -1) {
			person_responded = false;
			//endConversation();
		} else if (next_question_index == 0) {
			startNewConversation();
		} else {
			current_question = questions[next_question_index];
			ask();
		}
	}
}

void startNewConversation() {
	current_question = q1;
	having_conversation = true;
	starting_new_conversation = false;
	ask();
}

void ask() {
	// Face
	makeFace();

	// Speak
	//play(question.audio_file);
	if (talk) {
		Serial.println(current_question.audio_file);
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
		Serial.print("\tpresent = " );
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
}

void endConversation() {
	// clear face?
	// stop audio?
	having_conversation = false;
}
