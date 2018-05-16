/*
 Name:		ApolloLED.ino
 Created:	4/26/2018 1:25:41 PM
 Author:	piush
*/

#include "FastLED/FastLED.h"
#include "AltSoftSerial.h"
#include "ledBaseFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


AltSoftSerial BTSerial(10, 12); // RX | TX

#define FRAMES_PER_SECOND  30

enum animMode
{
	single_Color, rainbow_Wheel, music_animation1
};
;
enum palette
{
	eRainbowColors_p, eCloudColors_p, eLavaColors_p, eOceanColors_p, eForestColors_p, ePartyColors_p
};

enum configMsgID
{
	ebrightness, espeed, esenisivity
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
uint8_t sensitivity = 4;
uint8_t brightness = 5;
uint8_t speed = 60;
bool btFlag = false;
bool listening_to_sound = true;
animMode mode = music_animation1;
//TProgmemRGBPalette16 currentPalette = RainbowColors_p;


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	BTSerial.begin(9600);
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numLeds);
	fill_solid(leds, numLeds, CRGB::Black);
	FastLED.setBrightness(brightness);


	//Microphone setup with the instructions found at: https://blog.yavilevich.com/2016/08/arduino-sound-level-meter-and-spectrum-analyzer/
	pinMode(MIC_PIN, INPUT);	// only needed when the samples are read with analogRead() 
								// instead we'll get accurate sampling with 3.3V reference and free running
	analogReference(3.3);		// default analog reference is 5V, we change it to the 3.3V because its more stable
	// register explanation: http://maxembedded.com/2011/06/the-adc-of-the-avr/
	// 7 =&gt; switch to divider=128, default 9.6khz sampling
	ADCSRA = 0xe0 + 7; // "ADC Enable", "ADC Start Conversion", "ADC Auto Trigger Enable" and divider.
	ADMUX = 0x0; // Use adc0 (hardcoded, doesn't use MicPin). Use ARef pin for analog reference (same as analogReference(EXTERNAL)).
	ADMUX |= 0x40; // Use Vcc for analog reference.
	DIDR0 = 0x01; // turn off the digital input for adc0

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
	case music_animation1:
		musicAnimation1(peak, amp);
		break;
	default:
		break;
	}
	// this ensures that music is also measured during the break
	if (listening_to_sound) { //listening_to_sound has to be set in processMessage()
		timerBegin = millis();
		peak = false;
		amp = 0;
		while (millis() - timerBegin < 1000 / speed) {
			soundVol = MeasureVolume();
			if (soundVol > sensitivity) {
				peak = true;
				if (amp < (uint8_t)soundVol - sensitivity) {
					amp = (uint8_t)soundVol - sensitivity;
				}
			}
		}
	}
	else {
		delay(1000 / speed);
	}
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
		if (mode == music_animation1) {
			listening_to_sound = true;
		}
		else {
			listening_to_sound = false;
		}
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
		else if (configID == esenisivity)
		{
			sensitivity = value;
			Serial.println(sensitivity);
		}
		FastLED.show();
	}
}

void connectAnim(uint8_t type)
{
	// if leds divisible by two and rest, then set the middle led
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
	// if no rest then set the middle two leds
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
	FastLED.show();
	fadeFromMiddle();
}

void musicAnimation1(bool peak, uint8_t amp) {
	if (peak) {
		for (int i = 0; i <= amp; i++) {
			leds[random16(numLeds)] = ColorFromPalette(RainbowColors_p, random8());
		}
	}
	fadeToBlackBy(leds, numLeds, 40);
}

void musicAnimation2(bool peak, uint8_t amp) {
	bool ledArray[numLeds] = { 0 };
	if (peak) {
		if (numLeds % 2){
			leds[(numLeds / 2)] = ColorFromPalette(RainbowColors_p, 0);
			ledArray[(numLeds / 2)] = true;
		}
	}
	else {
		leds[(numLeds / 2)] = CRGB::Black;
		ledArray[(numLeds / 2)] = false;
	}

	for (uint8_t j = 0; j < (numLeds / 2); j++)
	{
		if (ledArray[j + 1] = true) {
			leds[j] = leds[j + 1];
			ledArray[j] = true;
		}	
	}
	for (uint8_t j = numLeds; j > (numLeds / 2); j--)
	{
		if (ledArray[j - 1] = true) {
			leds[j] = leds[j + 1];
			ledArray[j] = true;
		}
	}
}

void equalizerM()
{
	CRGB color = 0x00;
	uint8_t data[4] = { 1, 2, 3, 5 }; // current loudness
	processAudio(data); // calculate audio data
	uint8_t peak = ((data[0] + data[1] + data[2] + data[3]) / 4);
	static uint8_t val = 1;
	static uint8_t wheelPos = 1;

	if ((data[0] > sensitivity) && (data[1] > sensitivity) && (data[2] > sensitivity) && (data[3] > sensitivity))
	{
		val = (peak - sensitivity) * 2;
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

void rainbowWheel() {
	static uint8_t wheelpos = 0;
	for (int i = 0; i < numLeds; i++) {
		leds[i] = ColorFromPalette(RainbowColors_p, wheelpos + i*5);
	}
	wheelpos++;
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


void saveConfig()
{
	//TODO
}