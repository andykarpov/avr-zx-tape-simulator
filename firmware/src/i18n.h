/*
 * IncFile1.h
 *
 * Created: 03.06.2015 22:28:00
 *  Author: Trol
 */ 


#ifndef I18N_H_
#define I18N_H_

#include "settings.h"

#define DEFSTR(name, string)						const char name[] PROGMEM = string;
#define DEFSTRS(name, string_en, string_ru)			const char name##_EN[] PROGMEM = string_en; const char name##_RU[] PROGMEM = string_ru; 

DEFSTRS(STR_STOP, "Stop", "Стоп");
DEFSTRS(STR_PAUSE, "Pause", "Пауза");
DEFSTRS(STR_RESUME, "Resume", "Далее");
DEFSTRS(STR_CANCEL, "Cancel", "Отмена");
DEFSTRS(STR_YES, "Yes", "Да");
DEFSTRS(STR_NO, "No", "Нет");
DEFSTRS(STR_BACK, "Back", "Назад");
DEFSTRS(STR_FORCE, "Force", "Старт");
DEFSTRS(STR_BLOCK, "Block", "Блок");
DEFSTRS(STR_FILE_CORRUPTED, "File broken!", "Файл повреждён!");
DEFSTRS(STR_CRC_ERROR, "CRC error", "Ошибка CRC");
DEFSTRS(STR_READ_ERROR, "Read error", "Ошибка чтения");
DEFSTRS(STR_EMPTY, "<Empty>", "<Пусто>");
DEFSTRS(STR_DONE, "Done", "Готово");
DEFSTRS(STR_LOADING, "Loading...", "Загрузка...");

DEFSTRS(STR_PLAY, "PLAY", "ПРОЧИТАТЬ");
DEFSTRS(STR_SAVE, "SAVE", "СОХРАНИТЬ");
DEFSTRS(STR_RECORD, "RECORD", "ЗАПИСАТЬ");
DEFSTRS(STR_SETTINGS, "SETTINGS", "НАСТРОЙКИ");
DEFSTRS(STR_SD_INFO, "SD INFO", "SD КАРТА");
DEFSTRS(STR_ABOUT, "ABOUT", "ИНФО");

DEFSTRS(STR_LANGUAGE, "Language", "Язык");
DEFSTRS(STR_SOUND_KEY, "Sound key", "Звук клав.");
DEFSTRS(STR_SOUND_PLAY, "Sound play", "Звук вспр.");
DEFSTRS(STR_SOUND_REC, "Sound rec", "Звук зап.");
DEFSTRS(STR_BACKLIGHT, "Backlight ", "Подсветка");							   
DEFSTRS(STR_SPEED, "Speed", "Скорость");
DEFSTRS(STR_TIMINGS_TAP, "TAP", "TAP");
DEFSTRS(STR_SETTINGS_BAW, "BAW", "BAW");
DEFSTRS(STR_SETTINGS_RKR, "RKR", "RKR");
DEFSTRS(STR_DATE_TIME, "Date & time", "Дата и время");
DEFSTRS(STR_DISPLAY_CONTRAST, "Contrast", "Контраст");
//DEFSTRS(STR_CORRECTION, "Correct", "Коррекция");

DEFSTRS(STR_ON, "On", "Вкл");
DEFSTRS(STR_OFF, "Off", "Вык");
DEFSTRS(STR_AUTO, "Auto", "Авто");
DEFSTR(STR_X1, "x1");
DEFSTR(STR_X2, "x2");
DEFSTR(STR_X4, "x4");
DEFSTR(STR_X8, "x8");
DEFSTRS(STR_LANG_EN, "Eng", "Анг");
DEFSTRS(STR_LANG_RU, "Rus", "Рус");
DEFSTRS(STR_FALL, "fall", "спад");
DEFSTRS(STR_RISE, "rise", "фронт");
DEFSTRS(STR_MS, "ms", "мс");
DEFSTRS(STR_SEC, "s", "с");
DEFSTRS(STR_US, "us", "мкс");
DEFSTRS(STR_HZ, "Hz", "Гц");
DEFSTRS(STR_PULSE_CHAR, "\x89", "\x89");

DEFSTRS(STR_PILOT_LEN, "Pilot\x89", "Пилот\x89");
DEFSTRS(STR_PILOT_BLOCK_LEN, "Pilot", "Пилот");
DEFSTRS(STR_SYNC_1, "Sync 1", "Синх.1");
DEFSTRS(STR_SYNC_2, "Sync 2", "Синх.2");
DEFSTRS(STR_DATA, "Data", "Данные");
DEFSTRS(STR_CAPTURE, "Capture", "Захват");
DEFSTRS(STR_DETECT, "Detect", "Определить");
DEFSTRS(STR_DEFAULT, "Default", "По умолчанию");
DEFSTRS(STR_PAUSE_SHORT, "Pause2", "Пауза2");
DEFSTRS(STR_RATE, "Rate", "Част.");
DEFSTRS(STR_MIN_PAUSE, "Pause", "Пауза");
DEFSTRS(STR_AUTOSTOP, "Autostop", "Автостоп");
DEFSTRS(STR_AUTOSTOP_TIME, "if t >", "при t >");
DEFSTRS(STR_BAUDRATE, "Baudrate", "Скорость");

DEFSTRS(STR_CONFIRM_DEFAULT_TIMINGS_1, "Reset settings", "Сбросить");
DEFSTRS(STR_CONFIRM_DEFAULT_TIMINGS_2, "to default?", "настройки?");

DEFSTRS(STR_WAITING_SIGNAL_1, "Waiting for", "Ожидание");
DEFSTRS(STR_WAITING_SIGNAL_2, "the signal..", "сигнала...");

//DEFSTR(STR_APP_NAME, "ZX Taper");
DEFSTR(STR_APP_NAME, "Tape simulator");
DEFSTR(STR_APP_VERSION, "v\x01""e\x01r. 1.1.6");
DEFSTR(STR_APP_COPYRIGHT, "\x85\x86Ole\x01g Trifo\x01n\x01o\x01v");
DEFSTR(STR_APP_YEAR, "2015-2016");
DEFSTR(STR_APP_WEB, "w\x01w\x01w.t\x01r\x01ols\x01o\x01""ft.r\x01u");

DEFSTRS(STR_PILOT, "pilot", "пилот");
DEFSTRS(STR_BYTE_SHORT, "b", "б");
DEFSTRS(STR_BYTES, "bytes", "байт");
DEFSTRS(STR_BLOCKS, "Blocks:", "Блоки:");

DEFSTRS(STR_FREE_MB, "Free, MB", "Свободно, Мб");
DEFSTRS(STR_MANUFACTURER, "Manufacturer", "Производитель");
DEFSTRS(STR_SERIAL_NUMBER, "Serial number", "Сер. номер");
DEFSTRS(STR_SIZE, "Size", "Объем");
DEFSTRS(STR_PRODUCT_NAME, "Product name:", "Имя продукта:");
DEFSTRS(STR_REVISION, "Revision: ", "Ревизия: ");
DEFSTRS(STR_FORMAT, "Format:", "Формат:");
DEFSTR(STR_FORMAT_HARDDISK, "HardDisk (MBR)");
DEFSTR(STR_FORMAT_SUPERFLOPPY, "SuperFloppy");
DEFSTRS(STR_FORMAT_UNIVERSAL, "Universal", "Универсальный");
DEFSTRS(STR_FORMAT_UNKNOWN, "Unknown", "Неизвестен");
DEFSTRS(STR_MB, "MB", "Мб");
DEFSTR(STR_K, "K");
DEFSTR(CHR_SLASH, "/");
DEFSTR(CHR_BRACKET_LEFT, "(");
DEFSTR(CHR_BRACKET_RIGHT, ")");
DEFSTR(CHR_SPACE, " ");


DEFSTR(STR_PARTITION_FREE, "Free");
DEFSTR(STR_PARTITION_FAT12, "FAT12");
DEFSTR(STR_PARTITION_FAT16_32MB, "FAT16 (32Mb)");
DEFSTR(STR_PARTITION_EXTENDED, "Extended");
DEFSTR(STR_PARTITION_FAT16, "FAT16");
DEFSTR(STR_PARTITION_FAT32, "FAT32");
DEFSTR(STR_PARTITION_FAT32_LBA, "FAT32 LBA");
DEFSTR(STR_PARTITION_FAT16_LBA, "FAT16 LBA");
DEFSTR(STR_PARTITION_EXTENDED_LBA, "EXTENDED LBA");

DEFSTRS(STR_PARTITION, "Partition:", "Раздел");
DEFSTRS(STR_SIZES, "Sizes", "Размеры");
DEFSTRS(STR_CLUSTER_SIZE, "Cluster: ", "Кластер: ");
DEFSTRS(STR_SECTOR_SIZE, "Sector: ", "Сектор: ");


DEFSTRS(STR_ALREADY_EXISTS, "Already exists", "Уже существует");
DEFSTRS(STR_REPLACE_IT, "Replace it?", "Заменить его?");
DEFSTRS(STR_CONTRAST, "Contrast", "Контрастность");
DEFSTRS(STR_ERROR, "ERROR", "ОШИБКА")

DEFSTRS(STR_CANT_CREATE_FILE, "Can't create\nfile", "Не могу\nсоздать файл");

DEFSTRS(STR_CANT_READ_FILE, "Can't read\nfile", "Не могу\nпрочитать\nфайл");

DEFSTRS(STR_CANT_WRITE_FILE, "Can't write\nfile", "Не могу\nсохранить\nфайл");

DEFSTRS(STR_WRONG_FILE_FORMAT, "Wrong file\nformat", "Неверный\nформат файла");

DEFSTRS(STR_WRONG_CRC, "CRC error", "Ошибка CRC");

DEFSTRS(STR_INSERT, "Insert", "Вставьте");
DEFSTRS(STR_SD_CARD, "SD-CARD", "карту памяти");
DEFSTRS(STR_SAVE_AS, "SAVE AS", "СОХРАНИТЬ КАК");

DEFSTRS(STR_PLAYING, "Playing", "Загрузка");
DEFSTRS(STR_READ_WRITE_TIMEOUT, "R/W timeout", "Таймаут R/W");
DEFSTRS(STR_READ_TIMEOUT, "Read timeout", "Таймаут чтения");
DEFSTRS(STR_WRITE_TIMEOUT, "Write timeout", "Таймаут записи");
DEFSTRS(STR__ERRORS, " errors", " ошибок")
DEFSTRS(STR_PLEASE_WAIT, "Please\nwait...", "Подождите\nпожалуйста..");

DEFSTR(STR_EXT_TAP, ".TAP");
DEFSTR(STR_EXT_WAV, ".WAV");
DEFSTR(STR_EXT_BAW, ".BAW");

DEFSTR(STR_TAP, "TAP");
DEFSTR(STR_WAV, "WAV");
DEFSTR(STR_BAW, "BAW");


DEFSTR(STR_VALUE_EDIOR_CHAR, "\x80");	// стрелки вверх-вниз
DEFSTR(STR_MENU_CHAR, "\x81");			// стрелка вправо

const char CHAR_RIGHT = '\x81';		// стрелка вправо
const char CHAR_LEFT = '\x82';		// стрелка влево
const char CHAR_UP = '\x83';		// стрелка вверх
const char CHAR_DOWN = '\x84';		// стрелка вниз
const char CHAR_ENTER = '\x87';		// символ enter
const char CHAR_BACKSPACE = '\x88';	// символ backspace

#define i18n(str)		(GetLang() == LANG_EN ? str##_EN : str##_RU)

// c0 eq e2
// MSG("Abc Абв");
//#define STR_READ_ERROR	(GetLang() == LANG_EN ? STR_READ_ERROR_EN : STR_READ_ERROR_RU)

#endif