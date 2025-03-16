
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

void Face::flagColours() {

	int leftEye[] = {
  52, 53, 54, 55,    
  51, 50, 49, 48,   
  36, 37, 38, 39,  
  35, 34, 33, 32 
	};
	int rightEye[] = {
  56, 57, 58, 59,
  47, 46, 45, 44,
  40, 41, 42, 43,
  31, 30, 29, 28
	};
	int mouth[] = {
  27, 26, 25, 24, 23, 22, 21,
  14, 15, 16, 17, 18, 19, 20,
  13, 12, 11, 10,  9,  8,  7,
   0,  1,  2,  3,  4,  5,  6
	};
	paint(
		faceNeoPixels->Color(255, 0, 0),
		leftEye, 
		sizeof(leftEye)/sizeof(int));
	paint(
		faceNeoPixels->Color(0, 255, 0),
		rightEye, 
		sizeof(rightEye)/sizeof(int));
	paint(
		faceNeoPixels->Color(255, 255, 255),
		mouth, 
		sizeof(mouth)/sizeof(int));
	faceNeoPixels->show();
}

void Face::flag() {

	int leftEye[] = {
  52, 53, 54, 55,    
  51, 50, 49, 48,   
  36, 37, 38, 39,  
  35, 34, 33, 32 
	};
	int rightEye[] = {
  56, 57, 58, 59,
  47, 46, 45, 44,
  40, 41, 42, 43,
  31, 30, 29, 28
	};
	int mouthLeft[] = {
  27, 26, 
  14, 15,
  13, 12, 
   0,  1
	};
	int mouthCenter[] = {
  25, 24, 23, 
  16, 17, 18,
  11, 10,  9,
  2,  3,  4
	};
	int mouthRight[] = {
   22, 21,
   19, 20,
    8,  7,
    5,  6
	};
	paint(
		faceNeoPixels->Color(255, 0, 0),
		leftEye, 
		sizeof(leftEye)/sizeof(int));
	paint(
		faceNeoPixels->Color(255, 0, 0),
		rightEye, 
		sizeof(rightEye)/sizeof(int));
	paint(
		faceNeoPixels->Color(0, 255, 0),
		mouthLeft, 
		sizeof(mouthLeft)/sizeof(int));
	paint(
		faceNeoPixels->Color(255, 255, 255),
		mouthCenter, 
		sizeof(mouthCenter)/sizeof(int));
	paint(
		faceNeoPixels->Color(0, 0, 0),
		mouthRight, 
		sizeof(mouthRight)/sizeof(int));
	faceNeoPixels->show();
}

void Face::surprised() {

	int pixels[] = {
		2, 3, 4, 12, 8, 19, 15, 23, 24, 25,
		28, 29, 30, 31, 32, 33, 34, 35, 36,
		39, 40, 43, 44, 47, 48, 
		51, 52, 53, 54, 55, 56, 57, 58, 59
	};
	paint(
		faceNeoPixels->Color(191, 232, 79),
		pixels, 
		sizeof(pixels)/sizeof(int));
	faceNeoPixels->show();
}


void Face::angry() {

	int pixels[] = {
	14, 15, 16, 17, 18, 19, 20,
	7, 8, 9, 10, 11, 12, 13,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
	};
	paint(
		faceNeoPixels->Color(250,   0,   0),
		pixels, 
		sizeof(pixels)/sizeof(int));
	faceNeoPixels->show();
}

void Face::smile() {

	int pixels[] = {
	14, 12, 2, 3, 4, 8, 20,  // mouth 0-21
	51, 53, 54, 48, 36, 34, 33, // eyes
	39, 47, 57, 58, 44, 40, 30, 29, 43};
	paint(
		faceNeoPixels->Color(150,   100,   100),
		pixels, 
		sizeof(pixels)/sizeof(int));
	faceNeoPixels->show();
}

void Face::frown() {

	int pixels[] = {0, 12, 16, 17, 18, 8, 6,
	51, 53, 54, 48, 36, 34, 33, 39, 47, 57, 58, 44, 40, 30, 29, 43};
	paint(
		faceNeoPixels->Color(150,   100,   100),
		pixels, 
		sizeof(pixels)/sizeof(int));
	faceNeoPixels->show();
}

void Face::eyesLeft() {
	int iris[] = {
		49, 48,
		38, 39,
		45, 44,
		42, 43
	};
	int sclera[] = {
		56, 57, 58, 59,
		47, 46,
		40, 41,
		31, 30, 29, 28,
		52, 53, 54, 55,
		51, 50,
		36, 37,
		35, 34, 33, 32
	};
	int mouth[] = {
		23, 22, 21,
		14, 15, 16, 17, 18, 19, 20,
		13, 12, 11, 10, 9, 8, 7,
		4, 5, 6
	};

	paint(
		faceNeoPixels->Color(9, 65, 230),
		iris, 
		sizeof(iris)/sizeof(int));

	paint(
		faceNeoPixels->Color(255, 255, 255),
		sclera, 
		sizeof(sclera)/sizeof(int));

	paint(
		faceNeoPixels->Color(230, 40, 90),
		mouth, 
		sizeof(mouth)/sizeof(int));

	faceNeoPixels->show();
}

void Face::eyesRight() {
	int iris[] = {
		51, 50,
		36, 37,
		47, 46,
		40, 41
	};
	int sclera[] = {
		56, 57, 58, 59,
		45, 44,
		42, 43,
		31, 30, 29, 28,
		52, 53, 54, 55,
		49, 48, 
		38, 39,
		35, 34, 33, 32
	};
	int mouth[] = {
		27, 26, 25,
		14, 15, 16, 17, 18, 19, 20,
		13, 12, 11, 10, 9, 8, 7,
		0, 1, 2
	};

	paint(
		faceNeoPixels->Color(9, 65, 230),
		iris, 
		sizeof(iris)/sizeof(int));

	paint(
		faceNeoPixels->Color(255, 255, 255),
		sclera, 
		sizeof(sclera)/sizeof(int));

	paint(
		faceNeoPixels->Color(230, 40, 90),
		mouth, 
		sizeof(mouth)/sizeof(int));

	faceNeoPixels->show();
}

void Face::paint(uint32_t color, int* pixels, int pixelsLength) {
	while (pixelsLength--) {// decrement by one and returns the old value of
		faceNeoPixels->setPixelColor(*(pixels + pixelsLength), color);
	}
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
