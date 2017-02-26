/*
 * fileio.h
 *
 * Created: 05.06.2015 20:40:52
 *  Author: Trol
 */ 


#ifndef _FILEIO_H_
#define _FILEIO_H_


#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib/sdreader/sd_raw.h"

#define  DIR_DATA_SIZE	550

#define FIO_ERROR_READ_NOT_READY			1
#define FIO_ERROR_WRITE_NOT_READY		2

struct dir_data_struct {
	uint16_t dataSize;	// ���������� ��������� � ������ (�� ��������!)
	uint32_t dirSize;	// ���������� ��������� � ����������
	uint32_t offset;	// ���������� ����� ����� � ����������, ������� �������� ������ � ������
};


// ����� ������� ����������
void FioSetPath(const char* path);
void FioSetPath_P(const char *path);

// �������� ������� ����������
char* FioGetPath();

// ��������� ������� ���������� - ������� � ��������� �������
void FioGotoSubdir(const char *dir);
// ��������� ������� ���������� - ������� � ������������ �������
// ���������� true � ������ ������ � false, ���� ������� ������� � ��� ��������
bool FioGotoBack();

bool FioInit();
void FioClose();

// ������� ���� ��� ������
bool FioOpenFile(const char* fileName);

bool FioOpenFileAsync(const char* fileName);

// ������� ��� ������� ���� ��� ������
bool FioCreateFile(const char* fileName);

// ��������� ������ ���������� ����� ����� �� ������ ��������� ���������� ����
bool FioAllocSize(uint32_t bytes);

uint32_t FioGetFileSize();
void FioCloseFile();
uint8_t FioNextByte();
void FioPutByte(uint8_t byte);


bool FioFileExists(const char* fileName);

// �������� ��� ������������ �����, �������� ������ �� ����
void FioFlush();

// ������ ������ �����
bool FioResizeFileTo(uint32_t size);
// ������ ������ ����� �� ������� �������
uint32_t FioResizeFile();

// �������� ������� ������� � �����
void FioGetFilePos(uint32_t *offset);

// ������ ������� �� ������ �����
bool FioSeekFromStart(uint32_t offset);

// ������ ������� �� ����� �����
bool FioSeekFromEnd(uint32_t offset);

void FioOnIdle();
void FioReadDirFrom(uint32_t offset);
// ���������� ����� ������������� � ����������, ���� �� �������, ���������� 0xffffffff
uint32_t FioGetDirectoryIndex(const char *path);

char* GetDirectoryData();

char *GetOpenFileName();

struct dir_data_struct* FioGetDirData();

//uint32_t* GetDirectorySize();
//uint16_t* GetDirectoryDataSize();
//uint32_t* GetDirectoryDataOffset();

bool FioReadDiskInfo();
struct sd_raw_info* FioGetDiskInfo();
uint32_t FioGetFsFreeSpace();
uint32_t FioGetFsSize();

uint8_t FioReadByte();
uint16_t FioReadWord();
uint32_t FioReadDword();

void FioPutWord(uint16_t w);
void FioPutDword(uint32_t dw);

void FioClearError();
uint8_t FioGetErrorMask();
uint16_t FioGetErrorCount();

void FioEnableFastReadonlyMode();

bool FioIsFileOpen();

void FioSetWriteMode();
void FioSetReadMode();


uint8_t FioGetPartitionType();
uint16_t FioGetClusterSize();
uint16_t FioGetSectorSize();
 
#define FioCheckSdCard() (!(PIND & _BV(7)))

void readNext();

/**
 * ���������� ������� ��������� ������� ������ (0..255)
 *    0   - ����� � ������� �����, �������������� ��������, ��������, ���������� ��� ������ �����
 *    255 - ��� ����� �� ������� ���� � ����� ������, ���������� ������ ��������� ������ �� �����, ������� �� �������� ����������
 */ 
uint8_t FioGetWriteBusyValue();

#endif // _FILEIO_H_