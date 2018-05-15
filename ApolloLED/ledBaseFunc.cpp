#include "ledBaseFunc.h"

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
		re[i] = analogRead(MIC_PIN);
		im[i] = 0;
	}
	fix_fft(re, im, 7, 0);

	// return value of 4 frequencies
	data[0] = (uint8_t)sqrt(re[data[0]] * re[data[0]] + im[data[0]] * im[data[0]]);
	data[1] = (uint8_t)sqrt(re[data[1]] * re[data[1]] + im[data[1]] * im[data[1]]);
	data[2] = (uint8_t)sqrt(re[data[2]] * re[data[2]] + im[data[2]] * im[data[2]]);
	data[3] = (uint8_t)sqrt(re[data[3]] * re[data[3]] + im[data[3]] * im[data[3]]);
	//Serial.println(data[0]);
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

/*
FIX_MPY() - fixed-point multiplication & scaling.
Substitute inline assembly for hardware-specific
optimization suited to a particluar DSP processor.
Scaling ensures that result remains 16-bit.
*/
inline char FIX_MPY(char a, char b)
{

	//Serial.println(a);
	//Serial.println(b);


	/* shift right one less bit (i.e. 15-1) */
	int c = ((int)a * (int)b) >> 6;
	/* last bit shifted out = rounding-bit */
	b = c & 0x01;
	/* last shift + rounding bit */
	a = (c >> 1) + b;

	/*
	Serial.println(Sinewave[3]);
	Serial.println(c);
	Serial.println(a);
	while(1);*/

	return a;
}

/*
fix_fft() - perform forward/inverse fast Fourier transform.
fr[n],fi[n] are real and imaginary arrays, both INPUT AND
RESULT (in-place FFT), with 0 <= n < 2**m; set inverse to
0 for forward transform (FFT), or 1 for iFFT.
*/
int fix_fft(char fr[], char fi[], int m, int inverse)
{
	int mr, nn, i, j, l, k, istep, n, scale, shift;
	char qr, qi, tr, ti, wr, wi;

	n = 1 << m;

	/* max FFT size = N_WAVE */
	if (n > N_WAVE)
		return -1;

	mr = 0;
	nn = n - 1;
	scale = 0;

	/* decimation in time - re-order data */
	for (m = 1; m <= nn; ++m) {
		l = n;
		do {
			l >>= 1;
		} while (mr + l > nn);
		mr = (mr & (l - 1)) + l;

		if (mr <= m)
			continue;
		tr = fr[m];
		fr[m] = fr[mr];
		fr[mr] = tr;
		ti = fi[m];
		fi[m] = fi[mr];
		fi[mr] = ti;
	}

	l = 1;
	k = LOG2_N_WAVE - 1;
	while (l < n) {
		if (inverse) {
			/* variable scaling, depending upon data */
			shift = 0;
			for (i = 0; i<n; ++i) {
				j = fr[i];
				if (j < 0)
					j = -j;
				m = fi[i];
				if (m < 0)
					m = -m;
				if (j > 16383 || m > 16383) {
					shift = 1;
					break;
				}
			}
			if (shift)
				++scale;
		}
		else {
			/*
			fixed scaling, for proper normalization --
			there will be log2(n) passes, so this results
			in an overall factor of 1/n, distributed to
			maximize arithmetic accuracy.
			*/
			shift = 1;
		}
		/*
		it may not be obvious, but the shift will be
		performed on each data point exactly once,
		during this pass.
		*/
		istep = l << 1;
		for (m = 0; m<l; ++m) {
			j = m << k;
			/* 0 <= j < N_WAVE/2 */
			wr = pgm_read_word_near(Sinewave + j + N_WAVE / 4);

			/*Serial.println("asdfasdf");
			Serial.println(wr);
			Serial.println(j+N_WAVE/4);
			Serial.println(Sinewave[256]);

			Serial.println("");*/


			wi = -pgm_read_word_near(Sinewave + j);
			if (inverse)
				wi = -wi;
			if (shift) {
				wr >>= 1;
				wi >>= 1;
			}
			for (i = m; i<n; i += istep) {
				j = i + l;
				tr = FIX_MPY(wr, fr[j]) - FIX_MPY(wi, fi[j]);
				ti = FIX_MPY(wr, fi[j]) + FIX_MPY(wi, fr[j]);
				qr = fr[i];
				qi = fi[i];
				if (shift) {
					qr >>= 1;
					qi >>= 1;
				}
				fr[j] = qr - tr;
				fi[j] = qi - ti;
				fr[i] = qr + tr;
				fi[i] = qi + ti;
			}
		}
		--k;
		l = istep;
	}
	return scale;
}