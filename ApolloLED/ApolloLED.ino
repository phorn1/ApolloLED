/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

#include "FastLED\FastLED.h"
#include "ledBaseFunc.h"


CRGB leds[NUM_LEDS];
uint8_t volume = 5;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.setBrightness(255);
	FastLED.show();
}

// the loop function runs over and over again until power down or reset
void loop() {

	musicAnimation();
	delay(5);
}


// class test
//
void musicAnimation()
{
	CRGB color = 0x00;
	uint8_t data[4] = { 1, 2, 3, 5 }; // current loudness
	processAudio(data); // calculate audio data
	uint8_t peak = ((data[0] + data[1] + data[2] + data[3]) / 4);
	static uint8_t val = 1;
	static uint8_t wheelPos = 1;

	if (peak > volume) {
		
		val = (peak * 15 / volume);
		Serial.println(val);
		if (val > NUM_LEDS)
		{
			val = NUM_LEDS;
		}

		for (int i = 0; i < val; i++)
		{
			wheel(wheelPos + (i*5), 1, &color);
			leds[i] = color;
		}
	}

	for (uint8_t i = 0; i < val; i++)
	{
		leds[i].fadeToBlackBy((i/2) + 10);
	}
	for (uint8_t i = val; i < NUM_LEDS; i++)
	{
		leds[i].fadeToBlackBy(50);
	}
	wheelPos++;
	FastLED.show();
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

void singlecolor(struct CRGB* color) {
	color-> r = 0;
}
