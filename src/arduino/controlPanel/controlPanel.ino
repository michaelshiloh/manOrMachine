/*
  Robot control panel
  Based on the built-in LCD example Hello World

  Todo:
  - add a software serial port to communicate with the Arduino on the robot

*/

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

/*
   Pin assignments
*/

// unused: 8, 9, 10, 11, 12, 13, A2
const int controlPanelTxPin = 6;
const int controlPanelRxPin = 7;
const int rs = A1, en = A0, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// One or both of 11, 12 don't work
// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int leftButtonPin = A3;
const int rightButtonPin = A4;
const int forwardButtonPin = A5;

SoftwareSerial controlPanelSerial(controlPanelRxPin, controlPanelTxPin); // RX, TX

void controlPanelInit() {
  controlPanelSerial.begin(9600);
}

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2); // LCD size
  lcd.print("hello, world!");

  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(forwardButtonPin, INPUT_PULLUP);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

  lcd.setCursor (14, 1);
  lcd.print(digitalRead(leftButtonPin));
  lcd.setCursor (12, 1);
  lcd.print(digitalRead(rightButtonPin));
  lcd.setCursor (10, 1);
  lcd.print(digitalRead(forwardButtonPin));
}
