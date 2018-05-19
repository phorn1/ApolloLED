#include "ledBaseFunc.h"
#include "fix_fft.h"

// Get Audio data and start FFT
uint8_t processAudio() {
	char im[FFT_SIZE], re[FFT_SIZE], data[BUF_SIZE];
	uint8_t maxPeekVal = 0;			// max value of peek

	for (uint8_t i = 0; i < FFT_SIZE; i++)
	{
		re[i] = (analogRead(MIC_PIN) / 4 -128);
		im[i] = 0;
	}
	fix_fft(re, im, 7, 0);	// fft function

	data[0] = sqrt(re[0] * re[0] + im[0] * im[0]) * 10;
	data[1] = sqrt(re[1] * re[1] + im[1] * im[1]) * 10;
	data[2] = sqrt(re[2] * re[2] + im[2] * im[2]) * 10;
	data[3] = sqrt(re[3] * re[3] + im[3] * im[3]) * 10;

	// getting highest peek and return value
	for (uint8_t i = 0; i < BUF_SIZE; i++)
	{
		if (data[i] > maxPeekVal)
		{
			maxPeekVal = data[i];
		}
	}
	return maxPeekVal;
}


void saveConfigEEPROM(struct globalConfig c)
{
	Serial.println("Saving Config EEPROM");
	eeprom_write_block(&c, 0, sizeof(globalConfig));
}

void loadConfigEEPROM(struct globalConfig* c)
{
	Serial.println("Reading Config EEPROM");
	eeprom_read_block(c, 0, sizeof(globalConfig));
}

