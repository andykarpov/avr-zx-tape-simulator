/*
 * baw_player.h
 *
 * Created: 16.10.2015 22:01:42
 *  Author: Trol
 */ 

#ifndef BAW_PLAYER_H_
#define BAW_PLAYER_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define BAW_PLAYER_IO_ERROR			0xfffe
#define BAW_PLAYER_FORMAT_ERROR		0xffff


#define PLAY_BAW_OK					0
#define PLAY_BAW_IO_ERROR			1
#define PLAY_BAW_WRONG_FORMAT		2


uint8_t playBaw(const char* fileName);

void BawPlayerNextSample();

bool BawPlayingInProgress();

uint16_t BawGetDuration(const char *fileName);

void closeBaw();

void BawLoadPause();
bool BawLoadResume();

bool BawUpdatePlayerTime();

void BawPlayerIdle(uint16_t tm);

#endif // BAW_PLAYER_H_ 