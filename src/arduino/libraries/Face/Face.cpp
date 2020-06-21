
// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Adafruit_NeoPixel faceNeoPixels(NEOPIXEL_LED_COUNT, NEOPIXEL_LED_PIN, NEO_GRB + NEO_KHZ800);


/*


  52 53 54 55    56 57 58 59
  51 50 49 48    47 46 45 44
  36 37 38 39    40 41 42 43
  35 34 33 32    31 30 29 28




  27 26 25 24 23 22 21
  14 15 16 17 18 19 20
  13 12 11 10  9  8  7
   0  1  2  3  4  5  6
*/


#include "Arduino.h"
#include "Face.h"

Face::Face ( int pixelCount, int pixelPin) {
_pixelCount = pixelCount;
_pixelPin = pixelPin;
}

void Face::init() {

	pinMode(LED_BUILTIN, OUTPUT);

	faceNeoPixels = new Adafruit_NeoPixel (
										_pixelCount, 
										_pixelPin, 
										NEO_GRB + NEO_KHZ800);

	// Neopixel initialization
	faceNeoPixels->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	faceNeoPixels->show();            // Turn OFF all pixels ASAP
	faceNeoPixels->setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}


void Face::smile() {

	int smilePixels[] = {
	14, 12, 2, 3, 4, 8, 20,  // mouth 0-21
	51, 53, 54, 48, 36, 34, 33, // eyes
	39, 47, 57, 58, 44, 40, 30, 29, 43};
	uint32_t color = faceNeoPixels->Color(150,   100,   100);
	for (int i = 0; i < (sizeof(smilePixels) / sizeof(int)); i++) {
		faceNeoPixels->setPixelColor(smilePixels[i], color);
	}
	faceNeoPixels->show();
}

void Face::frown() {

	int frownPixels[] = {0, 12, 16, 17, 18, 8, 6,
	51, 53, 54, 48, 36, 34, 33, 39, 47, 57, 58, 44, 40, 30, 29, 43};
	uint32_t color = faceNeoPixels->Color(150,   100,   100);
	for (int i = 0; i < (sizeof(frownPixels) / sizeof(int)); i++) {
		faceNeoPixels->setPixelColor(frownPixels[i], color);
	}
	faceNeoPixels->show();
}

void Face::clear() {
	uint32_t color = faceNeoPixels->Color(0, 0, 0);
	for (int i = 0; i < _pixelCount; i++) {
		faceNeoPixels->setPixelColor(i, color);
	}
	faceNeoPixels->show();
	}

void Face::flash(int times) {

	while (times--) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(700);
		digitalWrite(LED_BUILTIN, LOW);
		delay(700);
	}
	delay(2000);
}
