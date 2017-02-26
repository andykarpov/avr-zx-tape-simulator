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

DEFSTRS(STR_STOP, "Stop", "����");
DEFSTRS(STR_PAUSE, "Pause", "�����");
DEFSTRS(STR_RESUME, "Resume", "�����");
DEFSTRS(STR_CANCEL, "Cancel", "������");
DEFSTRS(STR_YES, "Yes", "��");
DEFSTRS(STR_NO, "No", "���");
DEFSTRS(STR_BACK, "Back", "�����");
DEFSTRS(STR_FORCE, "Force", "�����");
DEFSTRS(STR_BLOCK, "Block", "����");
DEFSTRS(STR_FILE_CORRUPTED, "File broken!", "���� ��������!");
DEFSTRS(STR_CRC_ERROR, "CRC error", "������ CRC");
DEFSTRS(STR_READ_ERROR, "Read error", "������ ������");
DEFSTRS(STR_EMPTY, "<Empty>", "<�����>");
DEFSTRS(STR_DONE, "Done", "������");
DEFSTRS(STR_LOADING, "Loading...", "��������...");

DEFSTRS(STR_PLAY, "PLAY", "���������");
DEFSTRS(STR_SAVE, "SAVE", "���������");
DEFSTRS(STR_RECORD, "RECORD", "��������");
DEFSTRS(STR_SETTINGS, "SETTINGS", "���������");
DEFSTRS(STR_SD_INFO, "SD INFO", "SD �����");
DEFSTRS(STR_ABOUT, "ABOUT", "����");

DEFSTRS(STR_LANGUAGE, "Language", "����");
DEFSTRS(STR_SOUND_KEY, "Sound key", "���� ����.");
DEFSTRS(STR_SOUND_PLAY, "Sound play", "���� ����.");
DEFSTRS(STR_SOUND_REC, "Sound rec", "���� ���.");
DEFSTRS(STR_BACKLIGHT, "Backlight ", "���������");							   
DEFSTRS(STR_SPEED, "Speed", "��������");
DEFSTRS(STR_TIMINGS_TAP, "TAP", "TAP");
DEFSTRS(STR_SETTINGS_BAW, "BAW", "BAW");
DEFSTRS(STR_SETTINGS_RKR, "RKR", "RKR");
DEFSTRS(STR_DATE_TIME, "Date & time", "���� � �����");
DEFSTRS(STR_DISPLAY_CONTRAST, "Contrast", "��������");
//DEFSTRS(STR_CORRECTION, "Correct", "���������");

DEFSTRS(STR_ON, "On", "���");
DEFSTRS(STR_OFF, "Off", "���");
DEFSTRS(STR_AUTO, "Auto", "����");
DEFSTR(STR_X1, "x1");
DEFSTR(STR_X2, "x2");
DEFSTR(STR_X4, "x4");
DEFSTR(STR_X8, "x8");
DEFSTRS(STR_LANG_EN, "Eng", "���");
DEFSTRS(STR_LANG_RU, "Rus", "���");
DEFSTRS(STR_FALL, "fall", "����");
DEFSTRS(STR_RISE, "rise", "�����");
DEFSTRS(STR_MS, "ms", "��");
DEFSTRS(STR_SEC, "s", "�");
DEFSTRS(STR_US, "us", "���");
DEFSTRS(STR_HZ, "Hz", "��");
DEFSTRS(STR_PULSE_CHAR, "\x89", "\x89");

DEFSTRS(STR_PILOT_LEN, "Pilot\x89", "�����\x89");
DEFSTRS(STR_PILOT_BLOCK_LEN, "Pilot", "�����");
DEFSTRS(STR_SYNC_1, "Sync 1", "����.1");
DEFSTRS(STR_SYNC_2, "Sync 2", "����.2");
DEFSTRS(STR_DATA, "Data", "������");
DEFSTRS(STR_CAPTURE, "Capture", "������");
DEFSTRS(STR_DETECT, "Detect", "����������");
DEFSTRS(STR_DEFAULT, "Default", "�� ���������");
DEFSTRS(STR_PAUSE_SHORT, "Pause2", "�����2");
DEFSTRS(STR_RATE, "Rate", "����.");
DEFSTRS(STR_MIN_PAUSE, "Pause", "�����");
DEFSTRS(STR_AUTOSTOP, "Autostop", "��������");
DEFSTRS(STR_AUTOSTOP_TIME, "if t >", "��� t >");
DEFSTRS(STR_BAUDRATE, "Baudrate", "��������");

DEFSTRS(STR_CONFIRM_DEFAULT_TIMINGS_1, "Reset settings", "��������");
DEFSTRS(STR_CONFIRM_DEFAULT_TIMINGS_2, "to default?", "���������?");

DEFSTRS(STR_WAITING_SIGNAL_1, "Waiting for", "��������");
DEFSTRS(STR_WAITING_SIGNAL_2, "the signal..", "�������...");

//DEFSTR(STR_APP_NAME, "ZX Taper");
DEFSTR(STR_APP_NAME, "Tape simulator");
DEFSTR(STR_APP_VERSION, "v\x01""e\x01r. 1.1.6");
DEFSTR(STR_APP_COPYRIGHT, "\x85\x86Ole\x01g Trifo\x01n\x01o\x01v");
DEFSTR(STR_APP_YEAR, "2015-2016");
DEFSTR(STR_APP_WEB, "w\x01w\x01w.t\x01r\x01ols\x01o\x01""ft.r\x01u");

DEFSTRS(STR_PILOT, "pilot", "�����");
DEFSTRS(STR_BYTE_SHORT, "b", "�");
DEFSTRS(STR_BYTES, "bytes", "����");
DEFSTRS(STR_BLOCKS, "Blocks:", "�����:");

DEFSTRS(STR_FREE_MB, "Free, MB", "��������, ��");
DEFSTRS(STR_MANUFACTURER, "Manufacturer", "�������������");
DEFSTRS(STR_SERIAL_NUMBER, "Serial number", "���. �����");
DEFSTRS(STR_SIZE, "Size", "�����");
DEFSTRS(STR_PRODUCT_NAME, "Product name:", "��� ��������:");
DEFSTRS(STR_REVISION, "Revision: ", "�������: ");
DEFSTRS(STR_FORMAT, "Format:", "������:");
DEFSTR(STR_FORMAT_HARDDISK, "HardDisk (MBR)");
DEFSTR(STR_FORMAT_SUPERFLOPPY, "SuperFloppy");
DEFSTRS(STR_FORMAT_UNIVERSAL, "Universal", "�������������");
DEFSTRS(STR_FORMAT_UNKNOWN, "Unknown", "����������");
DEFSTRS(STR_MB, "MB", "��");
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

DEFSTRS(STR_PARTITION, "Partition:", "������");
DEFSTRS(STR_SIZES, "Sizes", "�������");
DEFSTRS(STR_CLUSTER_SIZE, "Cluster: ", "�������: ");
DEFSTRS(STR_SECTOR_SIZE, "Sector: ", "������: ");


DEFSTRS(STR_ALREADY_EXISTS, "Already exists", "��� ����������");
DEFSTRS(STR_REPLACE_IT, "Replace it?", "�������� ���?");
DEFSTRS(STR_CONTRAST, "Contrast", "�������������");
DEFSTRS(STR_ERROR, "ERROR", "������")

DEFSTRS(STR_CANT_CREATE_FILE, "Can't create\nfile", "�� ����\n������� ����");

DEFSTRS(STR_CANT_READ_FILE, "Can't read\nfile", "�� ����\n���������\n����");

DEFSTRS(STR_CANT_WRITE_FILE, "Can't write\nfile", "�� ����\n���������\n����");

DEFSTRS(STR_WRONG_FILE_FORMAT, "Wrong file\nformat", "��������\n������ �����");

DEFSTRS(STR_WRONG_CRC, "CRC error", "������ CRC");

DEFSTRS(STR_INSERT, "Insert", "��������");
DEFSTRS(STR_SD_CARD, "SD-CARD", "����� ������");
DEFSTRS(STR_SAVE_AS, "SAVE AS", "��������� ���");

DEFSTRS(STR_PLAYING, "Playing", "��������");
DEFSTRS(STR_READ_WRITE_TIMEOUT, "R/W timeout", "������� R/W");
DEFSTRS(STR_READ_TIMEOUT, "Read timeout", "������� ������");
DEFSTRS(STR_WRITE_TIMEOUT, "Write timeout", "������� ������");
DEFSTRS(STR__ERRORS, " errors", " ������")
DEFSTRS(STR_PLEASE_WAIT, "Please\nwait...", "���������\n����������..");

DEFSTR(STR_EXT_TAP, ".TAP");
DEFSTR(STR_EXT_WAV, ".WAV");
DEFSTR(STR_EXT_BAW, ".BAW");

DEFSTR(STR_TAP, "TAP");
DEFSTR(STR_WAV, "WAV");
DEFSTR(STR_BAW, "BAW");


DEFSTR(STR_VALUE_EDIOR_CHAR, "\x80");	// ������� �����-����
DEFSTR(STR_MENU_CHAR, "\x81");			// ������� ������

const char CHAR_RIGHT = '\x81';		// ������� ������
const char CHAR_LEFT = '\x82';		// ������� �����
const char CHAR_UP = '\x83';		// ������� �����
const char CHAR_DOWN = '\x84';		// ������� ����
const char CHAR_ENTER = '\x87';		// ������ enter
const char CHAR_BACKSPACE = '\x88';	// ������ backspace

#define i18n(str)		(GetLang() == LANG_EN ? str##_EN : str##_RU)

// c0 eq e2
// MSG("Abc ���");
//#define STR_READ_ERROR	(GetLang() == LANG_EN ? STR_READ_ERROR_EN : STR_READ_ERROR_RU)

#endif