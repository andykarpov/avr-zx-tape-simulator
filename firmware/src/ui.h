/*
 * IncFile1.h
 *
 * Created: 02.06.2015 20:59:13
 *  Author: Trol
 */ 


#ifndef _UI_H_
#define _UI_H_

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>


#define SCREEN_MAIN_MENU					0
#define SCREEN_BROWSE						1	// навигация по файлам
#define SCREEN_TAP_LOAD						2	// загрузка файла с карты в Spectrum
#define SCREEN_TAP_SAVE						3	// сохранение файла из Spectrum-а на карту
#define SCREEN_SETTINGS						4	// экран настроек
#define SCREEN_ABOUT							5	// о программе
#define SCREEN_WRONG_TAP					6	// экран информации о том, что TAP-файл поврежден
#define SCREEN_WAIT_SAVE					7	// ожидания сигнала при приеме файла от Spectrum-а
#define SCREEN_SAVE_TAP_DONE				8	// информация о файле после сохранения
#define SCREEN_CARD_INFO					9	// информация о карте памяти
#define SCREEN_TAP_SETTINGS			10	// настройка таймингов TAP
#define SCREEN_CONFIRM_DEFAULT_TIMES	11	// подтверждение сброса настроек сигнала
#define SCREEN_KEYBOARD						12	// экран экранной клавиатуры
#define SCREEN_DATETIME						13	// экран ввода даты и времени
#define SCREEN_FILE_MENU					14	// меню дополнительных команд для файла
#define SCREEN_OVERWRITE_FILE				15	// подтверждение перезаписи файла
#define SCREEN_LOADING						16	// экран загрузки
#define SCREEN_INSERT_CARD					17	// экран "вставьте карту"
#define SCREEN_SETUP_CONTRAST				18	// экран настройки контрастности дисплея
#define SCREEN_ERROR							19	// экран отображения разных ошибок
#define SCREEN_PLAY_WAV						20	// воспроизведение wav-файла
#define SCREEN_SELECT_SAVE_FORMAT		21	// выбор формата сохранения файла
#define SCREEN_WAV_SAVE						22	// процесс сохранения wav-файла
#define SCREEN_PLAY_BAW						23	// воспроизведение baw-файла
#define SCREEN_BAW_SAVE						24	// процесс сохранения baw-файла
#define SCREEN_PLEASE_WAIT					25	// экран ожидания (окончание записи baw)
#define SCREEN_BAW_SETTINGS				26	// настройка таймингов BAW
#define SCREEN_RKR_SETTINGS				27 // настройка таймингов RKR
#define SCREEN_PLAY_RKR						28	// экран воспроизведения RKR

#define ERROR_CREATE_FILE					1	// i/o-ошибка при создании файла
#define ERROR_READ_FILE						2	// i/o-ошибка при чтении файла
#define ERROR_FIO_ERROR						3	// i/o-ошибка модуля fio. сообщение включает описание и количество ошибок
#define ERROR_WRONG_FILE_FORMAT			4	// ошибка формата файла
#define ERROR_WRITE_FILE					5	// i/o-ошибка при сохранении/закрытии файла
#define ERROR_WRONG_CRC						6	// ошибка crc


uint8_t GetScreen();
void SetScreen(uint8_t screen);

void Draw();

void onKeyPressed(uint8_t key);

void UiOnIdle();

const char * UiGetCurrentFileName();

void UiShowError(uint8_t msg, uint8_t backToScreen);

#endif	// _UI_H_