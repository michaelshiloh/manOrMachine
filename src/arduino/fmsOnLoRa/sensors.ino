/*

	Maxbotix distance measuring sensor

	based on the simple web controlled robot from my telepresence project

  Modification log

	15 Feb 2020 - Michael Shiloh - written by Michael Shiloh
  20 Jun 2020 - MS - incorporated into integrated program
  12 Aug 2020 - MS - rewrote to use SAMD21 SERCOM


  TODO
  - see if pulseIn() would be more reliable (only 1 number)
*/




void sensorsInit() {

  sensorSerial.begin(9600);

  pinPeripheral(distanceSensorTxPin, PIO_SERCOM);
  pinPeripheral(distanceSensorRxPin, PIO_SERCOM);

  delay(250); // needs 250mSec before it's ready to receive anything
  delay(100); // first reading will take an additional ~100mS.
}

boolean stringComplete = false;
long delayBeforeNextReading = 49;
long lastReadingAt = 0;

/* do reading my way, one char at a time to minimize delays
  basic concept:
  - global array for incoming chars
  - global flag indiciating a valid reading is ready
  - when flag is cleared, time to read another

  If sensorDataValidFlag == true, return
  If sensorDataGotRFlag == false, get 'R'
  If sensorDataGotRFlag == true, get next byte
  If got 3 bytes, set sensorDataValidFlag == true and reset sensorDataGotRFlag
  -
*/

char sensorData[4] = {0, 0, 0, 0};  // null terminate
int sensorDataNextByte = 0;
boolean sensorDataValidFlag = false;
boolean sensorDataGotRFlag = false;

int sensorReadingTick() {
  char inChar;

  if (sensorDataValidFlag) {
    return (0);
  }

  if (!sensorDataGotRFlag) {
    if (debugPrint & debugSensorReadingTick) Serial.println("sensorReadingTick: waiting for R");

    if (!sensorSerial.available())
    {
      return (0);
    }

    inChar = (char)sensorSerial.read();
    if (inChar == 'R') {
      sensorDataGotRFlag = true;
    }
    return (0);
  } else { // we already have 'R'

    if (debugPrint & debugSensorReadingTick) Serial.print("sensorReadingTick: reading character ");
    if (debugPrint & debugSensorReadingTick) Serial.println(sensorDataNextByte);

    if (!sensorSerial.available())
    {
      return (0);
    }

    // Read the next character
    inChar = (char)sensorSerial.read();
    sensorData[sensorDataNextByte++] = inChar;

    // Do we have all three characters?
    if (sensorDataNextByte == 3) {
      sensorDataValidFlag == true;
      sensorDataGotRFlag = false;
      return atoi(sensorData);
    }
    return (0);
  }
}

void clearSensorDataValidFlag () {
  sensorDataValidFlag == false;
}
