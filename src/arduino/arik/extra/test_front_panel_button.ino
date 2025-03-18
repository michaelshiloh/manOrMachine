/*
	Pin usage
*/
// Front panel buttons
const int FRONT_PANEL_BUTTON_COUNT = 4;
const int front_panel_buttons[FRONT_PANEL_BUTTON_COUNT] = {10, 11, 13, 12}; // Wrong {10, 11, 12, 13};
int front_panel_button_states[FRONT_PANEL_BUTTON_COUNT] = {0, 0, 0, 0};

int current_state = 0;

void setup() {
	Serial.begin(9600);
	// Setup front panel buttons
	setupFrontPanelButtons();
}

void loop() {
	// Check front panel buttons
	getFrontPanelButtons();
}


void setupFrontPanelButtons() {
	for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {
		pinMode(front_panel_buttons[i], INPUT_PULLUP);
		Serial.println(front_panel_buttons[i]);
	}
}

void getFrontPanelButtons() {
	for (int i = 0; i < FRONT_PANEL_BUTTON_COUNT; i++) {
		current_state = !digitalRead(front_panel_buttons[i]);
		if (!current_state) {
			front_panel_button_states[i] = 0;
			delay(1); //set higher for better error handling
		}
		if (!front_panel_button_states[i] && current_state) {
			// Front panel button pushed
			front_panel_button_states[i] = 1;
			Serial.print("pressed: ");
			Serial.println(i);
		}
	}
}
