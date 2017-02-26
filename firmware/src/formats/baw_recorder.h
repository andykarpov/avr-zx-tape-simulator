/*
 * baw_recorder.h
 *
 * Created: 16.10.2015 23:11:13
 *  Author: Trol
 */ 


#ifndef BAW_RECORDER_H_
#define BAW_RECORDER_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>



bool BawSave(const char* fileName);
void BawSaveStart();
void BawRecorderNextSampleWithAutostop();
void BawRecorderNextSampleNoAutostop();
bool BawSaveStop(bool cancel);
bool BawSaveIsBegan();
void BawSaveCancel();

extern volatile uint16_t bawAutoStopCounter;


#endif // BAW_RECORDER_H_