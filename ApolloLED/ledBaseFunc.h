#include "FastLED\FastLED.h"
#pragma once
// basic led functions
//
#define FFT_SIZE	128
#define NUM_LEDS	31
#define DATA_PIN	9
#define MIC_PIN		A0
#define ASCII	    48


#define N_WAVE	256    /* full length of Sinewave[] */
#define LOG2_N_WAVE 8	/* log2(N_WAVE) */

#define pgm_read_word_near(address_short) __LPM_word((uint16_t)(address_short))

// calculates Color
void wheel(uint8_t WheelPos, uint8_t Dim, struct CRGB* color);

// Get Audio data and start FFT
void processAudio(uint8_t data[]);


// Do FFT
/*
Since we only use 3/4 of N_WAVE, we define only
this many samples, in order to conserve data space.
*/

const int8_t Sinewave[N_WAVE - N_WAVE / 4] PROGMEM = {
	0, 3, 6, 9, 12, 15, 18, 21,
	24, 28, 31, 34, 37, 40, 43, 46,
	48, 51, 54, 57, 60, 63, 65, 68,
	71, 73, 76, 78, 81, 83, 85, 88,
	90, 92, 94, 96, 98, 100, 102, 104,
	106, 108, 109, 111, 112, 114, 115, 117,
	118, 119, 120, 121, 122, 123, 124, 124,
	125, 126, 126, 127, 127, 127, 127, 127,

	127, 127, 127, 127, 127, 127, 126, 126,
	125, 124, 124, 123, 122, 121, 120, 119,
	118, 117, 115, 114, 112, 111, 109, 108,
	106, 104, 102, 100, 98, 96, 94, 92,
	90, 88, 85, 83, 81, 78, 76, 73,
	71, 68, 65, 63, 60, 57, 54, 51,
	48, 46, 43, 40, 37, 34, 31, 28,
	24, 21, 18, 15, 12, 9, 6, 3,

	0, -3, -6, -9, -12, -15, -18, -21,
	-24, -28, -31, -34, -37, -40, -43, -46,
	-48, -51, -54, -57, -60, -63, -65, -68,
	-71, -73, -76, -78, -81, -83, -85, -88,
	-90, -92, -94, -96, -98, -100, -102, -104,
	-106, -108, -109, -111, -112, -114, -115, -117,
	-118, -119, -120, -121, -122, -123, -124, -124,
	-125, -126, -126, -127, -127, -127, -127, -127,

	/*-127, -127, -127, -127, -127, -127, -126, -126,
	-125, -124, -124, -123, -122, -121, -120, -119,
	-118, -117, -115, -114, -112, -111, -109, -108,
	-106, -104, -102, -100, -98, -96, -94, -92,
	-90, -88, -85, -83, -81, -78, -76, -73,
	-71, -68, -65, -63, -60, -57, -54, -51,
	-48, -46, -43, -40, -37, -34, -31, -28,
	-24, -21, -18, -15, -12, -9, -6, -3, */
};

/*
FIX_MPY() - fixed-point multiplication & scaling.
Substitute inline assembly for hardware-specific
optimization suited to a particluar DSP processor.
Scaling ensures that result remains 16-bit.
*/
inline char FIX_MPY(char a, char b);
/*
fix_fft() - perform forward/inverse fast Fourier transform.
fr[n],fi[n] are real and imaginary arrays, both INPUT AND
RESULT (in-place FFT), with 0 <= n < 2**m; set inverse to
0 for forward transform (FFT), or 1 for iFFT.
*/
int fix_fft(char fr[], char fi[], int m, int inverse);