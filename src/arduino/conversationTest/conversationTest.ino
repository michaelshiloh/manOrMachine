// Front panel switches
const int BUTTON_ONE_PIN = 10;
const int BUTTON_TWO_PIN = 11;
const int BUTTON_THREE_PIN = 12;
const int BUTTON_FOUR_PIN = 13;



// Define states
//
//

const int STATE_IDLE = 0;
const int STATE_DRIVE = 99;
const int STATE_CONV_1 = 1;
const int STATE_CONV_2 = 2;
const int STATE_CONV_3 = 3;
const int STATE_CONV_4 = 4;

int currentState;

// Define radio commands
const int RADIO_COMMAND_NONE = 0;
const int RADIO_COMMAND_DRIVE = 1;
const int RADIO_COMMAND_INIT_CONV = 2;

void setup() {
  currentState = STATE_IDLE;

  // Read the hobby RC signals
  setupHobbyRC();

  // And finally the front panels switches
  pinMode(BUTTON_ONE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TWO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_THREE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_FOUR_PIN, INPUT_PULLUP);

}

void loop() {

  // Check if the radio is sending any command
  switch (getRadioCommand()) {

    case RADIO_COMMAND_NONE:
      // don't change state
      break;

    case RADIO_COMMAND_DRIVE:
      currentState = STATE_DRIVE break;

    case RADIO_COMMAND_INIT_CONV:
      currentState = STATE_CONV_1;
      break;

    default:
      // Indicate an error
      break;
  }

  switch (currentState) {

    case STATE_IDLE:
      // do nothing
      break;

    case STATE_DRIVE = 99;
      drive();
      break;

      case STATE_CONV_1:
      // express the greeting face
      newFaceState = FACE_STATE_GREETING;

      // Speak the greeting

      // proceed to next state
      currentState = STATE_CONV_2;
      break;

    case STATE_CONV_2:
      // we have spoken greeting, now check for an answer
      //
      // express the waiting face
      newFaceState = FACE_STATE_WAITING;

      // Speak the instructions

      // see if we have an answer
      if (!digitalRead(BUTTON_ONE_PIN)) {
        currentState = STATE_CONV_3;
      }

      break;

    default:
      // indicate an error
      break
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
