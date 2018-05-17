#include "FastLED/FastLED.h"
#include "arduinoFFT-1.4.0/src/arduinoFFT.h"
#pragma once
// basic led functions
//
#define FFT_SIZE	128
#define DATA_PIN	9
#define MIC_PIN		A0


// http://yaab-arduino.blogspot.co.il/2015/02/fast-sampling-from-analog-input.html
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &amp;= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))




// calculates Color
void wheel(uint8_t WheelPos, uint8_t Dim, struct CRGB* color);

// Get Audio data and start FFT
void processAudio(uint8_t data[]);

float MeasureVolume();