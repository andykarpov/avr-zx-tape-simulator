/*
 * ui.c
 *
 * Created: 05.06.2015 20:16:58
 *  Author: Trol
 */ 

#include "ui.h"

#include <string.h>
#include <stdio.h>

#include "lib/glcd/glcd.h"
#include "lib/glcd/fonts/font5x7.h"
#include "lib/sdreader/partition.h"

#include "debug.h"

#include "formats/tap_player.h"
#include "formats/tap_recorder.h"
#include "formats/wav_player.h"
#include "formats/wav_recorder.h"
#include "formats/baw_player.h"
#include "formats/baw_recorder.h"
#include "formats/rkr_player.h"
#include "fileio.h"
#include "keyboard.h"
#include "i18n.h"
#include "settings.h"
#include "time.h"
#include "beeper.h"
#include "general.h"

#include <avr/wdt.h>
#include <avr/pgmspace.h>

// типы файлов
#define FILE_UNKNOWN					0
#define FILE_TAP						1
#define FILE_BAW						2
#define FILE_WAV						3
#define FILE_RKR						4


#define SETTINGS_ITEM_LANG				0
#define SETTINGS_ITEM_SOUND_KEY		1
#define SETTINGS_ITEM_SOUND_PLAY		2
#define SETTINGS_ITEM_SOUND_REC		3
#define SETTINGS_ITEM_BACKLIGHT		4
#define SETTINGS_ITEM_SPEED			5
#define SETTINGS_ITEM_TAP_TIMES		6
#define SETTINGS_ITEM_BAW_SETTINGS	7
#define SETTINGS_ITEM_RKR_SETTINGS	8
#define SETTINGS_ITEM_DATETIME		9
#define SETTINGS_ITEM_CONTRAST		10



#define SETTINGS_ITEM_FIRST			SETTINGS_ITEM_LANG
#define SETTINGS_ITEM_LAST				SETTINGS_ITEM_CONTRAST


uint8_t errorMsg;
uint8_t errorBackScreen;
uint8_t selectedSaveFormat;		// FILE_xxx

extern volatile uint8_t currentBlockNumber;
extern volatile uint8_t numberOfBlocks;

//extern uint8_t fio_error_mask;
//extern uint16_t fio_error_cnt;

// это значение используется для того, чтобы не перерисовывать каждый раз весь экран если в этом нет необходимости
// например, тут можно хранить количество секунд для экранов загрузки/сохранения и выполнять перерисовку только 
// если это число изменилось
uint8_t _drawLastValue;

#include "ui/ui_utils.h"
#include "ui/ui_draw.h"
#include "ui/ui_keyboard.h"




void_proc ui_draw_handler;// = &EmptyVoidProc;
uint8_proc ui_key_handler;


uint8_t GetScreen() {
	return screen;
}

void __attribute__ ((noinline)) SetScreen(uint8_t aScreen) {
	screen = aScreen;
	switch (screen) {
		case SCREEN_MAIN_MENU:
			ui_draw_handler = &drawMainMenuScreen;
			ui_key_handler = &keyHandlerMainMenuScreen;
			break;
		case SCREEN_BROWSE:
			ui_draw_handler = &drawBrowserScreen;
			ui_key_handler = &keyHandlerBrowserScreen;
			break;
		case SCREEN_TAP_LOAD:
			ui_draw_handler = &drawPlayTapScreen;
			ui_key_handler = &keyHandlerPlayTapScreen;
			break;
		case SCREEN_TAP_SAVE:
			ui_draw_handler = &drawTapSaveScreen;
			ui_key_handler = &keyHandlerTapSaveScreen;
			break;
		case SCREEN_SETTINGS:
			ui_draw_handler = &drawSettingsScreen;
			ui_key_handler = &keyHandlerSettingsScreen;
			break;
		case SCREEN_ABOUT:
			ui_draw_handler = &drawAboutScreen;
			ui_key_handler = &keyHandlerAboutScreen;
			break;
		case SCREEN_WRONG_TAP:
			ui_draw_handler = &drawWrongTapFileScreen;
			ui_key_handler = &keyHandlerWrongTapScreen;
			break;
		case SCREEN_WAIT_SAVE:
			ui_draw_handler = &drawWaitTapeScreen;
			ui_key_handler = &keyHandlerWaitSaveScreen;
			break;
		case SCREEN_SAVE_TAP_DONE:
			ui_draw_handler = &drawTapSaveDoneScreen;
			ui_key_handler = &keyHandlerSaveTapDoneScreen;
			break;
		case SCREEN_CARD_INFO:
			ui_draw_handler = &drawCardInfoScreen;
			ui_key_handler = &keyHandlerCardInfoScreen;
			break;
		case SCREEN_TAP_SETTINGS:
			ui_draw_handler = &drawTapTimesSetupScreen;
			ui_key_handler = &keyHandlerTimeSettingsScreen;
			break;
		case SCREEN_CONFIRM_DEFAULT_TIMES:
			ui_draw_handler = &drawConfirmDefaultTimesDialog;
			ui_key_handler = &keyHandlerConfirmDefaultTimeSettingsScreen;
			break;
		case SCREEN_KEYBOARD:
			ui_draw_handler = &drawKeyboardScreen;
			ui_key_handler = &keyHandlerKeyboardScreen;
			break;
		case SCREEN_DATETIME:
			ui_draw_handler = &drawDateTimeScreen;
			ui_key_handler = &keyHandlerDateTimeScreen;
			break;
		case SCREEN_FILE_MENU:
			ui_draw_handler = &drawFileInfoScreen;
			ui_key_handler = &keyHandlerFileInfoScreen;
			break;
		case SCREEN_OVERWRITE_FILE:
			ui_draw_handler = &drawOverwriteFileScreen;
			ui_key_handler = &keyHandlerOverwriteFileScreen;
			break;
		case SCREEN_LOADING:
			ui_draw_handler = &drawLoadingScreen;
			ui_key_handler = &keyHandlerVoid;
			break;
		case SCREEN_INSERT_CARD:
			ui_draw_handler = &drawInsertCardScreen;
			ui_key_handler = &keyHandlerInsertCardScreen;
			break;
		case SCREEN_SETUP_CONTRAST:
			ui_draw_handler = &drawDisplayContrastScreen;
			ui_key_handler = &keyHandlerDisplayContrastScreen;
			break;
		case SCREEN_ERROR:
			ui_draw_handler = &drawErrorScreen;
			ui_key_handler = &keyHandlerErrorScreen;
			break;
		case SCREEN_PLAY_WAV:
			ui_draw_handler = &drawPlayWavScreen;
			ui_key_handler = &keyHandlerPlayWawScreen;
			break;
		case SCREEN_SELECT_SAVE_FORMAT:
			ui_draw_handler = drawSelectSaveFormatScreen;
			ui_key_handler = &keyHandlerSelectSaveFormatScreen;
			break;
		case SCREEN_WAV_SAVE:
			ui_draw_handler = &drawSaveWavScreen;
			ui_key_handler = &keyHandlerScreenSaveWav;
			break;
		case SCREEN_PLAY_BAW:
			ui_draw_handler = &drawPlayBawScreen;
			ui_key_handler = &keyHandlerPlayBawScreen;
			break;		
		case SCREEN_BAW_SAVE:
			_drawLastValue = 0xff;	// используется для того, чтобы не перерисовывать экран чаще раза в секунду
			ui_draw_handler = &drawSaveBawScreen;
			ui_key_handler = &keyHandlerSaveBawScreen;
			break;
		case SCREEN_PLEASE_WAIT:
			ui_draw_handler = &drawPleaseWaitScreen;
			ui_key_handler = &keyHandlerVoid;
			break;
		case SCREEN_BAW_SETTINGS:
			ui_draw_handler = &drawBawSettingsScreen;
			ui_key_handler = &keyHandlerBawSettingsScreen;
			break;
		case SCREEN_RKR_SETTINGS:
			ui_draw_handler = &drawRkrSettingsScreen;
			ui_key_handler = &keyHandlerRkrSettingsScreen;
			break;			
		case SCREEN_PLAY_RKR:
			ui_draw_handler = &drawPlayRkrScreen;
			ui_key_handler = &keyHandlerPlayRkrScreen;
			break;
	}
}

void UiShowError(uint8_t msg, uint8_t backToScreen) {
	errorMsg = msg;
	errorBackScreen = backToScreen;
	SetScreen(SCREEN_ERROR);
}


void Draw() {
	// при воспроизведении WAV процессорного времени совсем нет на рисование
	// TODO как-то это некрасиво выглядит
	if (screen == SCREEN_PLAY_WAV && (TIMSK & _BV(OCIE1A))) {
		return;
	}
	GLCD_SELECT();
	glcd_tiny_set_font(Font5x7, 5, 7, 32, 0xff);
	glcd_clear_buffer();
	
//	printf("draw %i\n", screen);
	ui_draw_handler();
//	switch (screen) {
//		case SCREEN_TAP_SAVE:
//			drawTapSaveScreen();
//			break;
//		case SCREEN_TAP_LOAD:
//			drawPlayTapScreen();
//			break;
//		case SCREEN_MAIN_MENU:
//			drawMainMenuScreen();
//			break;
//		case SCREEN_ABOUT:
//			drawAboutScreen();
//			break;
//		case SCREEN_BROWSE:
//			drawBrowserScreen();
//			break;
//		case SCREEN_WRONG_TAP:
//			drawWrongTapFileScreen();
//			break;
//		case SCREEN_WAIT_SAVE:
//			drawWaitTapeScreen();
//			break;
//		case SCREEN_SAVE_TAP_DONE:
//			drawTapSaveDoneScreen();
//			break;
//		case SCREEN_SETTINGS:
//			drawSettingsScreen();
//			break;
//		case SCREEN_CARD_INFO:
//			drawCardInfoScreen();
//			break;
//		case SCREEN_SETUP_TAP_TIMES:
//			drawTapTimesSetupScreen();
//			break;
//		case SCREEN_CONFIRM_DEFAULT_TIMES:
//			drawConfirmDefaultTimesDialog();
//			break;
//		case SCREEN_KEYBOARD:
//			drawKeyboardScreen();
//			break;
//		case SCREEN_DATETIME:
//			drawDateTimeScreen();
//			break;
//		case SCREEN_FILE_MENU:
//			drawFileInfoScreen();
//			break;
//		case SCREEN_OVERWRITE_FILE:
//			drawOverwriteFileScreen();
//			break;
//		case SCREEN_LOADING:
//			drawLoadingScreen();
//			break;
//		case SCREEN_INSERT_CARD:
//			drawInsertCardScreen();
//			break;
//		case SCREEN_SETUP_CONTRAST:
//			drawDisplayContrastScreen();
//			break;
//		case SCREEN_ERROR:
//			drawErrorScreen();
//			break;
//		case SCREEN_PLAY_WAV:
//			drawPlayWavScreen();
//			break;
//		case SCREEN_SELECT_SAVE_FORMAT:
//			drawSelectSaveFormatScreen();
//			break;
//		case SCREEN_WAV_SAVE:
//			drawSaveWavScreen();
//			break;
//		case SCREEN_PLAY_BAW:
//			drawPlayBawScreen();
//			break;
//		case SCREEN_BAW_SAVE:
//			drawSaveBawScreen();
//			break;
//		case SCREEN_PLEASE_WAIT:
//			drawPleaseWaitScreen();
//			break;
//		case SCREEN_BAW_SETTINGS:
//			drawBawSettingsScreen();
//			break;
//		case SCREEN_PLAY_RKR:
//			drawPlayRkrScreen();
//			break;
//	}
	glcd_write();
	GLCD_DESELECT();
}


void onKeyPressed(uint8_t key) {
	ui_key_handler(key);
//	switch (screen) {
//		case SCREEN_TAP_SAVE:
//			keyHandlerTapSaveScreen(key);
//			break;
//		case SCREEN_TAP_LOAD:
//			keyHandlerPlayTapScreen(key);
//			break;
//		case SCREEN_MAIN_MENU:
//			keyHandlerMainMenuScreen(key);
//			break;
//		case SCREEN_ABOUT:
//			SetScreen(SCREEN_MAIN_MENU);
//			break;
//		case SCREEN_BROWSE:
//			keyHandlerBrowserScreen(key);
//			break;
//		case SCREEN_WRONG_TAP:
//			SetScreen(SCREEN_BROWSE);
//			break;
//		case SCREEN_WAIT_SAVE:
//			if (key == KEY_LEFT) {
//				if (selectedSaveFormat == FILE_TAP) {
//					TapSaveCancel();
//				} else if (selectedSaveFormat == FILE_BAW) {
//					BawSaveCancel();
//				}
//				SetScreen(SCREEN_MAIN_MENU);
//				selectedIndex = 1;
//			}
//			break;
//		case SCREEN_SAVE_TAP_DONE:
//			selectedIndex = 1;
//			if (fio_error_mask) {
//				UiShowError(ERROR_FIO_ERROR, SCREEN_MAIN_MENU);
//			} else {
//				SetScreen(SCREEN_MAIN_MENU);
//			}
//			break;
//		case SCREEN_SETTINGS:
//			keyHandlerSettingsScreen(key);
//			break;
//		case SCREEN_SETUP_TAP_TIMES:
//			keyHandlerTimeSettingsScreen(key);
//			break;
//		case SCREEN_CONFIRM_DEFAULT_TIMES:
//			keyHandlerConfirmDefaultTimeSettingsScreen(key);
//			break;			
//		case SCREEN_CARD_INFO:
//			keyHandlerCardInfoScreen(key);
//			break;
//		case SCREEN_KEYBOARD:
//			keyHandlerKeyboardScreen(key);
//			break;
//		case SCREEN_DATETIME:
//			keyHandlerDateTimeScreen(key);
//			break;
//		case SCREEN_FILE_MENU:
//			keyHandlerFileInfoScreen(key);
//			break;
//		case SCREEN_OVERWRITE_FILE:
//			keyHandlerOverwriteFileScreen(key);
//			break;
//		case SCREEN_INSERT_CARD:
//			SetScreen(SCREEN_MAIN_MENU);
//			//selectedIndex = 1;
//			break;
//		case SCREEN_SETUP_CONTRAST:
//			keyHandlerDisplayContrastScreen(key);
//			break;
//		case SCREEN_ERROR:
//			if (errorMsg == ERROR_FIO_ERROR) {
//				fio_error_cnt = 0;
//				fio_error_mask = 0;
//			}
//			SetScreen(errorBackScreen);
//			break;
//		case SCREEN_PLAY_WAV:
//			if (key == KEY_LEFT) {
//				closeWav();
//			}
//			break;
//		case SCREEN_SELECT_SAVE_FORMAT:
//			keyHandlerSelectSaveFormatScreen(key);
//			break;
//		case SCREEN_WAV_SAVE:
//			keyHandlerScreenSaveWav(key);
//			break;
//		case SCREEN_BAW_SAVE:
//			keyHandlerSaveBawScreen(key);
//			break;
//		case SCREEN_PLAY_BAW:
//			keyHandlerPlayBawScreen(key);
//			break;
//		case SCREEN_BAW_SETTINGS:
//			keyHandlerBawSettingsScreen(key);
//			break;
//		case SCREEN_PLAY_RKR:
//			keyHandlerPlayRkrScreen(key);
//			break;
//	}
	Draw();	
}


void UiOnIdle() {
	static uint8_t cnt = 0;
	
	switch (screen) {
		case SCREEN_TAP_LOAD:
			break;
		case SCREEN_PLAY_BAW:
		case SCREEN_PLAY_RKR:			
			// обновляем экран при каждом 5 вызове
			if (++cnt < 5) {
				return;
			}
			cnt = 0;
			break;
		case SCREEN_BAW_SAVE:
			// если время не изменилось с момента последнего вызова, то выходим
			if (_drawLastValue == GetDateTime()->sec) {
				return;
			}	
			// если сейчас не самое удачное время для перерисования, выходим
			if (FioGetWriteBusyValue() > 50) {
				return;
			}
			// если пора прекращать запись по автостопу
			if (GetBawAutoStop() && bawAutoStopCounter == 0xffff) {
				keyHandlerSaveBawScreen(0);	// TODO key
			}
			_drawLastValue = GetDateTime()->sec;
			break;			
		case SCREEN_TAP_SAVE:
			// при чтении пилота нет смысла часто прерываться на отрисовку
			if (TapSaverGetStatus() == TAPE_IN_PILOT) {
				if (cnt == 100) {
					return;
				}
				cnt = 100;
				break;
			}
			// обновляем экран при каждом 5 вызове
			if (++cnt < 5) {
				return;
			}
			cnt = 0;
			break;
		case SCREEN_WAIT_SAVE:
			if (selectedSaveFormat == FILE_TAP && TapSaverGetProgress()->block > 0) {
				SetScreen(SCREEN_TAP_SAVE);
//			} else if (selectedSaveFormat == FILE_WAV && WavSaveIsBegan()) {
//				SetScreen(SCREEN_WAV_SAVE);
			} else if (selectedSaveFormat == FILE_BAW && BawSaveIsBegan()) {
				BawSaveStart();
				SetScreen(SCREEN_BAW_SAVE);
				datetime_t *dt = GetDateTime();
				dt->min = 0;
				dt->sec = 0;
				Draw();
				//BawSaveStart();
				return;
			} else if (BawSaveIsBegan()) {
				return;
			}
			break;
		case SCREEN_TAP_SETTINGS:
		case SCREEN_BAW_SETTINGS:
		case SCREEN_RKR_SETTINGS:
			if (editMode) {
				if (KeyboardRepeated(KEY_UP)) {
					onKeyPressed(KEY_UP);
				} else if (KeyboardRepeated(KEY_DOWN)) {
					onKeyPressed(KEY_DOWN);
				}
			}
			break;
		case SCREEN_KEYBOARD:
		case SCREEN_DATETIME:
			break;
		//case SCREEN_BROWSE:
		//case SCREEN_CARD_INFO:
			
		default:
			return;
	}	
	Draw();
}

void browserScroll(uint32_t offset) {
	
}


static char* browserGetSelectedFile() {
	if (firstVisibleItem + selectedIndex == 0) {
		return GetDirectoryData();
	}
	uint16_t index = 0;
	for (uint16_t pos = 0; pos < DIR_DATA_SIZE; pos++) {
		char ch = GetDirectoryData()[pos];
		if (ch == 0) {
			index++;
			if (index == firstVisibleItem + selectedIndex) {
				return GetDirectoryData() + pos + 1;
			}
		}
	}
	return 0;
}

// загружает и отображает текущую директорию
static void refreshBrowser() {
	SetScreen(SCREEN_LOADING);
	Draw();
	FioClose();		FioInit();	// TODO надо сменить директорию, для этого переинициализируемся
	FioReadDirFrom(0);
	//FioClose();
	firstVisibleItem = 0;
	selectedIndex = 0;
	SetScreen(SCREEN_BROWSE);	
}

// загружает и отображает текущую директорию с указанной позиции
static void refreshBrowserFrom(uint32_t offset) {
	//FioInit();
	FioReadDirFrom(offset);
	//FioClose();
}


static bool gotoBack() {
MSG("back");	
	// запоминаем текущий каталог
	// извлекаем его из полного пути
	char path[32];
	uint8_t namePos = 0;
	char *fullPath = FioGetPath();
printf("fullPath = %s %lu\n", fullPath, strlen(fullPath));	
	if (strlen(fullPath) == 0) {
//		fullPath[0] = '/';
//		fullPath[1] = 0;
		return false;
	}
	uint8_t pathPos = 0;
	while (true) {
		char c = fullPath[pathPos++];
		if (c == 0) {
			break;
		} else if (c == '/') {
			namePos = 0;
			path[0] = 0;
		} else {
			path[namePos++] = c;
			path[namePos] = 0;
		} 
	}
	bool result = FioGotoBack();
	if (!result) {
MSG("FioGotoBack()");
		return result;
	}
	FioClose(); FioInit();	// TODO полная переинициализация только ради смены директории
printf("? path %s\n", path);	
	uint32_t offset = FioGetDirectoryIndex(path);
MSG_DEC("path ", offset);

	firstVisibleItem = 0;
	selectedIndex = 0;	

	if (offset == 0xffffffff) {		// если директории по каким-то причинам не найдено, переходим в начало
		offset = 0;
	} else if (offset > FioGetDirData()->dirSize - LINES_PER_SCREEN) {	// если директория была в конце списка
printf("--> %i %i %i\n", offset, FioGetDirData()->dirSize, LINES_PER_SCREEN);
		if (FioGetDirData()->dirSize > LINES_PER_SCREEN) {	// если мы где-то в конце длинного списка
MSG_DEC("offset1 ", FioGetDirData()->dirSize);
			uint32_t oldOffset = offset;
			offset = FioGetDirData()->dirSize - LINES_PER_SCREEN;
			selectedIndex = oldOffset - offset;
		} else {	// если все элементы умещаются на одном экране
MSG_DEC("offset2 ", FioGetDirData()->dirSize);			
			selectedIndex = offset;
			offset = 0;
		}		
	} else if (offset < LINES_PER_SCREEN) {
		selectedIndex = offset;
		offset = 0;
	}

	FioReadDirFrom(offset);
	//FioClose();
	return result;
}

const char * UiGetCurrentFileName() {
	return currentFileName;
}