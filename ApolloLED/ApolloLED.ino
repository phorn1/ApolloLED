/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

//#include "../external/SoftwareSerial/SoftwareSerial.h"
#include "FastLED/FastLED.h"
#include "ledBaseFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

SoftwareSerial BTSerial(10, 12); // RX | TX

enum animMode
{
	single_Color, rainbow_Wheel, equalizer_M
};

enum configMsgID
{
	ebrightness, espeed, evolume
};

enum infoMsgType
{
	connect, disconnect, save
};

enum msgType
{
	info_Msg, singleColor_Msg, changeMode_Msg, config_Msg
};


uint8_t const numLeds = 31;
CRGB leds[31];
uint8_t msgLength[] = {1, 3, 1, 2, 0};	// length of message type, number of passed arguments
uint8_t volume = 5;
uint8_t brightness = 255;
uint8_t speed = 50;
bool btFlag = false;
animMode mode = single_Color;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	BTSerial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numLeds);
	fill_solid(leds, numLeds, CRGB::Black);
	FastLED.setBrightness(255);

	//Microphone
	pinMode(MIC_PIN, INPUT);

	FastLED.show();
}

// the loop function runs over and over again until power down or reset
void loop() {
	if (BTSerial.available())
	{
		// BT Input
		uint8_t typeID;
		BTSerial.readBytes(&typeID, 1);
		Serial.println((uint8_t)typeID);
		uint8_t msgBuf[10];
		BTSerial.readBytes(msgBuf, msgLength[typeID]);
		/*for (int i = 0; i < 10; i++)
		{
			Serial.println((uint8_t)msgBuf[i]);
		}*/
		processMessage(typeID, msgBuf);
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


void processMessage(uint8_t typeID, uint8_t* msgBuf)
{
	// switching which function is called
	// see protokoll definition for more informations
	//
	// 0: Info Message
	//       ID| type|
	// msg:  0 | 255 |
	if (typeID == info_Msg)
	{
		Serial.println("Info-Msg");
		uint8_t type = msgBuf[0];

		if (type == connect)
		{
			connectAnim(connect);
		}
		else if (type == disconnect)
		{
			connectAnim(disconnect);
		}
		else if (type == save)
		{
			connectAnim(save);
		}
	}
	// 1: Single Color
	//       ID|  r  |  g  |  b  | 
	// msg:  1 | 255 | 255 | 255 | 
	else if (typeID == singleColor_Msg)
	{
		Serial.println("SingleColor-Msg");
		mode = single_Color;
		uint8_t r = msgBuf[0];
		uint8_t g = msgBuf[1];
		uint8_t b = msgBuf[2];
		singlecolor(r, g, b);
	}
	// 2: Change Animation Mode
	//       ID|mode|
	// msg: 2 | 99 |
	else if (typeID == changeMode_Msg)
	{
		Serial.println("ChangeMode-Msg");
		mode = (animMode) msgBuf[0];
		Serial.println(mode);
	}
	// 3: Config Message, will change settings depending on configID
	//       ID|configID| value |
	// msg: 3 |   99   |  255  | 
	else if (typeID == config_Msg)
	{
		Serial.println("Config-Msg");
		uint8_t configID = msgBuf[0];
		uint8_t value = msgBuf[1];
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
			Serial.println(volume);
		}
		FastLED.show();
	}
}

void connectAnim(uint8_t type)
{
	// falls leds durch zwei teilbar und rest, dann setze die mittlere led
	if (numLeds % 2)
	{
		if (type == connect)
		{
			leds[(numLeds / 2)] = CRGB::Blue;
		}
		else if (type == disconnect)
		{
			leds[(numLeds / 2)] = CRGB::Red;
		}
		else if (type == save)
		{
			leds[(numLeds / 2)] = CRGB::Violet;
		}
	}
	// falls kein rest dann setze die mittleren beiden leds
	else
	{
		if (type == connect)
		{
			leds[(numLeds / 2)] = CRGB::Blue;
			leds[(numLeds / 2) - 1] = CRGB::Blue;
		}
		else if (type == disconnect)
		{
			leds[(numLeds / 2)] = CRGB::Red;
			leds[(numLeds / 2) -1] = CRGB::Red;
		}
		else if (type == save)
		{
			leds[(numLeds / 2)] = CRGB::Violet;
			leds[(numLeds / 2) - 1] = CRGB::Violet;
		}
	}
	fadeFromMiddle();
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

	if ((data[0] > volume) && (data[1] > volume) && (data[2] > volume) && (data[3] > volume))
	{
		val = (peak - volume) * 2;
		Serial.println(val);
		if (val > numLeds)
		{
			val = numLeds;
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
	for (uint8_t i = val; i < numLeds; i++)
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

	for (uint8_t i = 0; i < numLeds; i++) { // Set Leds
		wheel(((i * 256 / numLeds) + k) % 256, 1, &color);

		leds[i] = color;
	}
	k++;
	FastLED.show();
	delay(speed);
}

void singlecolor(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t i = 0; i < numLeds; i++)
	{
		leds[i].r = r;
		leds[i].g = g;
		leds[i].b = b;
	}
	FastLED.show();
}

void fadeFromMiddle()
{
	for (uint8_t i = 0; i < numLeds*3; i++)
	{
		for (uint8_t j = numLeds; j >(numLeds / 2); j--)
		{
			leds[j] = leds[j - 1];
		}
		for (uint8_t j = 0; j < (numLeds / 2); j++)
		{
			leds[j] = leds[j + 1];
		}
		for (uint8_t j = 0; j < numLeds; j++)
		{
			leds[j].fadeToBlackBy(15);
		}
		FastLED.show();
		delay(15);
	}
}
