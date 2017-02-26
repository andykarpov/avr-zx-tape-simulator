/*
 * ui_draw.h
 *
 * Created: 05.07.2015 21:20:44
 *  Author: Trol
 */ 


#include "ui_utils.h"

#ifndef UI_DRAW_H_
#define UI_DRAW_H_


static void drawItemSelector() {
	glcd_invert_area(2, selectedIndex*LINES_DY, GLCD_LCD_WIDTH-2, LINES_DY);
}

/************************************************************************/
/* Рисование экранов                                                    */
/************************************************************************/
// Экран главного меню
static void drawMainMenuScreen() {
	glcd_drawCenteredStr_p(i18n(STR_PLAY), 1, 1);
	glcd_drawCenteredStr_p(i18n(STR_RECORD), 1+LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_SETTINGS), 1+2*LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_SD_INFO), 1+3*LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_ABOUT), 1+4*LINES_DY, 1);
	
	// выделяем текущий
	if (GetLang() == LANG_EN) {
		glcd_invert_area(15, selectedIndex*LINES_DY, GLCD_LCD_WIDTH-30, LINES_DY);
	} else {
		glcd_invert_area(10, selectedIndex*LINES_DY, GLCD_LCD_WIDTH-20, LINES_DY);
	}
// TODO временно отображаем тут код ошибки ввода-вывода
if (FioGetErrorMask() != 0) {
	glcd_draw_uint16(0, 0, FioGetErrorMask());
	glcd_draw_uint16(0, 10, FioGetErrorCount());
}
}



// Экран информации
static void drawAboutScreen() {	
	glcd_drawCenteredStr_p(STR_APP_NAME, 0+2, 1);
	glcd_drawCenteredStr_p(STR_APP_VERSION, LINES_DY+2, 0);
	glcd_drawCenteredStr_p(STR_APP_COPYRIGHT, 2*LINES_DY+2, 0);
	glcd_drawCenteredStr_p(STR_APP_YEAR, 3*LINES_DY+2, 1);
	glcd_drawCenteredStr_p(STR_APP_WEB, 4*LINES_DY+2, 0);
}


static void drawScrollBar(uint32_t totalItems, uint32_t pos, uint8_t x, uint8_t width) {
	uint16_t scrollBarHeight_ = GLCD_LCD_HEIGHT * LINES_PER_SCREEN / totalItems;
	uint8_t scrollBarHeight = (uint8_t)scrollBarHeight_;
	if (scrollBarHeight < 3) {
		scrollBarHeight = 3;
	}

	// позиция скроллера
	//uint32_t scrolledFiles = pos + firstVisibleItem;
	uint32_t scrollBarPos_ = GLCD_LCD_HEIGHT * pos / totalItems;
	uint8_t scrollBarPos = (uint8_t)scrollBarPos_;
	if (scrollBarPos + scrollBarHeight > GLCD_LCD_HEIGHT) {
		scrollBarPos = GLCD_LCD_HEIGHT - scrollBarHeight;
	}
	
	glcd_fill_rect(x, scrollBarPos, width, scrollBarHeight, 1);
}

// Экран файлового браузера
static void drawBrowserScreen() {
	// выводим список файлов и директорий
	uint16_t index = 0;
	uint8_t x = 1;
	uint8_t y = 1;
	uint8_t nameLenCnt = 0;	// сколько символов имени было отрисовано
//	bool isDirectory = false;
	if (FioGetDirData()->dirSize == 0) {
		glcd_drawCenteredStr_p(i18n(STR_EMPTY), 0, 1);
		return;
	}
	for (uint16_t pos = 0; pos < DIR_DATA_SIZE; pos++) {
		char ch = GetDirectoryData()[pos];
//		if (nameLenCnt == 0) {
//			isDirectory = ch == '>';
//		}
		// обрезаем расширения у файлов и имена файлов до 12 символов
		//if ((!isDirectory && ch == '.') || nameLenCnt >= MAX_FILE_NAME_DISPLAY_LEN) {
		if (nameLenCnt >= MAX_FILE_NAME_DISPLAY_LEN) {
			// пропускаем концовку имени файла/расширение
			while (GetDirectoryData()[pos]) pos++;
			pos--;
			nameLenCnt = 0;	// чтобы снова не попасть на это условие
			continue;
		}
		if (ch == 0) {
			index++;
			if (index >= FioGetDirData()->dirSize) {
				break;
			}
			// заполнили все строки экрана
			if (index >= firstVisibleItem + LINES_PER_SCREEN) {
				break;
			}
			if (index > firstVisibleItem) {
				y += LINES_DY;
				x = 1;
				nameLenCnt = 0;
			}
		}
		// добрались до водимых строк
		if (ch != 0 && index >= firstVisibleItem) {
			x += glcd_draw_char_xy(x, y, ch);
			x++;
			nameLenCnt++;
		}
		
	}

	// выделяем текущий
	glcd_invert_area(0, selectedIndex*LINES_DY, GLCD_LCD_WIDTH-7, LINES_DY);
	// рисуем скроллбар
	if (FioGetDirData()->dirSize > LINES_PER_SCREEN) {
		glcd_fill_rect(GLCD_LCD_WIDTH-3, 0, 3, GLCD_LCD_HEIGHT, 0);
		drawScrollBar(FioGetDirData()->dirSize, FioGetDirData()->offset + firstVisibleItem, GLCD_LCD_WIDTH-2, 2);
	}

}

// Рисует общую часть экрана плеера - имя файла, индикатор прогресса, время проигрывания и паузу
void drawPlayerProgress(bool stillLoading, uint8_t progress) {
	// имя файла
	glcd_drawCenteredStr(currentFileName, 0, 1);

	// индикатор прогресса
	glcd_draw_rect(0, LINES_DY+1, GLCD_LCD_WIDTH, 7, 1);

	glcd_fill_rect(2, LINES_DY+3, progress, 3, 1);

	if (pause_mode) {
		glcd_drawCenteredStr_p(i18n(STR_PAUSE), 3*LINES_DY + 2, 1);
	} else {
		// прошедшее время
		// если файл в процессе загрузки
		uint8_t min, sec;
		if (stillLoading) {
			min = GetDateTime()->min;
			sec = GetDateTime()->sec;
			// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
			GetDateTime()->year = min;
			GetDateTime()->month = sec;
		} else {
			min = GetDateTime()->year;
			sec = GetDateTime()->month;
		}
		uint8_t x = (GLCD_LCD_WIDTH - 5*5) / 2;
		x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, min);
		x += glcd_draw_char_xy(x, 3*LINES_DY + 2, ':');
		glcd_draw_uint8_2(x, 3*LINES_DY + 2, sec);
	}
}


// Экран загрузки TAP файла
static void drawPlayTapScreen() {
	uint8_t x;
	
	// имя файла
//	glcd_drawCenteredStr(currentFileName, 0, 1);
	// индикатор прогресса
//	glcd_draw_rect(0, LINES_DY+1, GLCD_LCD_WIDTH, 7, 1);
//	uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * TapGetProcessedBytes() / fileSize;
		
//	glcd_fill_rect(2, LINES_DY+3, (uint8_t)progressWidth, 3, 1);
	
	// номер текущего блока
	if (currentBlockNumber > 0) {
		x = glcd_draw_string_xy_P(0, 2*LINES_DY+1, i18n(STR_BLOCK)) + 5;
		
		bool drawBlock;
		//if (TapLoadIsPaused()) {
		if (pause_mode) {
			// мигающий курсор
			uint8_t hsec = GetDateTime()->hsec;
			drawBlock = hsec < 25 || (hsec > 50 && hsec < 75);
		} else {
			drawBlock = true;
		}
		if (drawBlock) {
			x += glcd_draw_uint16(x, 2*LINES_DY+1, currentBlockNumber) + 2;
		} else {
			if (currentBlockNumber < 10) {
				x += 6 + 2;
			} else if (currentBlockNumber < 100) {
				x += 6*2 + 2;
			} else {
				x += 6*3 + 2;
			}
		}
		x += glcd_draw_char_xy(x, 2*LINES_DY+1, '/') + 2;
		glcd_draw_uint16(x, 2*LINES_DY+1, numberOfBlocks);
	}
	
	uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * TapGetProcessedBytes() / fileSize;
	drawPlayerProgress(TapGetProcessedBytes() < fileSize, (uint8_t)progressWidth);
	
//	if (TapLoadIsPaused()) {
//		glcd_drawCenteredStr_p(i18n(STR_PAUSE), 3*LINES_DY + 2, 1);
//	} else {
//		// прошедшее время
//		// если файл в процессе загрузки
//		uint8_t min, sec;
//		if (TapGetProcessedBytes() < fileSize) {
//			min = GetDateTime()->min;
//			sec = GetDateTime()->sec;
//			// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
//			GetDateTime()->year = min;
//			GetDateTime()->month = sec;
//		} else {
//			min = GetDateTime()->year;
//			sec = GetDateTime()->month;
//		}
//		x = (GLCD_LCD_WIDTH - 5*5) / 2;
//		x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, min);
//		x += glcd_draw_char_xy(x, 3*LINES_DY + 2, ':');
//		x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, sec);
//	}

	// подсказки к кнопкам
	if (TapGetProcessedBytes() < fileSize) {
		drawLeftKey(i18n(STR_STOP));
		if (TapLoadIsPaused()) {
			drawRightKey(i18n(STR_RESUME));
		} else {
			drawRightKey(i18n(STR_PAUSE));
		}
	} else {
		glcd_drawCenteredStr_p(i18n(STR_DONE), GLCD_LCD_HEIGHT - LINES_DY, 1);
	}
}



static void drawWrongTapFileScreen() {
	uint8_t x;
	
	// имя файла
	glcd_draw_string_xy(0, 0, currentFileName);
	glcd_draw_string_xy_P(0, LINES_DY, i18n(STR_FILE_CORRUPTED));
	// Номер блока если это ошибка CRC
	if (tapInfo.status == TAP_VERIFY_STATUS_CRC_ERROR) {
		glcd_draw_string_xy_P(0, 2*LINES_DY+2, i18n(STR_BLOCK));
		x = 6*5+5;
		x += glcd_draw_uint16(x, 2*LINES_DY+2, tapInfo.firstInvalidBlock) + 2;
		x += glcd_draw_char_xy(x, 2*LINES_DY+2, '/') + 2;
		glcd_draw_uint16(x, 2*LINES_DY+2, numberOfBlocks);
	}
}


static void drawWaitTapeScreen() {
	glcd_drawCenteredStr_p(i18n(STR_WAITING_SIGNAL_1), LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_WAITING_SIGNAL_2), 2*LINES_DY, 1);
	drawLeftKey(i18n(STR_CANCEL));
	if (selectedSaveFormat == FILE_BAW) {
		drawRightKey(i18n(STR_FORCE));
	}
}


static void drawTapSaveScreen() {
	uint8_t x;
	
	TapSaveOnIdle();
	glcd_drawCenteredStr(currentFileName, 0, 1);
	uint8_t block = TapSaverGetProgress()->block;
	x = glcd_draw_string_xy_P(0, LINES_DY, i18n(STR_BLOCK)) + 5;
	
	if (block == 1) {
		x += glcd_draw_uint16(x, LINES_DY, block);
		x += 5;
		if (TapSaverGetStatus() == TAPE_IN_PILOT) {
			glcd_draw_string_xy_P(x, LINES_DY, i18n(STR_PILOT));
			} else {
			x += glcd_draw_uint16(x, LINES_DY, TapSaverGetProgress()->lastBlockSize[0]) +2;
			glcd_draw_string_xy_P(x, LINES_DY, i18n(STR_BYTE_SHORT));
		}
	} else {
		// предыдущий блок
		x += glcd_draw_uint16(x, LINES_DY, block-1);
		x += 5;
		x += glcd_draw_uint16(x, LINES_DY, TapSaverGetProgress()->lastBlockSize[1]) + 2;
		glcd_draw_string_xy_P(x, LINES_DY, i18n(STR_BYTE_SHORT));
		
		// текущий блок
		x = glcd_draw_string_xy_P(0, 2*LINES_DY, i18n(STR_BLOCK)) + 5;
		
		x += glcd_draw_uint16(x, 2*LINES_DY, block);
		x += 5;
		if (TapSaverGetStatus() == TAPE_IN_PILOT) {
			glcd_draw_string_xy_P(x, 2*LINES_DY, i18n(STR_PILOT));
		} else {
			x += glcd_draw_uint16(x, 2*LINES_DY, TapSaverGetProgress()->lastBlockSize[0]) + 2;
			glcd_draw_string_xy_P(x, 2*LINES_DY, i18n(STR_BYTE_SHORT));
		}
	}
	drawLeftKey(i18n(STR_STOP));
	TapSaveOnIdle();
}


static void drawTapSaveDoneScreen() {
	uint8_t x;
	
	glcd_drawCenteredStr(currentFileName, 0, 1);
	x = glcd_draw_string_xy_P(0, LINES_DY, i18n(STR_BLOCKS)) + 5;
	glcd_draw_uint16(x, LINES_DY, TapSaverGetProgress()->block);
	
	x = glcd_draw_uint32(0, 2*LINES_DY, fileSize);
	x += 5;
	glcd_draw_string_xy_P(x, 2*LINES_DY, i18n(STR_BYTES));
	
	if (tapInfo.status == TAP_VERIFY_STATUS_FILE_CORRUPTED) {
		glcd_draw_string_xy_P(0, 3*LINES_DY, i18n(STR_FILE_CORRUPTED));
	} else if (tapInfo.status == TAP_VERIFY_STATUS_CRC_ERROR) {
		x = glcd_draw_string_xy_P(0, 3*LINES_DY, i18n(STR_CRC_ERROR));
		x += glcd_draw_char_xy(x, 3*LINES_DY, ':') + 5;
		x += glcd_draw_uint16(x, 3*LINES_DY, tapInfo.firstInvalidBlock) + 2;
	}
	
	drawLeftKey(i18n(STR_BACK));
}


static void drawSettingsItem(uint8_t num, const char *name, const char *value) {
	if (num >= firstVisibleItem && num < firstVisibleItem+5) {
		uint8_t y = 1 + (num - firstVisibleItem) * LINES_DY;
		glcd_draw_string_xy_P(3, y, name);
		
		uint8_t valWidth = strlen_P(value)*6;
		glcd_draw_string_xy_P(GLCD_LCD_WIDTH - valWidth, y, value);
	}
}


static const char* getHighlightValStr() {
	switch (GetHighlight()) {
		case HIGHLIGHT_ON:
			return i18n(STR_ON);
		case HIGHLIGHT_OFF:
			return i18n(STR_OFF);
	}
	return i18n(STR_AUTO);
}


static const char* getSpeedStr() {
	switch (GetTapSpeed()) {
		case SPEED_2X:
			return STR_X2;
		case SPEED_4X:
			return STR_X4;
		case SPEED_8X:
			return STR_X8;
	}
	return STR_X1;
}


static void drawSettingsScreen() {
	drawSettingsItem(SETTINGS_ITEM_LANG, i18n(STR_LANGUAGE), GetLang() == LANG_EN ? i18n(STR_LANG_EN) : i18n(STR_LANG_RU));
	drawSettingsItem(SETTINGS_ITEM_SOUND_KEY, i18n(STR_SOUND_KEY), GetSoundKey() ? i18n(STR_ON) : i18n(STR_OFF));
	drawSettingsItem(SETTINGS_ITEM_SOUND_PLAY, i18n(STR_SOUND_PLAY), GetSoundPlay() ? i18n(STR_ON) : i18n(STR_OFF));
	drawSettingsItem(SETTINGS_ITEM_SOUND_REC, i18n(STR_SOUND_REC), GetSoundRec() ? i18n(STR_ON) : i18n(STR_OFF));
	drawSettingsItem(SETTINGS_ITEM_BACKLIGHT, i18n(STR_BACKLIGHT), getHighlightValStr());
	drawSettingsItem(SETTINGS_ITEM_SPEED, i18n(STR_SPEED), getSpeedStr());
	drawSettingsItem(SETTINGS_ITEM_TAP_TIMES, i18n(STR_TIMINGS_TAP), STR_MENU_CHAR);
#ifdef SETTINGS_ITEM_BAW_SETTINGS	
	drawSettingsItem(SETTINGS_ITEM_BAW_SETTINGS, i18n(STR_SETTINGS_BAW), STR_MENU_CHAR);
#endif
#ifdef SETTINGS_ITEM_RKR_SETTINGS	
	drawSettingsItem(SETTINGS_ITEM_RKR_SETTINGS, i18n(STR_SETTINGS_RKR), STR_MENU_CHAR);
#endif
	drawSettingsItem(SETTINGS_ITEM_DATETIME, i18n(STR_DATE_TIME), STR_MENU_CHAR);
	drawSettingsItem(SETTINGS_ITEM_CONTRAST, i18n(STR_DISPLAY_CONTRAST), STR_MENU_CHAR);

	// выделяем текущий
	drawItemSelector();
	
	glcd_fill_rect(0, 0, 2, GLCD_LCD_HEIGHT, 0);
	drawScrollBar(SETTINGS_ITEM_LAST+1+4, firstVisibleItem + selectedIndex, 0, 1);
}


// возвращает значение, отображаемое для выбранного поля таймингов
static uint16_t getTimingValue(uint8_t index) {
	if (screen == SCREEN_TAP_SETTINGS) {
		if (editMode && index == firstVisibleItem + selectedIndex) {
			return editedValue;
		}
		switch (index) {
			case 0:
				return GetPauseDuration();
			case 1:
				return GetShortPauseDuration();			
			case 2:
				return GetPilotBlockDuration();
			case 3:
				return GetPilotPulseWidth();
			case 4:
				return GetSync1PulseWidth();
			case 5:
				return GetSync2PulseWidth();
			case 6:
				return GetDataPulseWidth();
		}
	} else if (screen == SCREEN_BAW_SETTINGS) {
		if (editMode && index == firstVisibleItem + selectedIndex) {
			if (index > 0) {
				return editedValue;
			} else {
				return GetBawSampleRateForIndex(editedValue);
			}
		}
		switch (index) {
			case 0:
				return GetBawSampleRate();
			case 1:
				return GetBawMinBlockInterval();
			case 3:
				return GetBawAutoStopInterval();
		}
	} else if (screen == SCREEN_RKR_SETTINGS) {
		if (editMode && index == firstVisibleItem + selectedIndex) {
			return editedValue;
		}
		switch (index) {
			case 0:
				return GetRkrBaudrate();
			case 1:
				return GetRkrPadding();
		}
	}
	return 0;
}

// устанавливает значение для указанной настройки таймингов
static void setTimingValue(uint8_t index, uint16_t val) {
	if (screen == SCREEN_TAP_SETTINGS) {
		switch (index) {
			case 0:
				return SetPauseDuration(val);
			case 1:
				return SetShortPauseDuration(val);			
			case 2:
				return SetPilotBlockDuration(val);
			case 3:
				return SetPilotPulseWidth(val);
			case 4:
				return SetSync1PulseWidth(val);
			case 5:
				return SetSync2PulseWidth(val);
			case 6:
				return SetDataPulseWidth(val);
		}
	} else if (screen == SCREEN_BAW_SETTINGS) {
		switch (index) {
			case 0:	
				SetBawSampleRateIndex(val);
				break;
			case 1:
				SetBawMinBlockInterval(val);
				break;
			case 3:
				SetBawAutoStopInterval(val);
				break;
		}
	} else if (screen == SCREEN_RKR_SETTINGS) {
		switch (index) {
			case 0:
				SetRkrBaudrate(val);
				break;
			case 1:
				SetRkrPadding(val);
				break;
		}
	}
}

// отображает целочисленный параметр настроек
static void drawSettingsValueItem(uint8_t num, const char *name, const char *unit) {
	if (num >= firstVisibleItem && num < firstVisibleItem+5) {
		uint8_t y = 1 + (num - firstVisibleItem) * LINES_DY;
		glcd_draw_string_xy_P(3, y, name);
		
		uint16_t val = getTimingValue(num);
		uint8_t valWidth = glcd_getUint16digits(val)*6 + 2;
		if (unit) {
			valWidth += strlen_P(unit)*6;
		}
		uint8_t x = GLCD_LCD_WIDTH - valWidth;
		x += glcd_draw_uint16(x, y, val) + 2;
		if (unit) {
			glcd_draw_string_xy_P(x, y, unit);
		}
		
		if (editMode && num == firstVisibleItem + selectedIndex) {
			glcd_draw_string_xy_P(GLCD_LCD_WIDTH - valWidth - 1 - 5, selectedIndex*LINES_DY+1, STR_VALUE_EDIOR_CHAR);
		}
	}
}

// отображает параметр настроек для времени в секундах
// msd = 1 -> '12.3 с' 
// msd = 2 -> '12.31 с' 
static void drawSettingsValueTimeItem(uint8_t num, const char *name, uint8_t msd) {
	if (num >= firstVisibleItem && num < firstVisibleItem+5) {
		uint8_t y = 1 + (num - firstVisibleItem) * LINES_DY;
		glcd_draw_string_xy_P(3, y, name);
		
		uint16_t val = getTimingValue(num);
		uint16_t sec = val / 1000;
		uint16_t milis = val % 1000;
		uint8_t valWidth = (3 + (sec < 10 ? 0 : 1) + msd)*6 + 2 - 1;
		uint8_t x = GLCD_LCD_WIDTH - valWidth;
		x += glcd_draw_uint16(x, y, sec);
		x += glcd_draw_char_xy(x, y, '.');
		if (msd == 2) {
			milis /= 10;
		} else {
			milis /= 100;
		}
		x += glcd_draw_uint16(x, y, milis);
		if (msd == 2 && milis == 0) {
			x += glcd_draw_char_xy(x, y, '0');
		}
		x += 2;
		glcd_draw_string_xy_P(x, y, i18n(STR_SEC));
		
		if (editMode && num == firstVisibleItem + selectedIndex) {
			glcd_draw_string_xy_P(GLCD_LCD_WIDTH - valWidth - 1 - 5, selectedIndex*LINES_DY+1, STR_VALUE_EDIOR_CHAR);
		}
	}
}




static void drawTapTimesSetupScreen() {
	drawSettingsValueTimeItem(0, i18n(STR_PAUSE), 2);
	drawSettingsValueTimeItem(1, i18n(STR_PAUSE_SHORT), 2);
	drawSettingsValueTimeItem(2, i18n(STR_PILOT_BLOCK_LEN), 2);
	drawSettingsValueItem(3, i18n(STR_PILOT_LEN), i18n(STR_US));
	drawSettingsValueItem(4, i18n(STR_SYNC_1), i18n(STR_US));
	drawSettingsValueItem(5, i18n(STR_SYNC_2), i18n(STR_US));
	drawSettingsValueItem(6, i18n(STR_DATA), i18n(STR_US));
	drawSettingsItem(7, i18n(STR_CAPTURE), GetCapture() == CAPTURE_RISE ? i18n(STR_RISE) : i18n(STR_FALL));
	drawSettingsItem(8, i18n(STR_DETECT), STR_MENU_CHAR);
	drawSettingsItem(9, i18n(STR_DEFAULT), STR_MENU_CHAR);

	// выделяем текущий
//	if (!editMode) {
	drawItemSelector();
//	}
	
	glcd_fill_rect(0, 0, 2, GLCD_LCD_HEIGHT, 0);
	if (!editMode) {
		drawScrollBar(9+4, firstVisibleItem + selectedIndex, 0, 1);
	}
}


static void drawBawSettingsScreen() {
	drawSettingsValueItem(0, i18n(STR_RATE), i18n(STR_HZ));
	drawSettingsValueItem(1, i18n(STR_MIN_PAUSE), i18n(STR_MS));
	drawSettingsItem(2, i18n(STR_AUTOSTOP), GetBawAutoStop() ? i18n(STR_ON) : i18n(STR_OFF));
	drawSettingsValueTimeItem(3, i18n(STR_AUTOSTOP_TIME), 1);
	
	// выделяем текущий
	drawItemSelector();
	
//	glcd_fill_rect(0, 0, 2, GLCD_LCD_HEIGHT, 0);
//	if (!editMode) {
//		drawScrollBar(9+4, firstVisibleItem + selectedIndex, 0, 1);
//	}
}


static void drawRkrSettingsScreen() {
	drawSettingsValueItem(0, i18n(STR_BAUDRATE), 0);
	drawSettingsValueItem(1, i18n(STR_PILOT_BLOCK_LEN), i18n(STR_PULSE_CHAR));

	// выделяем текущий
	drawItemSelector();
}


static void drawConfirmDefaultTimesDialog() {
	glcd_drawCenteredStr_p(i18n(STR_CONFIRM_DEFAULT_TIMINGS_1), LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_CONFIRM_DEFAULT_TIMINGS_2), 2*LINES_DY, 1);
	drawLeftKey(i18n(STR_NO));
	drawRightKey(i18n(STR_YES));
}


static const char* getPartitionTypeStr() {
	switch (FioGetPartitionType()) {
		case PARTITION_TYPE_FREE:
			return STR_PARTITION_FREE;
		case PARTITION_TYPE_FAT12:
			return STR_PARTITION_FAT12;
		case PARTITION_TYPE_FAT16_32MB:
			return STR_PARTITION_FAT16_32MB;
		case PARTITION_TYPE_EXTENDED:
			return STR_PARTITION_EXTENDED;
		case PARTITION_TYPE_FAT16:
			return STR_PARTITION_FAT16;
		case PARTITION_TYPE_FAT32:
			return STR_PARTITION_FAT32;
		case PARTITION_TYPE_FAT32_LBA:
			return STR_PARTITION_FAT32_LBA;
		case PARTITION_TYPE_FAT16_LBA:
			return STR_PARTITION_FAT16_LBA;
		case PARTITION_TYPE_EXTENDED_LBA:
			return STR_PARTITION_EXTENDED_LBA;
	}
	return i18n(STR_FORMAT_UNKNOWN);
}

static void drawCardInfoScreen() {
	char str[16];
	char tmp[8];
	uint8_t y = 0;
	
	switch (selectedIndex) {
		case 0:
			glcd_drawCenteredStr_p(i18n(STR_FREE_MB), y, 1);
			y += LINES_DY;
			sprintf_P(str, i18n(STR_FREE_MB));
			ltoa(FioGetFsFreeSpace(), str, 10);
			strcat_P(str, CHR_SLASH);
			ltoa(FioGetFsSize(), tmp, 10);
			strcat(str, tmp);
			glcd_drawCenteredStr(str, y, 1);
			y += LINES_DY + 3;

			glcd_drawCenteredStr_p(i18n(STR_MANUFACTURER), y, 1);
			y += LINES_DY;
			strcpy(str, (char*)FioGetDiskInfo()->oem);
			strcat_P(str, CHR_SPACE);
			strcat_P(str, CHR_BRACKET_LEFT);
			itoa(FioGetDiskInfo()->manufacturer, tmp, 16);
			strcat(str, tmp);
			strcat_P(str, CHR_BRACKET_RIGHT);
			glcd_drawCenteredStr(str, y, 1);
			y += LINES_DY;
			itoa(FioGetDiskInfo()->manufacturing_month, str, 10);
			strcat_P(str, CHR_SLASH);
			itoa(FioGetDiskInfo()->manufacturing_year+2000, tmp, 10);
			strcat(str, tmp);
			glcd_drawCenteredStr(str, y, 1);
			break;
		
		case 1:
			glcd_drawCenteredStr_p(i18n(STR_SERIAL_NUMBER), y, 1);
			y += LINES_DY;
			ltoa(FioGetDiskInfo()->serial, str, 16);
			glcd_drawCenteredStr(str, y, 1);
			y += LINES_DY + 7;
		
			glcd_drawCenteredStr_p(i18n(STR_SIZE), y, 1);
			y += LINES_DY;
			ltoa(FioGetDiskInfo()->capacity / 1024 / 1024, str, 10);
			strcat_P(str, CHR_SPACE);
			strcat_P(str, i18n(STR_MB));
			glcd_drawCenteredStr(str, y, 1);
			break;
		
		case 2:
			// Product name
			glcd_drawCenteredStr_p(i18n(STR_PRODUCT_NAME), y , 1);
			y += LINES_DY;
			glcd_drawCenteredStr((char*) FioGetDiskInfo()->product, y, 1);
			y += LINES_DY;
		
			strcpy_P(str, i18n(STR_REVISION));
			itoa(FioGetDiskInfo()->revision, tmp, 16);
			strcat(str, tmp);
			glcd_drawCenteredStr(str, y, 1);
			y += LINES_DY;
		
			glcd_drawCenteredStr_p(i18n(STR_FORMAT), y, 1);
			y += LINES_DY;
			switch (FioGetDiskInfo()->format) {
				case SD_RAW_FORMAT_HARDDISK:
					glcd_drawCenteredStr_p(STR_FORMAT_HARDDISK, y, 1);
					break;
				case SD_RAW_FORMAT_SUPERFLOPPY:
					glcd_drawCenteredStr_p(STR_FORMAT_SUPERFLOPPY, y, 1);
					break;
				case SD_RAW_FORMAT_UNIVERSAL:
					glcd_drawCenteredStr_p(i18n(STR_FORMAT_UNIVERSAL), y, 1);
					break;
				case SD_RAW_FORMAT_UNKNOWN:
					glcd_drawCenteredStr_p(i18n(STR_FORMAT_UNKNOWN), y, 1);
					break;
			}
			break;
			
		case 3:
			// Partition type
			glcd_drawCenteredStr_p(i18n(STR_PARTITION), y , 1);
			y += LINES_DY;
			glcd_drawCenteredStr_p(getPartitionTypeStr(), y, 1);
			y += LINES_DY + 3;
			
			glcd_drawCenteredStr_p(i18n(STR_SIZES), y , 1);
			y += LINES_DY;

			// Sector size
			strcpy_P(str, i18n(STR_SECTOR_SIZE));
			itoa(FioGetSectorSize(), tmp, 10);
			strcat(str, tmp);
			glcd_drawCenteredStr(str, y, 1);
			y += LINES_DY;
			
			// Cluster size
			strcpy_P(str, i18n(STR_CLUSTER_SIZE));
			uint16_t clusterSize = FioGetClusterSize();
			if (clusterSize <= 1024*8) {
				itoa(clusterSize, tmp, 10);
				strcat(str, tmp);
			} else {
				itoa(clusterSize/1024, tmp, 10);
				strcat(str, tmp);
				strcat_P(str, STR_K);
			}
			glcd_drawCenteredStr(str, y, 1);			
			break;
	}

	drawScrollBar(20, selectedIndex*5, GLCD_LCD_WIDTH-1, 1);
}


// Возвращает символ в ячейке экранной клавиатуры
static char getKeyboardChar(uint8_t row, uint8_t col) {
	switch (row) {
		case 0:
			return '0' + col;
		case 1:
			return 'A' + col;
		case 2:
			return 'K' + col;
		case 3:
			if (col < 6) {
				return 'U' + col;
			} else if (col == 6) {
				return '-';
			} else if (col == 7) {
				return '_';
			} else if (col == 8) {
				return CHAR_BACKSPACE;
			} else {
				return CHAR_ENTER;
			}
	}
	return 0;
}


static void drawKeyboardScreen() {
	// символы
	for (uint8_t col = 0; col < 10; col++) {
		glcd_draw_char_xy(2 + col*8, 1, getKeyboardChar(0, col));
		glcd_draw_char_xy(2 + col*8, 1 + LINES_DY, getKeyboardChar(1, col));
		glcd_draw_char_xy(2 + col*8, 1 + 2*LINES_DY, getKeyboardChar(2, col));
		glcd_draw_char_xy(2 + col*8, 1 + 3*LINES_DY, getKeyboardChar(3, col));
	}
	// выделяем текущий
	uint8_t x = (selectedIndex % 10) * 8;
	uint8_t y = (selectedIndex / 10) * LINES_DY;
	glcd_invert_area(x, y, 9, LINES_DY);
	// поле введенного имени
	x = glcd_draw_char_xy(0, GLCD_LCD_HEIGHT - LINES_DY, CHAR_RIGHT);
	x = glcd_draw_string_xy(x + 2, GLCD_LCD_HEIGHT - LINES_DY, currentFileName);
	
	// мигающий курсор
	uint8_t hsec = GetDateTime()->hsec;
	if (hsec < 25 || (hsec > 50 && hsec < 75)) {
		glcd_invert_area(x, GLCD_LCD_HEIGHT - 3, 5, 2);
	}
}



static void drawDateTimeScreen() {
	uint8_t x = 10;
	uint8_t y = 7;

	datetime_t *dt = GetDateTime();

	if (selectedIndex == 0) {
		glcd_draw_char_xy(x + 3, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 3, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint8_2(x, y, dt->day) + 1;
	if (selectedIndex == 0) {
		glcd_invert_area(x - 14, y-2, 14, LINES_DY+2);
	}
	
	x += glcd_draw_char_xy(x+1, y, '-') + 3;
	
	if (selectedIndex == 1) {
		glcd_draw_char_xy(x + 3, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 3, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint8_2(x, y, dt->month) + 1;
	if (selectedIndex == 1) {
		glcd_invert_area(x - 14, y-2, 14, LINES_DY+2);
	}
	
	x += glcd_draw_char_xy(x+1, y, '-') + 3;
	
	if (selectedIndex == 2) {
		glcd_draw_char_xy(x + 9, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 9, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint16(x, y, GetBaseYear() + dt->year) + 1;
	if (selectedIndex == 2) {
		glcd_invert_area(x - 26, y-2, 26, LINES_DY+2);
	}
	
	x = 15;
	y = 30;
	if (selectedIndex == 3) {
		glcd_draw_char_xy(x + 3, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 3, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint8_2(x, y, dt->hour) + 1;
	if (selectedIndex == 3) {
		glcd_invert_area(x - 15, y-2, 15, LINES_DY+2);
	}
	x += glcd_draw_char_xy(x, y, ':') + 1;
	if (selectedIndex == 4) {
		glcd_draw_char_xy(x + 3, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 3, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint8_2(x, y, dt->min) + 1;
	if (selectedIndex == 4) {
		glcd_invert_area(x - 15, y-2, 15, LINES_DY+2);
	}
	x += glcd_draw_char_xy(x, y, ':') + 1;
	if (selectedIndex == 5) {
		glcd_draw_char_xy(x + 3, y - 6, CHAR_UP);
		glcd_draw_char_xy(x + 3, y + 6, CHAR_DOWN);
	}
	x += glcd_draw_uint8_2(x, y, dt->sec) + 1;
	if (selectedIndex == 5) {
		glcd_invert_area(x - 15, y-2, 15, LINES_DY+2);
	}
}

static void drawFileInfoScreen() {
	glcd_drawCenteredStr(currentFileName, 0, 1);
}


static void drawOverwriteFileScreen() {
	glcd_drawCenteredStr(currentFileName, 0, 1);
	//char str[15];
	
	glcd_drawCenteredStr_p(i18n(STR_ALREADY_EXISTS), 2*LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_REPLACE_IT), 3*LINES_DY, 1);
	
	drawLeftKey(i18n(STR_NO));
	drawRightKey(i18n(STR_YES));
}


static void drawLoadingScreen() {
	glcd_drawCenteredStr_p(i18n(STR_LOADING), 2*LINES_DY, 1);
}

static void drawInsertCardScreen() {
	glcd_drawCenteredStr_p(i18n(STR_INSERT), 1*LINES_DY, 1);
	glcd_drawCenteredStr_p(i18n(STR_SD_CARD), 2*LINES_DY, 1);
}


static void drawDisplayContrastScreen() {
	glcd_drawCenteredStr_p(i18n(STR_CONTRAST), 1*LINES_DY, 1);
	glcd_draw_uint16(GLCD_LCD_WIDTH/2 - 5, 2*LINES_DY + 5, GetDisplayContrast());
	
	glcd_fill_rect(10, 3*LINES_DY + 5, GLCD_LCD_WIDTH - 20, 8, 1);
}


static void drawErrorScreen() {
	glcd_drawCenteredStr_p(i18n(STR_ERROR), 2, 1);
	glcd_invert_area(0, 0, GLCD_LCD_WIDTH, LINES_DY+2);
	switch (errorMsg) {
		case ERROR_CREATE_FILE:
			glcd_drawCenterStrings_p(i18n(STR_CANT_CREATE_FILE), 1*LINES_DY+7, 1);
			break;
		case ERROR_READ_FILE:
			glcd_drawCenterStrings_p(i18n(STR_CANT_READ_FILE), 1*LINES_DY+7, 1);
			break;
		case ERROR_WRITE_FILE:
			glcd_drawCenterStrings_p(i18n(STR_CANT_WRITE_FILE), 1*LINES_DY+7, 1);
			break;			
		case ERROR_FIO_ERROR:
			if ((fio_error_mask & FIO_ERROR_READ_NOT_READY) && (fio_error_mask & FIO_ERROR_WRITE_NOT_READY)) {
				glcd_drawCenteredStr_p(i18n(STR_READ_WRITE_TIMEOUT), 1*LINES_DY+7, 1);
			} else if (fio_error_mask & FIO_ERROR_READ_NOT_READY) {
				glcd_drawCenteredStr_p(i18n(STR_READ_TIMEOUT), 1*LINES_DY+7, 1);
			} else if (fio_error_mask & FIO_ERROR_WRITE_NOT_READY) {
				glcd_drawCenteredStr_p(i18n(STR_WRITE_TIMEOUT), 1*LINES_DY+7, 1);
			}
			char str[16];
			itoa(fio_error_cnt, str, 10);
			strcat_P(str, i18n(STR__ERRORS));
			glcd_drawCenteredStr(str, 2*LINES_DY+7, 1);
			break;
		case ERROR_WRONG_FILE_FORMAT:
			glcd_drawCenterStrings_p(i18n(STR_WRONG_FILE_FORMAT), 1*LINES_DY+7, 1);
			break;
		case ERROR_WRONG_CRC:
			glcd_drawCenterStrings_p(i18n(STR_WRONG_CRC), 1*LINES_DY+7, 1);
	}
	glcd_draw_rect(0, 0, GLCD_LCD_WIDTH, GLCD_LCD_HEIGHT, 1);
}


static void drawPlayWavScreen() {
//	// при воспроизведении WAV процессорного времени совсем нет на рисование
//	if (TIMSK & _BV(OCIE1A)) {
//		return;
//	}
	glcd_drawCenteredStr_p(i18n(STR_PLAYING), 2, 1);
	glcd_drawCenteredStr(currentFileName, 2+LINES_DY, 1);
	drawLeftKey(i18n(STR_BACK));
}	


static void drawSelectSaveFormatScreen() {
	glcd_drawCenteredStr_p(i18n(STR_SAVE_AS), 2, 1);
	glcd_invert_area(0, 0, GLCD_LCD_WIDTH, LINES_DY+2);
	glcd_draw_rect(0, 0, GLCD_LCD_WIDTH, GLCD_LCD_HEIGHT, 1);
	
	glcd_drawCenteredStr_p(STR_TAP, LINES_DY + 7, 1);
	glcd_drawCenteredStr_p(STR_BAW, 2*LINES_DY + 7, 1);
//	glcd_drawCenteredStr_p(STR_WAV, 3*LINES_DY + 7, 1);	
	
	glcd_invert_area(25, LINES_DY + 6 + selectedIndex*LINES_DY, GLCD_LCD_WIDTH-50, LINES_DY);
}


static void drawSaveWavScreen() {
	glcd_drawCenteredStr(currentFileName, 0, 1);
}


static void drawPlayBawScreen() {
	uint8_t x;
	
//	// имя файла
//	glcd_drawCenteredStr(currentFileName, 0, 1);
//	// индикатор прогресса
//	glcd_draw_rect(0, LINES_DY+1, GLCD_LCD_WIDTH, 7, 1);
	
	
	
	uint8_t min, sec;
	if (BawPlayingInProgress()) {
		min = GetDateTime()->min;
		sec = GetDateTime()->sec;
		// TODO !!! привести все плееры к единому виду !!!
		// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
		GetDateTime()->year = min;
		GetDateTime()->month = sec;
	} else {
		min = GetDateTime()->year;
		sec = GetDateTime()->month;
	}
	
	uint16_t wavePlayTime = min*60 + sec;
	BawPlayerIdle(wavePlayTime*100 + GetDateTime()->hsec);
	//uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * wavePlayTime / playedFileDuration;
	//glcd_fill_rect(2, LINES_DY+3, (uint8_t)progressWidth, 3, 1);	
	
	// номер текущего блока
	if (currentBlockNumber > 0) {
		x = glcd_draw_string_xy_P(0, 2*LINES_DY+1, i18n(STR_BLOCK)) + 5;
		
		bool drawBlock;
		if (pause_mode) {
			// мигающий курсор
			uint8_t hsec = GetDateTime()->hsec;
			drawBlock = hsec < 25 || (hsec > 50 && hsec < 75);
		} else {
			drawBlock = true;
		}
		if (drawBlock) {
			x += glcd_draw_uint16(x, 2*LINES_DY+1, currentBlockNumber) + 2;
		} else {
			if (currentBlockNumber < 10) {
				x += 6 + 2;
			} else if (currentBlockNumber < 100) {
				x += 6*2 + 2;
			} else {
				x += 6*3 + 2;
			}
		}
		x += glcd_draw_char_xy(x, 2*LINES_DY+1, '/') + 2;
		glcd_draw_uint16(x, 2*LINES_DY+1, numberOfBlocks);
	}
	
	uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * wavePlayTime / playedFileDuration;
	drawPlayerProgress(BawPlayingInProgress(), (uint8_t)progressWidth);
	
//	if (pause_mode) {
//		glcd_drawCenteredStr_p(i18n(STR_PAUSE), 3*LINES_DY + 2, 1);
//	} else {
//		// прошедшее время
//		// если файл в процессе загрузки
//		uint8_t min, sec;
//		if (BawPlayingInProgress()) {
//			min = GetDateTime()->min;
//			sec = GetDateTime()->sec;
//			// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
//			GetDateTime()->year = min;
//			GetDateTime()->month = sec;
//		} else {
//			min = GetDateTime()->year;
//			sec = GetDateTime()->month;
//		}
//		x = (GLCD_LCD_WIDTH - 5*5) / 2;
//		x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, min);
//		x += glcd_draw_char_xy(x, 3*LINES_DY + 2, ':');
//		x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, sec);
//	}

	// подсказки к кнопкам
	if (BawPlayingInProgress()|| pause_mode) {
		drawLeftKey(i18n(STR_STOP));
		if (pause_mode) {
			drawRightKey(i18n(STR_RESUME));
		} else {
			drawRightKey(i18n(STR_PAUSE));
		}
	} else {
		glcd_drawCenteredStr_p(i18n(STR_DONE), GLCD_LCD_HEIGHT - LINES_DY, 1);
	}
/*

	
	
	// имя файла
	glcd_drawCenteredStr(currentFileName, 0, 1);

	// индикатор прогресса
	glcd_draw_rect(0, LINES_DY+1, GLCD_LCD_WIDTH, 7, 1);

	uint8_t min, sec;
	if (BawPlayingInProgress()) {
		min = GetDateTime()->min;
		sec = GetDateTime()->sec;
		// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
		GetDateTime()->year = min;
		GetDateTime()->month = sec;
	} else {
		min = GetDateTime()->year;
		sec = GetDateTime()->month;
	}
	
	uint16_t wavePlayTime = min*60 + sec;
	uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * wavePlayTime / waveFileDuration;
	glcd_fill_rect(2, LINES_DY+3, (uint8_t)progressWidth, 3, 1);
	
	uint8_t x = (GLCD_LCD_WIDTH - 5*5) / 2;
	x += glcd_draw_uint8_2(x, 3*LINES_DY - 3, min);
	x += glcd_draw_char_xy(x, 3*LINES_DY - 3, ':');
	x += glcd_draw_uint8_2(x, 3*LINES_DY - 3, sec);

	// подсказки к кнопкам
	if (BawPlayingInProgress()) {
		drawLeftKey(i18n(STR_STOP));
	} else {
		glcd_drawCenteredStr_p(i18n(STR_DONE), GLCD_LCD_HEIGHT - LINES_DY, 1);
	}
 */ 
}


static void drawSaveBawScreen() {
	datetime_t *dt = GetDateTime();
	uint8_t min = dt->min;
	uint8_t sec = dt->sec;

	// имя файла
	glcd_drawCenteredStr(currentFileName, 0, 1);
	
	uint8_t x = (GLCD_LCD_WIDTH - 5*5) / 2;
	x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, min);
	x += glcd_draw_char_xy(x, 3*LINES_DY + 2, ':');
	x += glcd_draw_uint8_2(x, 3*LINES_DY + 2, sec);
	
//glcd_draw_uint16(0, LINES_DY, bawAutoStopCounter);
	
	drawLeftKey(i18n(STR_STOP));
}


static void drawPleaseWaitScreen() {
	glcd_drawCenterStrings_p(i18n(STR_PLEASE_WAIT), LINES_DY + 7, 1);
}

static void drawPlayRkrScreen() {
	uint32_t progressWidth = (GLCD_LCD_WIDTH-4) * RkrGetPlayProgress() / 100;
	drawPlayerProgress(RkrPlayingInProgress(), (uint8_t)progressWidth);
	
	// адреса блока
	uint8_t x = 10;
	x = glcd_drawHexWord(rkr_start_address, x, 19, 1);
	x += glcd_draw_char_xy(x + 1, 19, '-') + 2;
	glcd_drawHexWord(rkr_end_address, x, 19, 1);
	
	// подсказки к кнопкам
	if (RkrPlayingInProgress()) {
		drawLeftKey(i18n(STR_STOP));
	} else {
		glcd_drawCenteredStr_p(i18n(STR_DONE), GLCD_LCD_HEIGHT - LINES_DY, 1);
	}	
}

#endif // UI_DRAW_H_ 