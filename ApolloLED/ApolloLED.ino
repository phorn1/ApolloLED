/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

#include "FastLED\FastLED.h"

#define NUM_LEDS	31
#define DATA_PIN	9

CRGB leds[NUM_LEDS];


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
	fill_solid(leds, NUM_LEDS, CRGB::White);
	FastLED.show();
}

// the loop function runs over and over again until power down or reset
void loop() {

	rainbowWheel();
	delay(5);
}


// rainbow color 
void rainbowWheel()
{
	struct CRGB color = 0x000000; // store current color

	static uint8_t k = 0; // WheelPosition

	for (uint8_t i = 0; i < NUM_LEDS; i++) { // Set Leds
		wheel(((i * 256 / NUM_LEDS) + k) % 256, 1, &color);

		leds[i] = color;
	}
	k++;
	FastLED.show();
}

void rainbowAll() {
	struct CRGB color = 0x000000; // store current color
	static uint8_t j = 0;

	wheel(j, 1, &color);

	for (uint8_t i = 0; i < NUM_LEDS; i++) { // Set Leds
		leds[i] = color;
	}

	j++;
	FastLED.show();
}

// calculates Color
void wheel(uint8_t WheelPos, uint8_t Dim, struct CRGB* color) {
	if (WheelPos < 85) {
		//*color = (uint32_t)(0 << 16) | (uint32_t)((WheelPos * 3 / Dim) << 8) | ((255 - WheelPos * 3) / Dim);
		color->r = 0;
		color->g = WheelPos * 3 / Dim;
		color->b = (255 - WheelPos * 3) / Dim;
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		//*color = (uint32_t)((WheelPos * 3 / Dim) << 16)| (uint32_t)(((255 - WheelPos * 3) / Dim) << 8) | 0;
		color->r = WheelPos * 3 / Dim;
		color->g = (255 - WheelPos * 3) / Dim;
		color->b = 0;
	}
	else {
		WheelPos -= 170;
		//*color = (uint32_t)(((255 - WheelPos * 3) / Dim) << 16) | (uint32_t)(0 << 8) | (WheelPos * 3 / Dim);
		color->r = (255 - WheelPos * 3) / Dim;
		color->g = 0;
		color->b = WheelPos * 3 / Dim;
	}
}