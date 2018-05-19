#pragma once
#include "FastLED/FastLED.h"
// basic led functions
//
#define FFT_SIZE	128
#define DATA_PIN	9
#define MIC_PIN		A0
#define BUF_SIZE	4


enum animMode
{
	single_Color, rainbow_Wheel, music_animation1, music_animation2, eEqualizerM
};
;
enum palette
{
	eRainbowColors_p, eCloudColors_p, eLavaColors_p, eOceanColors_p, eForestColors_p, ePartyColors_p
};

enum configMsgID
{
	ebrightness, espeed, esenisivity, enumLeds, etheme
};

enum infoMsgType
{
	connect, disconnect, save
};

enum msgType
{
	info_Msg, singleColor_Msg, changeMode_Msg, config_Msg
};


// start values
struct globalConfig {
	uint8_t sensitivity = 100;
	uint8_t brightness = 255;
	uint8_t speed = 100;
	animMode mode = single_Color;
	uint8_t numLeds = 20;
	CRGB color = CRGB::Green;
	CRGBPalette16 currentPalette;
};


// Get Audio data and start FFT
uint8_t processAudio();

void saveConfigEEPROM(struct globalConfig c);
void loadConfigEEPROM(struct globalConfig* c);
