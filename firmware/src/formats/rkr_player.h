/* 
 * File:   rkr_player.h
 * Author: trol
 *
 * Created on August 10, 2016, 2:41 PM
 */
#ifndef _RKR_PLAYER_H_
#define _RKR_PLAYER_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define RKR_STATUS_OK					0
#define RKR_STATUS_READ_ERROR			1
#define RKR_STATUS_FORMAT_ERROR		2
#define RKR_STATUS_CRC_ERROR			3


extern uint16_t rkr_start_address;			// стартовый адрес, инициализируется функцией прелоада
extern uint16_t rkr_end_address;				// конечный адрес, инициализируется функцией прелоада


uint8_t RkrFilePreload(const char *fileName);

bool RkrPlayFile(const char *fileName);

bool RkrPlayingInProgress();

// 0..100
uint8_t RkrGetPlayProgress();

void RkrClose();

#endif // _RKR_PLAYER_H_