/*
 * ui_keyboard.h
 *
 * Created: 05.07.2015 21:21:05
 *  Author: Trol
 */ 


#ifndef UI_KEYBOARD_H_
#define UI_KEYBOARD_H_


#include "ui_utils.h"
#include "ui_draw.h"

uint32_t savedFileOffset;	// сюда сохраняется смещение первого файла в буфере перед воспроизведением файла для последующего возврата к нему

extern uint8_t tasksFlag;
/************************************************************************/
/* Обработчики клавиатуры                                               */
/************************************************************************/

static void browserOpenItem(const char* name) {
	// если это каталог, то переходим в него
	if (name[0] == '>') {
//		SetScreen(SCREEN_LOADING);
//		Draw();
		FioGotoSubdir(name+1);
		refreshBrowser();
//		SetScreen(SCREEN_BROWSE);
	} else {
		// начинаем загрузку файла
		MSG_DEC("load_file first visible ", firstVisibleItem);
		MSG_DEC("selected index ", selectedIndex);
		MSG_DEC("dir size ", FioGetDirData()->dirSize);
		MSG_DEC("offset ", FioGetDirData()->offset);

		// сохраняем номер файла чтобы потом вернуться к нему
		savedFileOffset = FioGetDirData()->offset;
		
		// определяем тип файла: tap / baw / wav / rkr
		switch (checkFileExt(name)) {
			case FILE_TAP:
				SetScreen(SCREEN_LOADING);
				Draw();
				TapFilePreload(name, &tapInfo);

				if (tapInfo.status == TAP_VERIFY_STATUS_IO_ERROR) {
					UiShowError(ERROR_READ_FILE, SCREEN_MAIN_MENU);
					return;
				} else if (tapInfo.status != TAP_VERIFY_STATUS_OK) {
					SetScreen(SCREEN_WRONG_TAP);
					return;
				}

				SetScreen(SCREEN_TAP_LOAD);
				FioOpenFile(name);
				strlcpy(currentFileName, name, sizeof(currentFileName));
				fileSize = FioGetFileSize();
				TapLoadStart();
				TimeReset();
				break;
			
			case FILE_WAV:
				SetScreen(SCREEN_PLAY_WAV);
				strlcpy(currentFileName, name, sizeof(currentFileName));
				Draw();
				switch (playWav(name)) {
					case PLAY_WAV_WRONG_FORMAT:
						UiShowError(ERROR_WRONG_FILE_FORMAT, SCREEN_BROWSE);
						break;
					case PLAY_WAV_IO_ERROR:
						UiShowError(ERROR_READ_FILE, SCREEN_BROWSE);
						break;
				}
				break;
				
			case FILE_BAW:
				SetScreen(SCREEN_LOADING);
				// обязательно надо скопировать имя, иначе, оно может быть разрушено вызовом BawGetDuration())
				strlcpy(currentFileName, name, sizeof(currentFileName));
				Draw();
				playedFileDuration = BawGetDuration(currentFileName);
				
				if (playedFileDuration == BAW_PLAYER_IO_ERROR) {
					UiShowError(ERROR_READ_FILE, SCREEN_BROWSE);
					return;
				} else if (playedFileDuration == BAW_PLAYER_FORMAT_ERROR) {
					UiShowError(ERROR_READ_FILE, SCREEN_BROWSE);
					return;
				}
				playBaw(currentFileName);
				TimeReset();
				SetScreen(SCREEN_PLAY_BAW);
				break;
				
			case FILE_RKR:
				SetScreen(SCREEN_LOADING);
				Draw();
				switch (RkrFilePreload(name)) {
					case RKR_STATUS_OK:
						strlcpy(currentFileName, name, sizeof(currentFileName));
						SetScreen(SCREEN_PLAY_RKR);
						RkrPlayFile(currentFileName);
						break;
					case RKR_STATUS_READ_ERROR:
						UiShowError(ERROR_READ_FILE, SCREEN_BROWSE);
						return;
					case RKR_STATUS_FORMAT_ERROR:
						UiShowError(ERROR_WRONG_FILE_FORMAT, SCREEN_BROWSE);
						return;
					case RKR_STATUS_CRC_ERROR:
						UiShowError(ERROR_WRONG_CRC, SCREEN_BROWSE);
						return;					
				}
				break;
		}
	}
}

static void browserShowFileMenu(const char* fileName) {
	SetScreen(SCREEN_FILE_MENU);
	strlcpy(currentFileName, fileName, sizeof(currentFileName)-1);
}

static void keyHandlerBrowserScreen(uint8_t key) {
	// Если мы находимся в пустой директории, то можем только вернуться назад
	if (FioGetDirData()->dirSize == 0) {
		if (!gotoBack()) {
			SetScreen(SCREEN_MAIN_MENU);
			selectedIndex = 0;			
		}
		return;
	}
	if (key == KEY_UP) {
		if (selectedIndex == 0) {	// если выделен первый элемент списка и надо прокрутить список вверх
			// перемещаемся по буферу
			if (firstVisibleItem > 0) {	// если есть куда перемещаться в буфере
				firstVisibleItem--;
			} else if (FioGetDirData()->offset > 0) {	// если буфер смотрит не на первый элемент директории, то перечитываем данные с карты
				// вычисляем смещение с которого надо прочитать каталог
				uint16_t d2 = FioGetDirData()->dataSize/2;
				//uint32_t loadFromOffset = FioGetDirData()->offset > d2 ? FioGetDirData()->offset - d2 : 0;
				uint32_t loadFromOffset;
				if (FioGetDirData()->offset + firstVisibleItem > d2) {
					loadFromOffset = FioGetDirData()->offset + firstVisibleItem - d2;
					} else {
					loadFromOffset = 0;
				}
				firstVisibleItem = FioGetDirData()->offset - loadFromOffset - 1;		// после завершения чтения
				refreshBrowserFrom(loadFromOffset);
			}
		} else {	// если выделен не первый элемент каталога, то достаточно просто переместить курсор
			selectedIndex--;
		}
	} else if (key == KEY_DOWN) {
		// нельзя выделить элемент больший, чем количество файлов в директории
		if (selectedIndex + firstVisibleItem >= FioGetDirData()->dirSize-1) {
			return;
		}
		// если курсор не выходит за пределы экрана
		if (selectedIndex < LINES_PER_SCREEN-1) {
			selectedIndex++;	// ... то просто перемещаем курсор вниз
		} else {
			// если в буфере есть данные для прокрутки на элемент вперед
			if (firstVisibleItem < FioGetDirData()->dataSize - LINES_PER_SCREEN) {
				firstVisibleItem++;
			} else if (FioGetDirData()->offset + FioGetDirData()->dataSize <= FioGetDirData()->dirSize-1) {	// если конец буфера еще не достиг конца директории
				uint16_t d2 = FioGetDirData()->dataSize/2;
				//uint32_t loadFromOffset = FioGetDirData()->offset + d2 < FioGetDirData()->dirSize ? FioGetDirData()->offset + d2 : FioGetDirData()->dirSize - d2 - 1;
				uint32_t loadFromOffset;
				if (FioGetDirData()->offset + firstVisibleItem > d2) {
					loadFromOffset = FioGetDirData()->offset + firstVisibleItem - d2;
				} else {
					loadFromOffset = 0;
				}
				// подгружаем следующие элементы директории
				firstVisibleItem += FioGetDirData()->offset - loadFromOffset + 1;
				refreshBrowserFrom(loadFromOffset);
			}
		}
	} else if (key == KEY_LEFT) {
		if (!gotoBack()) {
			// выход в меню если уже достигнут корень
			SetScreen(SCREEN_MAIN_MENU);
			selectedIndex = 0;
		}
	} else if (key == KEY_RIGHT) {
		browserOpenItem(browserGetSelectedFile());
	} else if (key == KEY_ENTER) {
		browserShowFileMenu(browserGetSelectedFile());
	}
}


static void keyHandlerPlayTapScreen(uint8_t key) {
	// если загружаемый файл закончился
	if (TapGetProcessedBytes() >= fileSize) {
		if (key == KEY_LEFT || key == KEY_RIGHT) {
			MSG("<=");
			//FioClose();
			if (fio_error_mask) {
				UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
			} else {
				SetScreen(SCREEN_BROWSE);
			}
		}
	} else {		// загрузка файла в процессе
		switch (key) {
			case KEY_LEFT:
				// прервать загрузку
				TapLoadStop();
MSG_DEC("saved offset ", savedFileOffset);								
				FioReadDirFrom(savedFileOffset);
				if (fio_error_mask) {
					UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
				} else {
					SetScreen(SCREEN_BROWSE);
				}
				// перечитать директорию и сфокусироваться на файле
				//FioInit();				
MSG_DEC("back first visible ", firstVisibleItem);
MSG_DEC("selected index ", selectedIndex);
MSG_DEC("dir size ", FioGetDirData()->dirSize);
MSG_DEC("offset ", FioGetDirData()->offset);
				break;
			case KEY_RIGHT:
				// приостановить / возобновить загрузку
				if (!TapLoadIsPaused()) {
					TapLoadPause();
				} else {
					TapLoadResume();
				}
				break;
			case KEY_UP:
				if (TapLoadIsPaused()) {
					if (currentBlockNumber < numberOfBlocks) {
						currentBlockNumber++;
					}
				}
				break;
			case KEY_DOWN:
				if (TapLoadIsPaused()) {
					if (currentBlockNumber > 1) {
						currentBlockNumber--;
					}
				}
				break;
			case KEY_ENTER:
				break;
		}
	}
}


static void keyHandlerTapSaveScreen(uint8_t key) {
	if (key == KEY_LEFT) {
		fileSize = TapSaveStop();
		
		//FioInit();
		TapFilePreload(currentFileName, &tapInfo);
		//FioClose();
		SetScreen(SCREEN_SAVE_TAP_DONE);
	}
}



static void keyHandlerMainMenuScreen(uint8_t key) {
	switch (key) {
		case KEY_UP:
			if (selectedIndex > 0) {
				selectedIndex--;
			} else {
				selectedIndex = 4;
			}
			break;
		case KEY_DOWN:
			if (selectedIndex < 4) {
				selectedIndex++;
			} else {
				selectedIndex = 0;
			}
			break;
		case KEY_RIGHT:
		case KEY_ENTER:
			if (selectedIndex == 0) {
				if (FioCheckSdCard()) {
					SetScreen(SCREEN_BROWSE);
					refreshBrowser();
				} else {
					SetScreen(SCREEN_INSERT_CARD);
				}				
			} else if (selectedIndex == 1) {
				if (FioCheckSdCard()) {
					SetScreen(SCREEN_SELECT_SAVE_FORMAT);
					//SetScreen(SCREEN_KEYBOARD);
					//currentFileName[0] = 0;
					selectedIndex = 0;
				} else {
					SetScreen(SCREEN_INSERT_CARD);
				}
			} else if (selectedIndex == 2) {
				SetScreen(SCREEN_SETTINGS);
				firstVisibleItem = 0;
				selectedIndex = 0;
			} else if (selectedIndex == 3) {
				SetScreen(SCREEN_LOADING);
				Draw();
				if (FioCheckSdCard()) {
					SetScreen(SCREEN_CARD_INFO);
					selectedIndex = 0;
					FioReadDiskInfo();
				} else {
					SetScreen(SCREEN_INSERT_CARD);
				}
			} else if (selectedIndex == 4) {
				SetScreen(SCREEN_ABOUT);
			}
			break;
	}
}


static void keyHandlerSettingsScreen(uint8_t key) {
	switch (key) {
		case KEY_LEFT:
			selectedIndex = 2;
			SetScreen(SCREEN_MAIN_MENU);
			SettingSave();
			break;
		case KEY_RIGHT:
			switch (firstVisibleItem + selectedIndex) {
				case SETTINGS_ITEM_LANG:
					SetLang(GetLang() == LANG_EN ? LANG_RU : LANG_EN);
					break;
				case SETTINGS_ITEM_SOUND_KEY:
					SetSoundKey(!GetSoundKey());
					break;
				case SETTINGS_ITEM_SOUND_PLAY:
					SetSoundPlay(!GetSoundPlay());
					break;
				case SETTINGS_ITEM_SOUND_REC:
					SetSoundRec(!GetSoundRec());
					break;
				case SETTINGS_ITEM_BACKLIGHT:
					if (GetHighlight() == HIGHLIGHT_OFF) {
						SetHighlight(HIGHLIGHT_ON);
					} else if (GetHighlight() == HIGHLIGHT_ON) {
						SetHighlight(HIGHLIGHT_AUTO);
					} else {
						SetHighlight(HIGHLIGHT_OFF);
					}
					break;
				case SETTINGS_ITEM_SPEED:
					if (GetTapSpeed() == SPEED_8X) {
						SetTapSpeed(SPEED_1X);
					} else {
						SetTapSpeed(GetTapSpeed()+1);
					}
					break;
				case SETTINGS_ITEM_TAP_TIMES:
					SetScreen(SCREEN_TAP_SETTINGS);
					editMode = false;
					selectedIndex = 0;
					firstVisibleItem = 0;
					break;
#ifdef SETTINGS_ITEM_BAW_SETTINGS					
				case SETTINGS_ITEM_BAW_SETTINGS:
					SetScreen(SCREEN_BAW_SETTINGS);
					selectedIndex = 0;
					firstVisibleItem = 0;					
					break;
#endif
#ifdef SETTINGS_ITEM_RKR_SETTINGS
				case SETTINGS_ITEM_RKR_SETTINGS:
					SetScreen(SCREEN_RKR_SETTINGS);
					selectedIndex = 0;
					firstVisibleItem = 0;					
					break;
#endif
				case SETTINGS_ITEM_DATETIME:
					SetScreen(SCREEN_DATETIME);
#if USE_RTC_8583
					pcf8583_GetDateTime(GetDateTime());
#elif USE_RTC_8563
					pcf8563_GetDateTime(GetDateTime());
#endif				
					selectedIndex = 0;
					break;
				case SETTINGS_ITEM_CONTRAST:
					SetScreen(SCREEN_SETUP_CONTRAST);
					break;
			
			}
			break;
		case KEY_UP:
			if (selectedIndex > SETTINGS_ITEM_FIRST) {
				selectedIndex--;
			} else if (firstVisibleItem > 0) {
				firstVisibleItem--;
			}
			break;
		case KEY_DOWN:
			if (selectedIndex < 4) {
				selectedIndex++;
			} else if (firstVisibleItem < SETTINGS_ITEM_LAST+1-5) {
				firstVisibleItem++;
			}
			break;
	}
}


static void keyHandlerTimeSettingsScreen(uint8_t key) {
	switch (key) {
		case KEY_LEFT:
			if (editMode) {
				editMode = false;
			} else {
				firstVisibleItem = 2;	// TODO !!!! use constants
				selectedIndex = 4;
				SetScreen(SCREEN_SETTINGS);
				// TODO update settings
			}
			break;
		case KEY_RIGHT:
			switch (firstVisibleItem + selectedIndex) {
				case 0:	// Pause
				case 1:	// Pause short
				case 2:	// Pilot block
				case 3:	// Pilot pulse
				case 4:	// Sync 1
				case 5:	// Sync 2
				case 6:	// Data
					if (editMode) {
						setTimingValue(firstVisibleItem + selectedIndex, editedValue);
						editMode = false;
					} else {
						editedValue = getTimingValue(firstVisibleItem + selectedIndex);
						editMode = true;
					}
					break;
				case 7:
					SetCapture(GetCapture() == CAPTURE_FALL ? CAPTURE_RISE : CAPTURE_FALL);
					break;
				case 8:	// Detect
					SetScreen(SCREEN_WAIT_SAVE);
					selectedSaveFormat = FILE_UNKNOWN;
					break;
				case 9:	// Default
					SetScreen(SCREEN_CONFIRM_DEFAULT_TIMES);
					break;
			}
			break;
		case KEY_UP:
			if (editMode) {
				uint8_t delta = selectedIndex < 3 ? 10 : 1;
				editedValue += delta;
				uint16_t maxValue = pgm_read_word(&MAX_TAP_TIMING_VALUES[firstVisibleItem + selectedIndex]);
				if (editedValue > maxValue) {
					editedValue = maxValue;
				}
			} else if (selectedIndex > 0) {
				selectedIndex--;
			} else if (firstVisibleItem > 0) {
				firstVisibleItem--;
			}
			break;
		case KEY_DOWN:
			if (editMode) {
				uint8_t delta = selectedIndex < 3 ? 10 : 1;
				editedValue -= delta;
				uint16_t minValue = pgm_read_word(&MIN_TAP_TIMING_VALUES[firstVisibleItem + selectedIndex]);
				if (editedValue < minValue) {
					editedValue = minValue;
				}
			} else if (selectedIndex < 4) {
				selectedIndex++;
			} else if (firstVisibleItem < 10-5) {
				firstVisibleItem++;
			}
			break;
	}
}


static void keyHandlerConfirmDefaultTimeSettingsScreen(uint8_t key) {
	if (key == KEY_LEFT) {
		SetScreen(SCREEN_TAP_SETTINGS);
		firstVisibleItem = 5;
		selectedIndex = 4;
	} else if (key == KEY_RIGHT) {
		SetScreen(SCREEN_TAP_SETTINGS);
		firstVisibleItem = 5;
		selectedIndex = 4;
		SetupDefaultTimings();
	}
}


static void keyHandlerCardInfoScreen(uint8_t key) {
	switch (key) {
		case KEY_UP:
			if (selectedIndex > 0) {
				selectedIndex--;
			}
			break;
		case KEY_DOWN:
			if (selectedIndex < 3) {
				selectedIndex++;
			}
			break;
		default:
			selectedIndex = 3;
			SetScreen(SCREEN_MAIN_MENU);
	}
}


static void keyHandlerAboutScreen(uint8_t key) {
	SetScreen(SCREEN_MAIN_MENU);
}

static void keyHandlerWrongTapScreen(uint8_t key) {
	SetScreen(SCREEN_BROWSE);
}

static void keyHandlerInsertCardScreen(uint8_t key) {
	SetScreen(SCREEN_MAIN_MENU);
}

static void keyHandlerErrorScreen(uint8_t key) {
	if (errorMsg == ERROR_FIO_ERROR) {
		fio_error_cnt = 0;
		fio_error_mask = 0;
	}
	SetScreen(errorBackScreen);	
}

static void keyHandlerPlayWawScreen(uint8_t key) {
	if (key == KEY_LEFT) {
		closeWav();
	}	
}

static void keyHandlerWaitSaveScreen(uint8_t key) {
	if (key == KEY_LEFT) {
		if (selectedSaveFormat == FILE_TAP) {
			TapSaveCancel();
		} else if (selectedSaveFormat == FILE_BAW) {
			BawSaveCancel();
		}
		SetScreen(SCREEN_MAIN_MENU);
			selectedIndex = 1;
	} else if (selectedSaveFormat == FILE_BAW && key == KEY_RIGHT) {
		BawSaveStart();
		SetScreen(SCREEN_BAW_SAVE);
		datetime_t *dt = GetDateTime();
		dt->min = 0;
		dt->sec = 0;
		Draw();
	}
}

static void keyHandlerSaveTapDoneScreen(uint8_t key) {
	selectedIndex = 1;
	if (fio_error_mask) {
		UiShowError(ERROR_FIO_ERROR, SCREEN_MAIN_MENU);
	} else {
		SetScreen(SCREEN_MAIN_MENU);
	}	
}

static void keyHandlerKeyboardScreen(uint8_t key) {
	uint8_t len;
	
	switch (key) {
		case KEY_LEFT:
			if (selectedIndex > 0) {
				selectedIndex--;
			}
			break;
		case KEY_RIGHT:
			if (selectedIndex < 40-1) {
				selectedIndex++;
			}
			break;
		case KEY_UP:
			selectedIndex -= 10;
			if (selectedIndex > 100) {
				selectedIndex = 0;
			}
			break;
		case KEY_DOWN:
			selectedIndex += 10;
			if (selectedIndex >= 40) {
				selectedIndex = 39;
			}
			break;
		case KEY_ENTER:
			len = strlen(currentFileName);
			if (selectedIndex == 38) {
				if (len > 0) {
					currentFileName[len-1] = 0;
				}
			} else if (selectedIndex == 39) {
				if (len == 0) {
					selectedIndex = 1;
					SetScreen(SCREEN_MAIN_MENU);
					return;
				}
				// переход к сохранению файла
				if (selectedSaveFormat == FILE_TAP) {
					strcat_P(currentFileName, STR_EXT_TAP);
				} else if (selectedSaveFormat == FILE_WAV) {
					strcat_P(currentFileName, STR_EXT_WAV);
				} else if (selectedSaveFormat == FILE_BAW) {
					strcat_P(currentFileName, STR_EXT_BAW);
				}
				SetScreen(SCREEN_PLEASE_WAIT);
				Draw();
				TapSaveCd();
				wdt_reset();
				FioClose(); FioInit();	// TODO переинициализируем карту памяти чтобы сменить директорию - это не очень здорово
				wdt_reset();
				bool exists = FioFileExists(currentFileName);
				//FioClose();
				if (exists) {
					SetScreen(SCREEN_OVERWRITE_FILE);
				} else {
					if (selectedSaveFormat == FILE_TAP) {
						if (!TapSave(currentFileName)) {
							// удаляем расширение файла
							len = strlen(currentFileName);
							currentFileName[len-4] = 0;
							UiShowError(ERROR_CREATE_FILE, SCREEN_KEYBOARD);
						} else {
							SetScreen(SCREEN_WAIT_SAVE);
						}
					} else if (selectedSaveFormat == FILE_WAV) {
						if (!WavSave(currentFileName)) {
							len = strlen(currentFileName);
							currentFileName[len-4] = 0;
							UiShowError(ERROR_CREATE_FILE, SCREEN_KEYBOARD);
						} else {
							SetScreen(SCREEN_WAIT_SAVE);
						}
					} else if (selectedSaveFormat == FILE_BAW) {
						if (!BawSave(currentFileName)) {
							len = strlen(currentFileName);
							currentFileName[len-4] = 0;
							UiShowError(ERROR_CREATE_FILE, SCREEN_KEYBOARD);
						} else {
							SetScreen(SCREEN_WAIT_SAVE);
						}
					}
				}
			} else {
				if (len < MAX_FILE_NAME_DISPLAY_LEN) {
					currentFileName[len] = getKeyboardChar(selectedIndex / 10, selectedIndex % 10);
					currentFileName[len+1] = 0;
				}
			}
			break;
	}
}


static uint8_t* getDatetimeSelectedVal() {
	switch (selectedIndex) {
		case 0:
			return &(GetDateTime()->day);
		case 1:
			return &(GetDateTime()->month);
		case 3:
			return &(GetDateTime()->hour);
		case 4:
			return &(GetDateTime()->min);
		case 5:
			return &(GetDateTime()->sec);
	}
	return 0;
}


static uint8_t getMaxDatetimeVal() {
	// dd-mm-yyyy
	// hh:mm:ss
	switch (selectedIndex) {
		case 0:
			return GetDaysPerMonth(GetDateTime());
		case 1:
			return 12;
		case 3:
			return 23;
	}
	return 59;
}


static void keyHandlerDateTimeScreen(uint8_t key) {
	switch (key) {
		case KEY_LEFT:	// назад в настройки
			SetScreen(SCREEN_SETTINGS);
			firstVisibleItem = SETTINGS_ITEM_DATETIME - LINES_PER_SCREEN + 1;
			selectedIndex = LINES_PER_SCREEN-1;			
			SettingSave();
			break;
			case KEY_RIGHT:
			if (selectedIndex < 5) {
				selectedIndex++;
			} else {
				selectedIndex = 0;
			}
			break;
		case KEY_UP:
			if (selectedIndex == 2) {	// год
				uint16_t year = GetBaseYear() + GetDateTime()->year;
				if (year < 2100) {
					year++;
				}
				year -= GetBaseYear();
				if (year >= 4) {
					year -= 4;
					SetBaseYear(GetBaseYear() + 4);
				}
				GetDateTime()->year = year;
			} else {
				uint8_t maxValue = getMaxDatetimeVal();
				uint8_t *val = getDatetimeSelectedVal();
				if (*val < maxValue) {
					(*val)++;
				}
			}
#if USE_RTC_8583
			pcf8583_SetDateTime(GetDateTime());
#elif USE_RTC_8563
			pcf8563_SetDateTime(GetDateTime());
#endif
			break;
		case KEY_DOWN:
			if (selectedIndex == 2) {	// год
				uint16_t year = GetBaseYear() + GetDateTime()->year;
				if (year > 2015) {
					year--;
				}
				if (year < GetBaseYear()) {
					SetBaseYear(GetBaseYear() - 4);
					year += 4;
				}
				year -= GetBaseYear();
				GetDateTime()->year = year;
			} else {
				uint8_t *val = getDatetimeSelectedVal();
				if ((selectedIndex != 1 && (*val) > 0) || (*val) > 1) {
					(*val)--;
				}
			}
#if USE_RTC_8583
			pcf8583_SetDateTime(GetDateTime());
#elif USE_RTC_8563
			pcf8563_SetDateTime(GetDateTime());
#endif
			break;
		
		case KEY_ENTER:
			if (GetDateTime()->sec > 30) {
				GetDateTime()->sec = 59;
				GetDateTime()->hsec = 99;
				DateTimeTick100();
			} else {
				GetDateTime()->sec = 0;
			}
#if USE_RTC_8583
			pcf8583_SetDateTime(GetDateTime());
#elif USE_RTC_8563
			pcf8563_SetDateTime(GetDateTime());
#endif
			break;
	}
}


static void keyHandlerFileInfoScreen(uint8_t key) {
	switch (key) {
		case KEY_UP:
			break;
		case KEY_DOWN:
			break;
		case KEY_LEFT:
			SetScreen(SCREEN_BROWSE);
			break;
		case KEY_RIGHT:
			break;
	}
}

static void keyHandlerOverwriteFileScreen(uint8_t key) {
	uint8_t len;
	
	switch (key) {
		case KEY_LEFT:	// Нет
			// удаляем расширение файла
			len = strlen(currentFileName);
			currentFileName[len-4] = 0;
			SetScreen(SCREEN_KEYBOARD);
			break;
		case KEY_RIGHT:	// Да
			if (selectedSaveFormat == FILE_TAP) {
				if (TapSave(currentFileName)) {
					SetScreen(SCREEN_WAIT_SAVE);
				} else {
					// удаляем расширение файла
					len = strlen(currentFileName);
					currentFileName[len-4] = 0;

					UiShowError(ERROR_CREATE_FILE, SCREEN_KEYBOARD);
				}
			} else if (selectedSaveFormat == FILE_BAW) {
				if (BawSave(currentFileName)) {
					SetScreen(SCREEN_WAIT_SAVE);
				} else {
					// удаляем расширение файла
					len = strlen(currentFileName);
					currentFileName[len-4] = 0;

					UiShowError(ERROR_CREATE_FILE, SCREEN_KEYBOARD);
				}				
			}
			break;
	}
}


static void keyHandlerDisplayContrastScreen(uint8_t key) {
	uint8_t contrast = GetDisplayContrast();
	switch (key) {
		case KEY_UP:
			if (contrast < 128) {
				contrast++;
				SetDisplayContrast(contrast);
				if (selectedIndex == 0) {	// если пришли сюда при запуске
					beep(1000, 50);
				}
			} else {
				beep(1000, 1500);
			}
			break;
		case KEY_DOWN:
			if (contrast > 0) {
				contrast--;
				SetDisplayContrast(contrast);
				if (selectedIndex == 0) {	// если пришли сюда при запуске
					beep(1000, 50);
				}
			} else {
				beep(1000, 1500);
			}
			break;
		case KEY_ENTER:
			contrast = (contrast + 10) % 128;
			SetDisplayContrast(contrast);
			if (selectedIndex == 0) {	// если пришли сюда при запуске
					beep(1000, 150);
				}
			break;
		case KEY_RIGHT:
			SettingSave();
			if (selectedIndex == 0) {		// если пришли сюда при запуске
				beep(700, 50);
				beep(1000, 50);
				beep(1500, 30);
				beep(2000, 20);
				beep(1500, 30);
				beep(1000, 50);
			}
		case KEY_LEFT:
			if (selectedIndex == 0) {
				SetScreen(SCREEN_MAIN_MENU);
			} else {
				SetScreen(SCREEN_SETTINGS);
			}
			break;
			
	}
}


static void keyHandlerSelectSaveFormatScreen(uint8_t key) {
	switch (key) {
		case KEY_UP:
			if (selectedIndex > 0) {
				selectedIndex--;
			}
			break;
		case KEY_DOWN:
			if (selectedIndex < 1) {
				selectedIndex++;
			}
			break;
		case KEY_LEFT:
			SetScreen(SCREEN_MAIN_MENU);
			selectedIndex = 1;
			break;
		case KEY_RIGHT:
		case KEY_ENTER:
			selectedSaveFormat = FILE_TAP + selectedIndex;
			selectedIndex = 0;			
			SetScreen(SCREEN_KEYBOARD);
			currentFileName[0] = 0;
			break;
	}
}


static void keyHandlerScreenSaveWav(uint8_t key) {
	WavSaveStop();
	selectedIndex = 1;
	if (fio_error_mask) {
		UiShowError(ERROR_FIO_ERROR, SCREEN_MAIN_MENU);
	} else {
		SetScreen(SCREEN_MAIN_MENU);
	}
}


static void keyHandlerSaveBawScreen(uint8_t key) {
	selectedIndex = 1;	
	if (!BawSaveStop(false)) {
		UiShowError(ERROR_WRITE_FILE, SCREEN_MAIN_MENU);
		return;
	}	// TODO check error code
	if (fio_error_mask) {
		UiShowError(ERROR_FIO_ERROR, SCREEN_MAIN_MENU);
	} else {
		SetScreen(SCREEN_MAIN_MENU);
	}
}


static void keyHandlerPlayBawScreen(uint8_t key) {
	switch (key) {
		case KEY_LEFT:
			SetScreen(SCREEN_LOADING);
			Draw();
			//if (BawPlayingInProgress()) {
			closeBaw();
			//}
			FioReadDirFrom(savedFileOffset);
			if (fio_error_mask) {
				UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
			} else {
				SetScreen(SCREEN_BROWSE);
			}
			break;
		case KEY_RIGHT:
			// если чтение файла закончено
			if (!FioIsFileOpen()) {
				FioReadDirFrom(savedFileOffset);
				if (fio_error_mask) {
					UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
				} else {
					SetScreen(SCREEN_BROWSE);
				}
			}
			// приостановить / возобновить загрузку
			if (!pause_mode) {
				// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
				GetDateTime()->year = GetDateTime()->min;
				GetDateTime()->month = GetDateTime()->sec;				
				BawLoadPause();
			} else {
				BawLoadResume();
				// для экономии памяти будем временно использовать переменные года и месяца для последующего отображения времени после окончания загрузки
				GetDateTime()->min = GetDateTime()->year;
				GetDateTime()->sec = GetDateTime()->month;
			}
			break;
		case KEY_UP:
			if (!BawPlayingInProgress()) {
				if (currentBlockNumber < numberOfBlocks) {
					currentBlockNumber++;
					BawUpdatePlayerTime();
				}
			}
			break;
		case KEY_DOWN:
			if (!BawPlayingInProgress()) {
				if (currentBlockNumber > 1) {
					currentBlockNumber--;
					BawUpdatePlayerTime();
				}
			}
			break;
		case KEY_ENTER:
			break;
	}
}

static void keyHandlerBawSettingsScreen(uint8_t key) {
	uint8_t index = firstVisibleItem + selectedIndex;
	switch (key) {
		case KEY_LEFT:
			if (editMode) {
				editMode = false;
			} else {
				firstVisibleItem = SETTINGS_ITEM_BAW_SETTINGS - LINES_PER_SCREEN + 1;
				selectedIndex = LINES_PER_SCREEN-1;
				SetScreen(SCREEN_SETTINGS);
				// TODO update settings
			}
			break;
		case KEY_RIGHT:
			switch (index) {
				case 0:	// SampleRate
				case 1:	// Min block pause
				case 3:	// Autostop time
					if (editMode) {
						if (index == 0) {
							SetBawSampleRateIndex(editedValue);
						} else {
							setTimingValue(index, editedValue);
						}
						editMode = false;
					} else {
						editedValue = index == 0 ? GetBawSampleRateIndex() : getTimingValue(index);
						editMode = true;
					}
					break;
				case 2:	// Autostop
					SetBawAutoStop(!GetBawAutoStop());
					break;
//				case 4:	// Default
//					SetScreen(SCREEN_CONFIRM_DEFAULT_TIMES);
//					break;
			}
			break;
		case KEY_UP:
			if (editMode) {
				uint8_t delta = pgm_read_byte(&BAW_TIMINGS_DELTA[index]);
				editedValue += delta;
				uint16_t maxValue = pgm_read_word(&MAX_BAW_TIMING_VALUES[index]);
				if (editedValue > maxValue) {
					editedValue = maxValue;
				}
			} else if (selectedIndex > 0) {
				selectedIndex--;
//			} else if (firstVisibleItem > 0) {
//				firstVisibleItem--;
			}
			break;
		case KEY_DOWN:
			if (editMode) {
				uint8_t delta = pgm_read_byte(&BAW_TIMINGS_DELTA[index]);
				uint16_t minValue = pgm_read_word(&MIN_BAW_TIMING_VALUES[index]);
				if (editedValue > minValue) {
					editedValue -= delta;
				}
				if (editedValue < minValue) {
					editedValue = minValue;
				}
			} else if (selectedIndex < 3) {
				selectedIndex++;
//			} else if (firstVisibleItem < 5-5) {
//				firstVisibleItem++;
			}
			break;
	}
}


static void keyHandlerRkrSettingsScreen(uint8_t key) {
	uint8_t index = firstVisibleItem + selectedIndex;
	switch (key) {
		case KEY_LEFT:
			if (editMode) {
				editMode = false;
			} else {
				firstVisibleItem = SETTINGS_ITEM_RKR_SETTINGS - LINES_PER_SCREEN + 1;
				selectedIndex = LINES_PER_SCREEN-1;
				SetScreen(SCREEN_SETTINGS);
			}
			break;
		case KEY_RIGHT:
			switch (index) {
				case 0:	// Baudrate
				case 1:	// Padding
					if (editMode) {
						if (index == 0) {
							SetRkrBaudrate(editedValue);
						} else {
							SetRkrPadding(editedValue);
						}
						editMode = false;
					} else {
						editedValue = index == 0 ? GetRkrBaudrate() : GetRkrPadding();
						editMode = true;
					}
					break;
			}
			break;
		case KEY_UP:
			if (editMode) {
				uint8_t delta = pgm_read_byte(&RKR_TIMINGS_DELTA[index]);
				editedValue += delta;
				uint16_t maxValue = pgm_read_word(&MAX_RKR_TIMING_VALUES[index]);
				if (editedValue > maxValue) {
					editedValue = maxValue;
				}
			} else if (selectedIndex > 0) {
				selectedIndex--;
//			} else if (firstVisibleItem > 0) {
//				firstVisibleItem--;
			}
			break;
		case KEY_DOWN:
			if (editMode) {
				uint8_t delta = pgm_read_byte(&RKR_TIMINGS_DELTA[index]);
				uint16_t minValue = pgm_read_word(&MIN_RKR_TIMING_VALUES[index]);
				if (editedValue > minValue) {
					editedValue -= delta;
				}
				if (editedValue < minValue) {
					editedValue = minValue;
				}
			} else if (selectedIndex < 1) {
				selectedIndex++;
//			} else if (firstVisibleItem < 5-5) {
//				firstVisibleItem++;
			}
			break;
	}
	
}


static void keyHandlerPlayRkrScreen(uint8_t key) {
	if (key == KEY_LEFT || !RkrPlayingInProgress()) {
		SetScreen(SCREEN_LOADING);
		Draw();
		RkrClose();
		FioReadDirFrom(savedFileOffset);
		if (fio_error_mask) {
			UiShowError(ERROR_FIO_ERROR, SCREEN_BROWSE);
		} else {
			SetScreen(SCREEN_BROWSE);
		}
	}
}

static void keyHandlerVoid(uint8_t key) {
	
}
#endif // UI_KEYBOARD_H_