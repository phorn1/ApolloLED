#include "ledBaseFunc.h"
#include "fix_fft.h"

// calculates Color
void wheel(uint8_t WheelPos, uint8_t Dim, struct CRGB* color) {
	if (WheelPos < 85) {
		//*color = (uint32_t)(0 << 16) | (uint32_t)((WheelPos * 3 / Dim) << 8) | ((255 - WheelPos * 3) / Dim);
		color->r = 0;
		color->g = WheelPos * 3 / Dim;
		color->b = (255 - WheelPos * 3) / Dim;
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		//*color = (uint32_t)((WheelPos * 3 / Dim) << 16)| (uint32_t)(((255 - WheelPos * 3) / Dim) << 8) | 0;
		color->r = WheelPos * 3 / Dim;
		color->g = (255 - WheelPos * 3) / Dim;
		color->b = 0;
	}
	else {
		WheelPos -= 170;
		//*color = (uint32_t)(((255 - WheelPos * 3) / Dim) << 16) | (uint32_t)(0 << 8) | (WheelPos * 3 / Dim);
		color->r = (255 - WheelPos * 3) / Dim;
		color->g = 0;
		color->b = WheelPos * 3 / Dim;
	}
}
// Get Audio data and start FFT
void processAudio(uint8_t data[]) {
	char im[FFT_SIZE], re[FFT_SIZE];
	for (uint8_t i = 0; i < FFT_SIZE; i++)
	{
		re[i] = (analogRead(MIC_PIN) / 4 -128);
		im[i] = 0;
	}
	fix_fft(re, im, 7, 0);

	data[0] = sqrt(re[0] * re[0] + im[0] * im[0]);
	data[1] = sqrt(re[1] * re[1] + im[1] * im[1]);
	data[2] = sqrt(re[2] * re[2] + im[2] * im[2]);
	data[3] = sqrt(re[3] * re[3] + im[3] * im[3]);
	data[4] = sqrt(re[4] * re[4] + im[4] * im[4]);
	data[5] = sqrt(re[5] * re[5] + im[5] * im[5]);
}

#define MicSamples (1024*2)
#define AmpMax (1024 /2)

//returns commonly values in the range of 2.5 and 15
float MeasureVolume()
{
	static int averageVal = 320;				// estimatated base line of adc
	long avg = 0, soundVolRMS = 0;
	for (int i = 0; i < MicSamples; i++)
	{
		while (!(ADCSRA & /*0x10*/_BV(ADIF)));	// wait for adc to be ready (ADIF)
		sbi(ADCSRA, ADIF);						// restart adc
		byte m = ADCL;							// fetch adc data
		byte j = ADCH;
		int k = ((int)j << 8) | m;				// form into an int
		avg += k;
		int amp = abs(k - averageVal);			
		
		soundVolRMS += ((long)amp*amp);			//Root Mean Square(RMS) so that higher amplitutes get weighted more
	}
	averageVal = avg / MicSamples;
	soundVolRMS /= MicSamples;

	return (float)sqrt(soundVolRMS);					//Return the RMS of the amplitudes
}
