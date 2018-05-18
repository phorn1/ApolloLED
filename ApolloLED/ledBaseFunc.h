#pragma once
#include "FastLED/FastLED.h"
// basic led functions
//
#define FFT_SIZE	128
#define DATA_PIN	9
#define MIC_PIN		A0
#define BUF_SIZE	4



// Get Audio data and start FFT
uint8_t processAudio();

void saveConfigEEPROM(struct globalConfig c);
void loadConfigEEPROM(struct globalConfig* c);
