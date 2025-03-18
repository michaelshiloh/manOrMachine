/*
	Pin usage
*/


//TODO
// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
//#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
//Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);


//TODO
// Control verbosity of messages
const int verboseMotor = 4;
const int debugMotor = 8;
const int verboseMOTORTIMEOUT = 16;
const int debugSensorReadingTick = 32;
const int debugPrint = 0;

// for controlling the motors
const int MYFORWARD = 0;
const int MYREVERSE = 1;
const int MOTORTIMEOUT = 100;


/*
	global objects
*/
//TODO
SabertoothMotorControllerSerial3 myMotorController(MOTORTIMEOUT);

void setup() {
	// Sensors need a trigger to start sending data
	pinMode(SENSOR_TRIGGER_PIN, OUTPUT);
	digitalWrite(SENSOR_TRIGGER_PIN, LOW);

	myMotorController.init();

	// So the robot can speak
	//setupMusicMakerShield();

}

void loop() {
	// Normally, just wander aimlessly avoiding obstacles
	//avoidObstacles();

	//TODO
	// Keep the motors running
	// this must be called regularly so never use delay()
	myMotorController.tick();
}

bool hobbyRCCommand() {
	// Channel 1 is the steering wheel
	if (rc_values[RC_CH1] > 1600) {
		if (debug) {
			Serial.print("right!");
		}
		myMotorController.right(180);
	} else if (rc_values[RC_CH1] < 1400) {
		if (debug) {
			Serial.print("left!");
		}
		myMotorController.left(180);
	}

	// Channel 2 is the throttle
	if (rc_values[RC_CH2] < 1400) {
		if (debug) {
			Serial.print("forward!");
		}
		int go = map(constrain( rc_values[RC_CH2], 900, 1400), 1400, 900, 0, 255);
		myMotorController.forward(go);
	} else if (rc_values[RC_CH2] > 1600) {
		if (debug) {
			Serial.print("backward!");
		}
		int go = map(constrain( rc_values[RC_CH2], 1600, 2000), 1600, 2000, 0, 255);
		myMotorController.backward(go);
	}
}

void updateMotors(char inChar) {
	/*
		1 = pin 13 LED on
		0 = pin 13 LED off
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
			if (debug) {
				Serial.println("LED on");
			}
			digitalWrite(LED_BUILTIN, HIGH);
			break;
		case '0':
			if (debug) {
				Serial.println("LED off");
			}
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
			if (debug) {
				Serial.println("invalid message");
			}
	}
}

void setupMusicMakerShield() {
	//Commented out for now until I fix the interrupt vector redefinition error

	//	if (!musicPlayer.begin()) {  // initialise the music player
	//		Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
	//		while (1)
	//			;
	//	}
	if (debug) {
		Serial.println(F("VS1053 found"));
	}

	if (!SD.begin(CARDCS)) {
		if (debug) {
			Serial.println(F("SD card failed or not present"));
		}
		while (1)
			;  // don't do anything more
	}

	// Set volume for left, right channels. lower numbers == louder volume!
	//  musicPlayer.setVolume(20, 20);

	// Timer interrupts are not suggested, better to use DREQ interrupt!
	//musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

	// If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
	// audio playing
	// musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}
