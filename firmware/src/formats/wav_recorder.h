/*
 * wav_recorder.h
 *
 * Created: 11.10.2015 11:10:13
 *  Author: Trol
 */ 


#ifndef WAV_RECORDER_H_
#define WAV_RECORDER_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>



bool WavSave(const char* fileName);
void WavRecorderNextSample();
bool WavSaveStop();
bool WavSaveIsBegan();


#endif // WAV_RECORDER_H_