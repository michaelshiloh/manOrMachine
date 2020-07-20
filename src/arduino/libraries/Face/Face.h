
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

#ifndef Face_h
#define Face_h

#include "Arduino.h"

class Face
{
  public:

    Face ( int pixelCount, int pixelPin);
    void init();
    void smile();
		void frown();
		void eyesLeft();
		void eyesRight();
		void clear();

	private:

    Adafruit_NeoPixel *faceNeoPixels;
		int _pixelCount;
		int _pixelPin;
		void flash(int times);
		void paint( uint32_t color, int* pixels, int pixelsLength);
};

#endif
