int pin = 21;
unsigned long duration;

int min = 1500;
int max = 1500;

void setup() {
	Serial.begin(9600);
	pinMode(pin, INPUT);
}
// pin 5: trigger
// 974
// 2035

// pin 6: wheel
// 993
// 2036
void loop() {
	duration = pulseIn(pin, HIGH);
	// Serial.println(duration);

	//if (duration < min) {
	//	min = duration;
	//	Serial.println(duration);
	//}
	//if (duration > max) {
	//	max = duration;
	//	Serial.println(duration);
	//}

	Serial.println(duration);
}
