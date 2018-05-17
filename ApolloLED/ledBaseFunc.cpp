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

	uint8_t data2[64];

	for (int i = 0; i < 64; i++)
		data2[i] = (uint8_t)sqrt(re[i] * re[i] + im[i] * im[i]);

	for (int i = 0; i < 64; i++)
	{
		Serial.print(data2[i]);
		Serial.print(" ");
	}
	Serial.println();
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
