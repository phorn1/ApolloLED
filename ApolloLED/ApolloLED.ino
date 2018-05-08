/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

#include "FastLED\FastLED.h"
#include "SoftwareSerial-master\SoftwareSerial.h"
#include "ledBaseFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

SoftwareSerial BTSerial(10, 11); // RX | TX

enum animMode
{
	single_Color, rainbow_Wheel, equalizer_M
};

enum configID
{
	ebrightness, espeed, evolume
};

CRGB leds[NUM_LEDS];
uint8_t volume = 5;
uint8_t brightness = 255;
uint8_t speed = 50;
bool btFlag = false;
animMode mode = single_Color;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	BTSerial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.setBrightness(255);

	FastLED.show();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (BTSerial.available())
	{
		// BT Input until reading '\n'
		char msgBuf[20];
		BTSerial.readBytesUntil('\n', msgBuf, 20);

		// correcting ASCII Value of message
		for (int i = 1; i < 20; i++)
		{
			msgBuf[i] -= ASCII;
		}

		processMessage(msgBuf);
	}
	// calling animation depending on mode value
	switch (mode)
	{
	case rainbow_Wheel:
		rainbowWheel();
		break;
	case equalizer_M:
		equalizerM();
		break;
	default:
		break;
	}
}


void processMessage(char* msgBuf)
{
	// extracting function ID
	char funcID = msgBuf[0];

	// switching which function is called
	// see protokoll definition for more informations
	//
	// A: Single Color
	//       ID|  r  |  g  |  b  | 
	// msg: "A | 255 | 255 | 255 | \n"
	if (funcID == 'A')
	{
		Serial.println("A");
		mode = single_Color;
		uint8_t r = msgBuf[1] * 100 + msgBuf[2] * 10 + msgBuf[3];
		uint8_t g = msgBuf[4] * 100 + msgBuf[5] * 10 + msgBuf[6];
		uint8_t b = msgBuf[7] * 100 + msgBuf[8] * 10 + msgBuf[9];
		singlecolor(r, g, b);
	}
	// B: Change Animation Mode
	//       ID|mode|
	// msg: "B | 99 | \n"
	else if (funcID == 'B')
	{
		Serial.println("B");
		mode = static_cast<animMode>(msgBuf[1] * 10 + msgBuf[2]);
		Serial.println(mode);
	}
	// C: Config Message, will change settings depending on configID
	//       ID|configID| value |
	// msg: "C |   99   |  255  | \n"
	else if (funcID == 'C')
	{
		Serial.println("C");
		uint8_t configID = msgBuf[1] * 10 + msgBuf[2];
		uint8_t value = msgBuf[3] * 100 + msgBuf[4] * 10 + msgBuf[5];
		// config brightness of strip
		if (configID == ebrightness)
		{
			brightness = value;
			FastLED.setBrightness(value);
		}
		// config speed of animations
		else if (configID == espeed)
		{
			speed = value;
		}
		// config sensetivity of music animations
		else if (configID == evolume)
		{
			volume = value;
		}
		FastLED.show();
	}
	// D:
	//TODO

	// S: Message to save current settings on controller EEPROM
	//       ID|
	// msg: "S | \n"
	else if (funcID == 'S')
	{
		//EEPROM Saving Data
		//TODO
	}
}




// class test
//
void equalizerM()
{
	CRGB color = 0x00;
	uint8_t data[4] = { 1, 2, 3, 5 }; // current loudness
	processAudio(data); // calculate audio data
	uint8_t peak = ((data[0] + data[1] + data[2] + data[3]) / 4);
	static uint8_t val = 1;
	static uint8_t wheelPos = 1;

	if (peak > volume) {
		
		val = (peak * 15 / volume);
		//Serial.println(val);
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
	delay(speed);
}

void singlecolor(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t i = 0; i < NUM_LEDS; i++)
	{
		leds[i].r = r;
		leds[i].g = g;
		leds[i].b = b;
	}
	FastLED.show();
}
