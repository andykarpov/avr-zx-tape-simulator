/*
 * settings.c
 *
 * Created: 14.06.2015 19:06:36
 *  Author: Trol
 */ 

#include "settings.h"

#include <avr/eeprom.h>

#include "debug.h"
#include "beeper.h"
#include "formats/baw.h"

#include "lib/glcd/glcd.h"
#include "simulator/stubs/avr/eeprom.h"

#define SOUND_KEY_MASK		1
#define SOUND_PLAY_MASK		2
#define SOUND_REC_MASK		4



#define OFFSET_SETTINGS_STRUCT			0
#define OFFSET_TM_PAUSE						(uint16_t*)4
#define OFFSET_TM_SHORT_PAUSE				(uint16_t*)6
#define OFFSET_TM_PILOT_LENGHT			(uint16_t*)8
#define OFFSET_TM_PILOT_PULSE				(uint16_t*)10
#define OFFSET_TM_SYNC1_PULSE				(uint16_t*)12
#define OFFSET_TM_SYNC2_PULSE				(uint16_t*)14
#define OFFSET_TM_DATA_PULSE				(uint16_t*)16
#define OFFSET_BASE_YEAR					(uint16_t*)18
#define OFFSET_DISPLAY_CONTRAST			(uint8_t*)20
#define OFFSET_BAW_RATE_INDEX				(uint8_t*)21
#define OFFSET_BAW_MIN_INTERVAL			(uint16_t*)22
#define OFFSET_BAW_AUTOSTOP_INTERVAL	(uint16_t*)24
#define OFFSET_RKR_BAUDRATE				(uint16_t*)26
#define OFFSET_RKR_PADDING_SIZE			(uint16_t*)28

//#define OFFSET_TIME_CORRECTION			(uint16_t*)20

uint8_t langAndSpeed;		// младшие 4 бита - код языка, старшие - код скорости
uint8_t soundAndHighlight;

struct settings_struct {
	unsigned lang		  : 2;	
	unsigned tapeSpeed  : 2;
	unsigned hightlight : 2;
	unsigned soundKey   : 1;
	unsigned soundPlay  : 1;
	unsigned soundRec   : 1;
	unsigned capture	  : 1;
	unsigned bawAutostop: 1;
};

struct settings_struct settings;

uint16_t pilotBlockLength;		// длина блока пилота, мс
uint16_t pilotPulseWidth;		// длина импульса пилота, мкс
uint16_t sync1PulseWidth;		// длина первого импульса синхросигнала, мкс
uint16_t sync2PulseWidth;		// длина второго импульса синхросигнала, мкс
uint16_t dataPulseWidth;		// длина 0-бита данных, мкс
uint16_t pauseLength;			// длина паузы между блоками, мс
uint16_t pauseShortLength;		// длина паузы между заголовком и блоком, мс
uint8_t displayContrast;		// контрастность экрана (0..128)

uint16_t baseYear;				// текущий год (с 2000го) считается как baseYear * 4 + год в pcf8535
//int16_t timeCorrection;			// коррекция времени (мс / 12 часов)

uint8_t bawSamplerateIndex;
uint16_t bawBlockInterval;
uint16_t bawAutoStopInterval;

uint16_t rkrBaudrate;
uint16_t rkrPadding;


const uint16_t BAW_SAMPLERATES[6] PROGMEM = {
	BAW_SAMPLE_RATE_1,
	BAW_SAMPLE_RATE_2,
	BAW_SAMPLE_RATE_3,
	BAW_SAMPLE_RATE_4,
	BAW_SAMPLE_RATE_5,
	BAW_SAMPLE_RATE_6
};

uint8_t GetLang() {
	return settings.lang;
}


void SetLang(uint8_t lang) {
	settings.lang = lang;
}


uint8_t GetTapSpeed() {
	return settings.tapeSpeed;
}


void SetTapSpeed(uint8_t speed) {
	settings.tapeSpeed = speed;
}


bool GetSoundKey() {
	return settings.soundKey;
}


void SetSoundKey(bool on) {
	settings.soundKey = on;
}


bool GetSoundPlay() {
	return settings.soundPlay;
}


void SetSoundPlay(bool on) {
	settings.soundPlay = on;
}


bool GetSoundRec() {
	return settings.soundRec;
}


void SetSoundRec(bool on) {
	settings.soundRec = on;
}


uint8_t GetHighlight() {
	return settings.hightlight;
}


void SetHighlight(uint8_t on) {
	settings.hightlight = on;
	if (GetHighlight() == HIGHLIGHT_ON) {
		highlight_on();
	} else if (GetHighlight() == HIGHLIGHT_OFF) {
		highlight_off();
	}
}


uint16_t GetPilotBlockDuration() {
	return pilotBlockLength;		// длина блока пилота, мс
}


void SetPilotBlockDuration(uint16_t v) {
	pilotBlockLength = v;
}


uint16_t GetPauseDuration() {
	return pauseLength;	
}


void SetPauseDuration(uint16_t v) {
	pauseLength = v;
}

uint16_t GetShortPauseDuration() {
	return pauseShortLength;
}

void SetShortPauseDuration(uint16_t v) {
	pauseShortLength = v;
}

uint16_t GetPilotPulseWidth() {
	return pilotPulseWidth;		// длина импульса пилота, мкс
}


void SetPilotPulseWidth(uint16_t v) {
	pilotPulseWidth	= v;
}


uint16_t GetSync1PulseWidth() {
	return sync1PulseWidth;		// длина первого импульса синхросигнала, мкс
}


void SetSync1PulseWidth(uint16_t v) {
	sync1PulseWidth = v;
}


uint16_t GetSync2PulseWidth() {
	return sync2PulseWidth;		// длина второго импульса синхросигнала, мкс
}


void SetSync2PulseWidth(uint16_t v) {
	sync2PulseWidth = v;
}


uint16_t GetDataPulseWidth() {
	return dataPulseWidth;		// длина 0-бита данных, мкс
}


void SetDataPulseWidth(uint16_t v) {
	dataPulseWidth = v;
}

uint8_t GetCapture() {
	return settings.capture;
}


void SetCapture(uint8_t capture) {
	settings.capture = capture;
}

uint16_t GetBaseYear() {
	return baseYear;
}


void SetBaseYear(uint16_t v) {
	baseYear = v;
}


//int16_t GetTimeCorrection() {
//	return timeCorrection;
//}

//void SetTimeCorrection(uint16_t val) {
//	timeCorrection = val;
//}

void SetDisplayContrast(uint8_t v) {
	displayContrast = v;
	glcd_set_contrast(displayContrast);
}

uint8_t GetDisplayContrast() {
	return displayContrast;
}



uint8_t GetBawSampleRateIndex() {	
	return bawSamplerateIndex;
}

void SetBawSampleRateIndex(uint8_t v) {
	bawSamplerateIndex = v;
}

uint16_t GetBawSampleRate() {
	return pgm_read_word(&BAW_SAMPLERATES[bawSamplerateIndex]);
}

uint16_t GetBawSampleRateForIndex(uint8_t index) {
	return pgm_read_word(&BAW_SAMPLERATES[index]);
}

uint16_t GetBawMinBlockInterval() {
	return bawBlockInterval;
}

void SetBawMinBlockInterval(uint16_t v) {
	bawBlockInterval = v;
}

uint16_t GetBawAutoStopInterval() {
	return bawAutoStopInterval;
}

void SetBawAutoStopInterval(uint16_t v) {
	bawAutoStopInterval = v;
}

bool GetBawAutoStop() {
	return settings.bawAutostop;
}

void SetBawAutoStop(bool v) {
	settings.bawAutostop = v;
}

uint16_t GetRkrBaudrate() {
	return rkrBaudrate;
}

void SetRkrBaudrate(uint16_t v) {
	rkrBaudrate = v;
}

uint16_t GetRkrPadding() {
	return rkrPadding;
}

void SetRkrPadding(uint16_t v) {
	rkrPadding = v;
}


void SetupDefaultTimings() {
	SetPauseDuration(DEFAULT_PAUSE_DURATION);
	SetShortPauseDuration(DEFAULT_SHORT_PAUSE_DURATION);
	SetPilotBlockDuration(DEFAULT_PILOT_BLOCK_DURATION);
	SetPilotPulseWidth(DEFAULT_PILOT_WIDTH);
	SetSync1PulseWidth(DEFAULT_SYNC1_WIDTH);
	SetSync2PulseWidth(DEFAULT_SYNC2_WIDTH);
	SetDataPulseWidth(DEFAULT_DATA_WIDTH);
	SetCapture(CAPTURE_FALL);
	SetBaseYear(DEFAULT_BASE_YEAR);
//	SetTimeCorrection(0);
	displayContrast = DEFAULT_DSPLAY_CONTRAST;
	SetBawSampleRateIndex(DEFAULT_BAW_SAMPLERATE_INDEX);
	SetBawMinBlockInterval(DEFAULT_BAW_BLOCK_INTERVAL);
	SetBawAutoStopInterval(DEFAULT_BAW_AUTO_STOP_INTERVAL);
	SetRkrBaudrate(DEFAULT_RKR_BAUDRATE);
	SetRkrPadding(DEFAULT_RKR_PADDING_SIZE);
}


static uint16_t getUint16Val(uint16_t* offset, uint16_t min, uint16_t max, uint16_t defaultValue) {
	uint16_t result = eeprom_read_word(offset);
	if (result < min) {
		return defaultValue;
	}
	if (result > max) {
		return defaultValue;
	}
	return result;
}

//static int16_t getInt16Val(uint16_t* offset, int16_t min, int16_t max, int16_t defaultValue) {
//	int16_t result = eeprom_read_word(offset);
//	if (result < min) {
//		return defaultValue;
//	}
//	if (result > max) {
//		return defaultValue;
//	}
//	return result;
//}

static uint8_t getUint8Val(uint8_t* offset, uint8_t min, uint8_t max, uint8_t defaultValue) {
	uint8_t result = eeprom_read_byte(offset);
	if (result < min) {
		return defaultValue;
	}
	if (result > max) {
		return defaultValue;
	}
	return result;
}

void SettingLoad() {
	bool dataCorrupt = false;	

	eeprom_read_block(&settings, OFFSET_SETTINGS_STRUCT, sizeof(settings));
	
	if (settings.lang > LANG_RU) {
		dataCorrupt = true;
	}
	if (settings.tapeSpeed > SPEED_8X) {
		dataCorrupt = true;
	}
	if (settings.hightlight > HIGHLIGHT_AUTO) {
		dataCorrupt = true;
	}

	if (dataCorrupt) {
		SetupDefaultTimings();
		SetTapSpeed(SPEED_1X);
		SetHighlight(HIGHLIGHT_AUTO);
		SetLang(LANG_EN);
	}
	SetHighlight(GetHighlight());

	pauseLength = getUint16Val(OFFSET_TM_PAUSE, 500, 10000, DEFAULT_PAUSE_DURATION);
	pauseShortLength = getUint16Val(OFFSET_TM_SHORT_PAUSE, 500, 10000, DEFAULT_SHORT_PAUSE_DURATION);
	pilotBlockLength = getUint16Val(OFFSET_TM_PILOT_LENGHT, 500, 5000, DEFAULT_PILOT_BLOCK_DURATION);	
	pilotPulseWidth = getUint16Val(OFFSET_TM_PILOT_PULSE, 50, 2000, DEFAULT_PILOT_WIDTH);
	sync1PulseWidth = getUint16Val(OFFSET_TM_SYNC1_PULSE, 50, 1000, DEFAULT_SYNC1_WIDTH);
	sync2PulseWidth = getUint16Val(OFFSET_TM_SYNC2_PULSE, 50, 1000, DEFAULT_SYNC2_WIDTH);
	dataPulseWidth = getUint16Val(OFFSET_TM_DATA_PULSE, 50, 1000, DEFAULT_DATA_WIDTH);

	baseYear = getUint16Val(OFFSET_BASE_YEAR, 2000, 2100, DEFAULT_BASE_YEAR);
//	timeCorrection = getInt16Val(OFFSET_TIME_CORRECTION, -30000, 30000, 0);
	
	displayContrast = getUint8Val(OFFSET_DISPLAY_CONTRAST, 0, 128, DEFAULT_DSPLAY_CONTRAST);
	
	bawSamplerateIndex = getUint8Val(OFFSET_BAW_RATE_INDEX, 0, 5, DEFAULT_BAW_SAMPLERATE_INDEX);
	bawBlockInterval = getUint16Val(OFFSET_BAW_MIN_INTERVAL, 100, 3000, DEFAULT_BAW_BLOCK_INTERVAL);
	bawAutoStopInterval = getUint16Val(OFFSET_BAW_AUTOSTOP_INTERVAL, 1000, 20000, DEFAULT_BAW_AUTO_STOP_INTERVAL);
	rkrBaudrate = getUint16Val(OFFSET_RKR_BAUDRATE, 130, 60000, DEFAULT_RKR_BAUDRATE);
	rkrPadding = getUint16Val(OFFSET_RKR_PADDING_SIZE, 0, 1000, DEFAULT_RKR_PADDING_SIZE);
}


void SettingSave() {
	MSG("save");
	eeprom_update_block(&settings, OFFSET_SETTINGS_STRUCT, sizeof(settings));
	eeprom_update_word(OFFSET_TM_PAUSE, pauseLength);
	eeprom_update_word(OFFSET_TM_SHORT_PAUSE, pauseShortLength);
	eeprom_update_word(OFFSET_TM_PILOT_LENGHT, pilotBlockLength);
	eeprom_update_word(OFFSET_TM_PILOT_PULSE, pilotPulseWidth);
	eeprom_update_word(OFFSET_TM_SYNC1_PULSE, sync1PulseWidth);
	eeprom_update_word(OFFSET_TM_SYNC2_PULSE, sync2PulseWidth);
	eeprom_update_word(OFFSET_TM_DATA_PULSE, dataPulseWidth);
//	eeprom_update_word(OFFSET_TIME_CORRECTION, timeCorrection);
	eeprom_update_byte(OFFSET_DISPLAY_CONTRAST, displayContrast);
	eeprom_update_byte(OFFSET_BAW_RATE_INDEX, bawSamplerateIndex);
	eeprom_update_word(OFFSET_BAW_MIN_INTERVAL, bawBlockInterval);
	eeprom_update_word(OFFSET_BAW_AUTOSTOP_INTERVAL, bawAutoStopInterval);
	eeprom_update_word(OFFSET_RKR_BAUDRATE, rkrBaudrate);
	eeprom_update_word(OFFSET_RKR_PADDING_SIZE, rkrPadding);
}