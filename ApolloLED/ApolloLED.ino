/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/


#include "FastLED/FastLED.h"
#include "SoftwareSerial-master\SoftwareSerial.h"
#include "ledBaseFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


SoftwareSerial BTSerial(10, 11); // RX | TX

#define FRAMES_PER_SECOND  30

enum animMode
{
	single_Color, rainbow_Wheel, music_animation1, music_animation2
};
;
enum palette
{
	eRainbowColors_p, eCloudColors_p, eLavaColors_p, eOceanColors_p, eForestColors_p, ePartyColors_p
};

enum configMsgID
{
	ebrightness, espeed, esenisivity, etheme
};

enum infoMsgType
{
	connect, disconnect, save
};

enum msgType
{
	info_Msg, singleColor_Msg, changeMode_Msg, config_Msg
};



struct globalConfig {
	uint8_t sensitivity = 100;
	uint8_t brightness = 255;
	uint8_t speed = 100;
	animMode mode = single_Color;
	uint8_t numLeds = 0;
	CRGB color = CRGB::Black;
	CRGBPalette16 currentPalette;
};




uint8_t msgLength[] = {1, 3, 1, 2, 0};	// length of message type, number of passed arguments
struct globalConfig config;
CRGB* leds;



// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	BTSerial.begin(9600);
	pinMode(MIC_PIN, INPUT);	// only needed when the samples are read with analogRead() 
	analogReference(DEFAULT);

	loadConfigEEPROM(&config);

	// allocate memory for led array, setting free with changeNumLed function
	leds = (CRGB*) malloc(sizeof(CRGB) * config.numLeds);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, config.numLeds);

	// set color to stored color in eeprom. only need when no animation is running
	if (config.mode == single_Color)
	{
		fill_solid(leds, config.numLeds, config.color);
	}

	FastLED.setBrightness(config.brightness);
	FastLED.show();
}

// the loop function runs over and over again until power down or reset
void loop() {
	static bool peak = false;
	static uint8_t amp;
	unsigned long timerBegin;
	float soundVol;
	
	if (BTSerial.available())
	{
		// BT Input
		uint8_t typeID;
		BTSerial.readBytes(&typeID, 1);
		Serial.println((uint8_t)typeID);
		uint8_t msgBuf[10];
		BTSerial.readBytes(msgBuf, msgLength[typeID]);
		for (int i = 0; i < 10; i++)
		{
			Serial.println((uint8_t)msgBuf[i]);
		}
		processMessage(typeID, msgBuf);
	}
	// calling animation depending on mode value
	switch (config.mode)
	{
	case rainbow_Wheel:
		rainbowWheel();
		break;
	case music_animation1:
		musicAnimation1(peak, amp);
		break;
	case music_animation2:
		musicAnimation2(peak, amp);
		break;
	default:
		break;
	}
	delay(1000 / config.speed);
	FastLED.show();
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
			saveConfig();
			connectAnim(save);
		}
	}
	// 1: Single Color
	//       ID|  r  |  g  |  b  | 
	// msg:  1 | 255 | 255 | 255 | 
	else if (typeID == singleColor_Msg)
	{
		Serial.println("SingleColor-Msg");
		config.mode = single_Color;
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
		config.mode = (animMode) msgBuf[0];
		Serial.println(config.mode);
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
			config.brightness = value;
			FastLED.setBrightness(value);
		}
		// config speed of animations
		else if (configID == espeed)
		{
			config.speed = value;
		}
		// config sensetivity of music animations
		else if (configID == esenisivity)
		{
			config.sensitivity = value;
			//Serial.println(sensitivity);
		}
		else if (configID == etheme)
		{
			changeTheme(value);
		}
		FastLED.show();
	}
}

void connectAnim(uint8_t type)
{
	// if leds divisible by two and rest, then set the middle led
	if (config.numLeds % 2)
	{
		if (type == connect)
		{
			leds[(config.numLeds / 2)] = CRGB::Blue;
		}
		else if (type == disconnect)
		{
			leds[(config.numLeds / 2)] = CRGB::Red;
		}
		else if (type == save)
		{
			leds[(config.numLeds / 2)] = CRGB::Violet;
		}
	}
	// if no rest then set the middle two leds
	else
	{
		if (type == connect)
		{
			leds[(config.numLeds / 2)] = CRGB::Blue;
			leds[(config.numLeds / 2) - 1] = CRGB::Blue;
		}
		else if (type == disconnect)
		{
			leds[(config.numLeds / 2)] = CRGB::Red;
			leds[(config.numLeds / 2) -1] = CRGB::Red;
		}
		else if (type == save)
		{
			leds[(config.numLeds / 2)] = CRGB::Violet;
			leds[(config.numLeds / 2) - 1] = CRGB::Violet;
		}
	}
	FastLED.show();
	fadeFromMiddle();
}

void musicAnimation1(bool peak, uint8_t amp) {
	if (peak) {
		for (int i = 0; i <= amp; i++) {
			leds[random16(config.numLeds)] = ColorFromPalette(RainbowColors_p, random8());
		}
	}
	fadeToBlackBy(leds, config.numLeds, 40);
}

void musicAnimation2(bool peak, uint8_t amp) {
	if (peak) {

		leds[(config.numLeds / 2)] = ColorFromPalette(RainbowColors_p, random8());
	}

	for (uint8_t j = 0; j < (config.numLeds / 2) ; j++)
	{
		leds[j] = leds[j + 1];
	}
	for (uint8_t j = config.numLeds - 1; j > (config.numLeds / 2); j--)
	{
		leds[j] = leds[j - 1];
	}
	fadeToBlackBy(leds, config.numLeds, 100);
	FastLED.show();
}

void equalizerM()
{
	CRGB color = 0x00;
	uint8_t maxPeekVal = processAudio(); // calculate audio data and returns value of highest peek
	Serial.println(maxPeekVal);
	uint8_t numLedPeek;
	static uint8_t wheelPos = 1;

	numLedPeek = (maxPeekVal / 255) * config.numLeds;




	if (maxPeekVal >= config.sensitivity)
	{
		for (int i = 0; i < numLedPeek; i++)
		{
			leds[i] = ColorFromPalette(config.currentPalette, wheelPos, config.brightness);
		}
	}

	for (uint8_t i = 0; i < numLedPeek; i++)
	{
		leds[i].fadeToBlackBy((i/2) + 10);
	}
	for (uint8_t i = numLedPeek; i < config.numLeds; i++)
	{
		leds[i].fadeToBlackBy(50);
	}
	wheelPos++;
	FastLED.show();
}

void rainbowWheel() {
	static uint8_t wheelpos = 0;
	for (int i = 0; i < config.numLeds; i++) {
		leds[i] = ColorFromPalette(config.currentPalette, wheelpos + i*5);
	}
	wheelpos++;
}


void singlecolor(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t i = 0; i < config.numLeds; i++)
	{
		leds[i].r = r;
		leds[i].g = g;
		leds[i].b = b;
	}
	FastLED.show();
}

void fadeFromMiddle()
{
	for (uint8_t i = 0; i < config.numLeds*3; i++)
	{
		for (uint8_t j = config.numLeds - 1; j >(config.numLeds / 2); j--)
		{
			leds[j] = leds[j - 1];
		}
		for (uint8_t j = 0; j < (config.numLeds / 2); j++)
		{
			leds[j] = leds[j + 1];
		}
		for (uint8_t j = 0; j < config.numLeds; j++)
		{
			leds[j].fadeToBlackBy(15);
		}
		FastLED.show();
		delay(15);
	}
}


void saveConfig()
{
	//TODO
}

void changeTheme(uint8_t value)
{
	value = (palette)value;
	CRGBPalette16 tempPalette;

	switch (value)
	{
	case eRainbowColors_p:
		tempPalette = RainbowColors_p;
		break;
	case eCloudColors_p:
		tempPalette = CloudColors_p;
		break;
	case eLavaColors_p:
		tempPalette = LavaColors_p;
		break;
	case eOceanColors_p:
		tempPalette = OceanColors_p;
		break;
	case eForestColors_p:
		tempPalette = ForestColors_p;
		break;
	case ePartyColors_p:
		tempPalette = PartyColors_p;
		break;
	default:
		tempPalette = PartyColors_p;
		break;
	}
	config.currentPalette = tempPalette;
}
