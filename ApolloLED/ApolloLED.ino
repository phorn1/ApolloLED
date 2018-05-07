/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

#include "FastLED\FastLED.h"
#include "ledBaseFunc.h"

struct message {
	uint8_t mode;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t brightness;
	uint8_t volume;
};


CRGB leds[NUM_LEDS];
uint8_t volume = 5;
bool btFlag = false;
uint8_t mode = 0;

void btInterrupt()
{
	btFlag = true;
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.setBrightness(255);

	pinMode(RECV_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(RECV_PIN), btInterrupt, CHANGE);

	FastLED.show();
}

// the loop function runs over and over again until power down or reset
//TODO
void loop() {
	if (btInterrupt)
	{
		if (Serial.available())
		{
			uint8_t msg[6];
			for (uint8_t i = 0; i < 6; i++)
			{
				Serial.readBytes(&msg[i], 1);
			}

			//parsing message in struct message
			struct message currentMessage;
			currentMessage.mode = msg[0];
			currentMessage.r = msg[1];
			currentMessage.g = msg[2];
			currentMessage.b = msg[3];
			currentMessage.brightness = msg[4];
			currentMessage.volume = msg[5];
			changeMode(&currentMessage);
			config(&currentMessage);
		}
		btFlag = false;
	}
	else
	{
		// calling animation depending on mode value
		switch (mode)
		{
		case 1:
			musicAnimation();
			break;
		default:
			break;
		}
	}
}


void changeMode(struct message* currentMessage)
{
	switch (currentMessage->mode)
	{
	// single Color
	case 0:
		mode = 0;
		singlecolor(currentMessage->r, currentMessage->g, currentMessage->b);
		break;
	// music function
	case 1:
		mode = 1;
		break;
	// more Funktions
	}
}

void config(struct message* currentMessage)
{
	// setting global configs
	// TODO
	//
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

void singlecolor(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t i = 0; i < NUM_LEDS; i++)
	{
		leds[i].r = r;
		leds[i].g = g;
		leds[i].b = b;
	}
}
