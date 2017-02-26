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
	uint16_t dataSize;	// Количество элементов в буфере (не символов!)
	uint32_t dirSize;	// Количество элементов в директории
	uint32_t offset;	// Порядковый номер файла в директории, который является первым в буфере
};


// задаёт текущую директорию
void FioSetPath(const char* path);
void FioSetPath_P(const char *path);

// получает текущую директорию
char* FioGetPath();

// обновляет текущую директорию - заходит в указанный каталог
void FioGotoSubdir(const char *dir);
// обновляет текущую директорию - выходит в родительский каталог
// возвращает true в случае успеха и false, если текущий каталог и так корневой
bool FioGotoBack();

bool FioInit();
void FioClose();

// Открыть файл для чтения
bool FioOpenFile(const char* fileName);

bool FioOpenFileAsync(const char* fileName);

// Создать или открыть файл для записи
bool FioCreateFile(const char* fileName);

// Увеличить размер созданного файла чтобы он вмещал указанное количество байт
bool FioAllocSize(uint32_t bytes);

uint32_t FioGetFileSize();
void FioCloseFile();
uint8_t FioNextByte();
void FioPutByte(uint8_t byte);


bool FioFileExists(const char* fileName);

// Записать все накопившиеся байты, сбросить буферы на диск
void FioFlush();

// Задает размер файла
bool FioResizeFileTo(uint32_t size);
// Задает размер файла по текущей позиции
uint32_t FioResizeFile();

// Получает текущую позицию в файле
void FioGetFilePos(uint32_t *offset);

// Задает позицию от начала файла
bool FioSeekFromStart(uint32_t offset);

// Задает позицию от конца файла
bool FioSeekFromEnd(uint32_t offset);

void FioOnIdle();
void FioReadDirFrom(uint32_t offset);
// Возвращает номер поддиректории в директории, если не найдено, возвращает 0xffffffff
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
 * Возвращает степень занятости буферов записи (0..255)
 *    0   - места в буферах много, вычислительных ресурсов, вероятно, достаточно для других задач
 *    255 - нет места по крайней мере в одном буфере, необходимо срочно выполнить запись на карту, система не успевает записывать
 */ 
uint8_t FioGetWriteBusyValue();

#endif // _FILEIO_H_