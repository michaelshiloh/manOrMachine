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
const int controlPanelTxPin = 6; // red wire
const int controlPanelRxPin = 7; // yellow wire
const int rs = A1, en = A0, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// One or both of 11, 12 don't work
// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int leftButtonPin = A3;
const int rightButtonPin = A4;
const int forwardButtonPin = A5;

SoftwareSerial controlPanelSerial(controlPanelRxPin, controlPanelTxPin); // RX, TX



LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  lcd.begin(16, 2); // LCD size
  lcd.print("hello, world!");

  controlPanelSerial.begin(9600);
  Serial.begin(9600);

  Serial.begin(9600);
  Serial.print("hello, world!");

  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(forwardButtonPin, INPUT_PULLUP);
}

//boolean running = false;


void loop() {


  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

  int leftState = !digitalRead(leftButtonPin);
  int rightState = !digitalRead(rightButtonPin);
  int forwardState = !digitalRead(forwardButtonPin);

  lcd.setCursor (14, 1);
  lcd.print(leftState);
  lcd.setCursor (12, 1);
  lcd.print(rightState);
  lcd.setCursor (10, 1);
  lcd.print(forwardState);

  if (leftState || rightState || forwardState) {
   // running = true;

    char message = (leftState << 0) |
                   (rightState << 1) |
                   (forwardState << 2);
    Serial.print("sending message ]");
    Serial.print((int)message);
    Serial.println("[");


    int foo = controlPanelSerial.write(message);
    Serial.print("controlPanelSerial: ");
    Serial.print(foo);
    Serial.println(" bytes sent");
  } else {

    // get rid of this and replace with a timeout on the controller
//    if (running == true) {
//      Serial.println("stop");
//      for (int i = 5; i; i--) {
//        int foo = controlPanelSerial.write((char)0);
//      }
//      running = false;
//    }
  }
}
