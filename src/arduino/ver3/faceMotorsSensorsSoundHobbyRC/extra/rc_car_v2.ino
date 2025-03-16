/******************************************

RC Car
Version 2.0

By: Arik Rosenthal
Date: 2025-02-14

******************************************/

#include <Servo.h>


/* Debug */
const bool debug = false;


/* Steering wheel IN */
// 1460 to 1507
const uint16_t WHEEL_IDLE = 1484; //1490; //1504; // Idle wheel value, give or take about 4

const uint16_t WHEEL_DEADZONE = 30; //20; // Deadzone on either side of idle

const uint16_t WHEEL_PULL_START = WHEEL_IDLE - WHEEL_DEADZONE; // Start wheel pull input
const uint16_t WHEEL_PUSH_START = WHEEL_IDLE + WHEEL_DEADZONE; // Start wheel push input

const uint16_t WHEEL_PULL_STOP = 993; // Lowest reading from wheel
const uint16_t WHEEL_PUSH_STOP = 2036; // Highest reading from wheel


/* Motor OUT */
const uint8_t MOTOR_MAX = 255; // Fastest I can run the motor
const uint8_t MOTOR_MIN = 0; // Slowest I want to run the motor
const uint8_t MOTOR_IDL = 0; // No speed

/* Trigger IN */
// 1477 to 1523
const uint16_t TRIGGER_IDLE = 1500; //1514; // Idle trigger value, give or take about 4
// Note: this is is different from the midpoint of PULL and PUSH which
// is around 1549, and this means we have 540 units of precision when
// we pull the trigger and 522 when pushing

const uint16_t TRIGGER_DEADZONE = 30;//20; // Deadzone on either side of idle

const uint16_t TRIGGER_PULL_START = TRIGGER_IDLE - TRIGGER_DEADZONE; // Start trigger pull input
const uint16_t TRIGGER_PUSH_START = TRIGGER_IDLE + TRIGGER_DEADZONE; // Start trigger push input

const uint16_t TRIGGER_PULL_STOP = 974; // lowest reading from trigger
const uint16_t TRIGGER_PUSH_STOP = 2015; //2036; // highest reading from trigger


/* Pins */
  //IN//
const uint8_t PIN_IN_HIGH_BUTTON = 3;
const uint8_t PIN_IN_TRIGGER = 5;
const uint8_t PIN_IN_WHEEL = 6;

  //OUT//
const uint8_t PIN_OUT_MOTOR_5V_1 = 8; // IN1
const uint8_t PIN_OUT_MOTOR_5V_2 = 7; // IN2
const uint8_t PIN_OUT_MOTOR_SPEED = 11; // EN2
const uint8_t PIN_OUT_SERVO_1 = 9; // EN1
const uint8_t PIN_OUT_SERVO_2 = 12; // IN4
// !! Servo library disables PWM on pins 9 and 10 !!
// https://forum.arduino.cc/t/problem-with-servo-attach/195767/3


/* Variables */
uint16_t input_high_button = 1000;
uint16_t input_wheel = WHEEL_IDLE;
uint16_t input_trigger = TRIGGER_IDLE;
uint16_t output_motor = 0;
uint16_t last_speed = 0;
bool strafe = false;


/* SteeringServo Class */
class SteeringServo : public Servo {
public:
	uint16_t CENTER; // or lowercase center?
	uint16_t SWING_POSITIVE;
	uint16_t SWING_NEGATIVE;

	//SteeringServo();

	void swingPositive(uint16_t swing) {
		SWING_POSITIVE = CENTER + swing;
	}

	void swingNegative(uint16_t swing) {
		SWING_NEGATIVE = CENTER - swing;
	}

	void center() {
		this->writeMicroseconds(CENTER);
	}

	void turn(uint16_t input_wheel) {
		uint16_t output_servo = CENTER;
		if (input_wheel > WHEEL_PUSH_START) {
			output_servo = map(
				input_wheel,
				WHEEL_PUSH_START,
				WHEEL_PUSH_STOP,
				CENTER,
				SWING_POSITIVE
			);
			this->writeMicroseconds(min(output_servo, SWING_POSITIVE));
			if (debug) {
				Serial.print("Positive: ");
				Serial.println(output_servo);
			}
		} else if (input_wheel < WHEEL_PULL_START) {
			output_servo = map(
				input_wheel,
				WHEEL_PULL_START,
				WHEEL_PULL_STOP,
				CENTER,
				SWING_NEGATIVE
			);
			this->writeMicroseconds(max(output_servo, SWING_NEGATIVE));
			if (debug) {
				Serial.print("Negative: ");
				Serial.println(output_servo);
			}
		} else {
			this->writeMicroseconds(CENTER);
			if (debug) {
				Serial.print("Center: ");
				Serial.println(CENTER);
			}
		}
	}

	void mirror(uint16_t input_wheel) {
		uint16_t output_servo = CENTER;
		if (input_wheel > WHEEL_PUSH_START) {
			output_servo = map(
				input_wheel,
				WHEEL_PUSH_START,
				WHEEL_PUSH_STOP,
				CENTER,
				SWING_NEGATIVE
			);
			this->writeMicroseconds(max(output_servo, SWING_NEGATIVE));
			if (debug) {
				Serial.print("Negative: ");
				Serial.println(output_servo);
			}
		} else if (input_wheel < WHEEL_PULL_START) {
			output_servo = map(
				input_wheel,
				WHEEL_PULL_START,
				WHEEL_PULL_STOP,
				CENTER,
				SWING_POSITIVE
			);
			this->writeMicroseconds(min(output_servo, SWING_POSITIVE));
			if (debug) {
				Serial.print("Positive: ");
				Serial.println(output_servo);
			}
		} else {
			this->writeMicroseconds(CENTER);
			if (debug) {
				Serial.print("Center: ");
				Serial.println(CENTER);
			}
		}
	}
};

/* Servo OUT */
SteeringServo servo_1;
SteeringServo servo_2;


void setup() {
	if (debug) {
		Serial.begin(9600);
	}

	servo_1.CENTER = 1460;
	servo_1.swingPositive(300);
	servo_1.swingNegative(350);

	servo_2.CENTER = 1480;
	servo_2.swingPositive(300);
	servo_2.swingNegative(310);

	servo_1.center();
	servo_2.center();
	servo_1.attach(PIN_OUT_SERVO_1);
	servo_2.attach(PIN_OUT_SERVO_2);

	pinMode(PIN_OUT_MOTOR_5V_1, OUTPUT);
	pinMode(PIN_OUT_MOTOR_5V_2, OUTPUT);
}


void loop() {
	input_wheel = pulseIn(PIN_IN_WHEEL, HIGH);
	input_trigger = pulseIn(PIN_IN_TRIGGER, HIGH);
	input_high_button = pulseIn(PIN_IN_HIGH_BUTTON, HIGH);

	// Michael suggests testing analog write speed
	// to motor FIRST, then HIGH/LOW

	/* Trigger / Motor */
	if (input_trigger < TRIGGER_PULL_START) {
		/* Forward */
		digitalWrite(PIN_OUT_MOTOR_5V_1, HIGH);
		digitalWrite(PIN_OUT_MOTOR_5V_2, LOW);
		output_motor = map(
			input_trigger,
			TRIGGER_PULL_START,
			TRIGGER_PULL_STOP,
			MOTOR_MIN,
			MOTOR_MAX
		);
		last_speed = output_motor;
		analogWrite(PIN_OUT_MOTOR_SPEED, min(output_motor, MOTOR_MAX));
		if (debug) {
			Serial.print("(FWD)Speed: ");
			Serial.println(output_motor);
		}
	} else if (input_trigger > TRIGGER_PUSH_START) {
		/* Reverse */
		digitalWrite(PIN_OUT_MOTOR_5V_1, LOW);
		digitalWrite(PIN_OUT_MOTOR_5V_2, HIGH);
		output_motor = map(
			input_trigger,
			TRIGGER_PUSH_START,
			TRIGGER_PUSH_STOP,
			MOTOR_MIN,
			MOTOR_MAX
		);
		last_speed = output_motor;
		analogWrite(PIN_OUT_MOTOR_SPEED, min(output_motor, MOTOR_MAX));
		if (debug) {
			Serial.print("(REV)Speed: ");
			Serial.println(output_motor);
		}
	} else {
		/* Stop */
		if (last_speed > 10) { // change to 5?
			/* Decelerate */
			last_speed = last_speed / 1.5; // change to 1.3
		} else {
			/* Idle */
			last_speed = 0;
		}
		analogWrite(PIN_OUT_MOTOR_SPEED, last_speed);
		if (debug) {
			Serial.println("(IDL)Speed: 0");
		}
	}


	/* Wheel / Servo */

	if (input_high_button < 1500) {
		strafe = false;
	} else {
		strafe = true;
	}

	servo_1.turn(input_wheel);

	if (!strafe) {
		servo_2.mirror(input_wheel);
	} else {
		servo_2.turn(input_wheel);
	}
}
