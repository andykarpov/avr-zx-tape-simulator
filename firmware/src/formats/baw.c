#include "baw.h"

#include <avr/pgmspace.h>
#include "../settings.h"

#define BAW_INSTRUCTIONS_DELTA_8000		32
#define BAW_INSTRUCTIONS_DELTA_11025	32
#define BAW_INSTRUCTIONS_DELTA_16000	35
#define BAW_INSTRUCTIONS_DELTA_22050	35
#define BAW_INSTRUCTIONS_DELTA_32000	34
#define BAW_INSTRUCTIONS_DELTA_41000	32		// проверено

//const uint16_t BAW_SAMPLERATES[6] PROGMEM = {
//	BAW_SAMPLE_RATE_1,
//	BAW_SAMPLE_RATE_2,
//	BAW_SAMPLE_RATE_3,
//	BAW_SAMPLE_RATE_4,
//	BAW_SAMPLE_RATE_5,
//	BAW_SAMPLE_RATE_6
//};

// поправки для корректирования частоты таймера
const uint8_t BAW_INSTRUCTIONS_DELTA[6] PROGMEM = {
	BAW_INSTRUCTIONS_DELTA_8000,
	BAW_INSTRUCTIONS_DELTA_11025,
	BAW_INSTRUCTIONS_DELTA_16000,
	BAW_INSTRUCTIONS_DELTA_22050,
	BAW_INSTRUCTIONS_DELTA_32000,
	BAW_INSTRUCTIONS_DELTA_41000
};


uint8_t GetBawInstructionsDelta() {
	uint8_t index = GetBawSampleRateIndex();
	return pgm_read_byte(&BAW_INSTRUCTIONS_DELTA[index]);
}