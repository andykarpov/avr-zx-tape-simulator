#ifndef _DISPLAY_H_
#define _DISPLAY_H_


#include <stdbool.h>

#include "SDL2/SDL.h"

#include "stubs/avr/io.h"

#include "../lib/glcd/glcd.h"

#define GLCD_LCD_WIDTH 84
#define GLCD_LCD_HEIGHT 48


void glcd_init();
void glcd_set_contrast(uint8_t contrast);
void glcd_write();


bool key_down_pressed();

bool key_up_pressed();

bool key_left_pressed();

bool key_right_pressed();

bool key_enter_pressed() ;

bool key_any_pressed();


void initCwd();

#endif // _DISPLAY_H_