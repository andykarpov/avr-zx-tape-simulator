/*
 * settings.h
 *
 * Created: 14.06.2015 19:06:26
 *  Author: Trol
 */ 


#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#include <stdint.h>
#include <stdbool.h>

#define LANG_EN			0
#define LANG_RU			1

#define SPEED_1X		0
#define SPEED_2X		1
#define SPEED_4X		2
#define SPEED_8X		3

#define HIGHLIGHT_OFF		0
#define HIGHLIGHT_ON			1
#define HIGHLIGHT_AUTO		2

#define CAPTURE_FALL		0
#define CAPTURE_RISE		1


#define DEFAULT_PAUSE_DURATION			3500
#define DEFAULT_SHORT_PAUSE_DURATION	1000
#define DEFAULT_PILOT_BLOCK_DURATION	1700
#define DEFAULT_PILOT_WIDTH				619
#define DEFAULT_SYNC1_WIDTH				190
#define DEFAULT_SYNC2_WIDTH				201
#define DEFAULT_DATA_WIDTH					244

#define DEFAULT_BASE_YEAR					2015

#define DEFAULT_DSPLAY_CONTRAST			80

#define DEFAULT_BAW_SAMPLERATE_INDEX	5
#define DEFAULT_BAW_BLOCK_INTERVAL		200
#define DEFAULT_BAW_AUTO_STOP_INTERVAL	5000

#define DEFAULT_RKR_BAUDRATE				1300
#define DEFAULT_RKR_PADDING_SIZE			0x100


uint8_t GetLang();
void SetLang(uint8_t lang);
uint8_t GetTapSpeed();
void SetTapSpeed(uint8_t speed);
bool GetSoundKey();
void SetSoundKey(bool on);
bool GetSoundPlay();
void SetSoundPlay(bool on);
bool GetSoundRec();
void SetSoundRec(bool on);
uint8_t GetHighlight();
void SetHighlight(uint8_t on);

uint16_t GetPilotBlockDuration();
void SetPilotBlockDuration(uint16_t v);
uint16_t GetPauseDuration();
void SetPauseDuration(uint16_t v);
uint16_t GetShortPauseDuration();
void SetShortPauseDuration(uint16_t v);
uint16_t GetPilotPulseWidth();
void SetPilotPulseWidth(uint16_t v);
uint16_t GetSync1PulseWidth();
void SetSync1PulseWidth(uint16_t v);
uint16_t GetSync2PulseWidth();
void SetSync2PulseWidth(uint16_t v);
uint16_t GetDataPulseWidth();
void SetDataPulseWidth(uint16_t v);	
uint8_t GetCapture();
void SetCapture(uint8_t capture);

uint8_t GetBawSampleRateIndex();
void SetBawSampleRateIndex(uint8_t v);
uint16_t GetBawSampleRate();
uint16_t GetBawSampleRateForIndex(uint8_t index);
uint16_t GetBawMinBlockInterval();
void SetBawMinBlockInterval(uint16_t v);
uint16_t GetBawAutoStopInterval();
void SetBawAutoStopInterval(uint16_t v);
bool GetBawAutoStop();
void SetBawAutoStop(bool v);

uint16_t GetRkrBaudrate();
void SetRkrBaudrate(uint16_t v);
uint16_t GetRkrPadding();
void SetRkrPadding(uint16_t v);

uint16_t GetBaseYear();
void SetBaseYear(uint16_t v);


void SetupDefaultTimings();


void SetDisplayContrast(uint8_t v);
uint8_t GetDisplayContrast();

void SettingLoad();
void SettingSave();


#endif // _SETTINGS_H_