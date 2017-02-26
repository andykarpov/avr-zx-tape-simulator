/*
 * fileio.c
 *
 * Created: 05.06.2015 20:40:41
 *  Author: Trol
 */ 

#ifndef SIMULATION
#include "fileio.h"

#include <avr/io.h>

#include <string.h>


#include "debug.h"

#include "lib/sdreader/fat.h"
#include "lib/sdreader/fat_config.h"
#include "lib/sdreader/partition.h"
#include "lib/sdreader/sd_raw.h"
#include "lib/sdreader/sd_raw_config.h"



#include "general.h"


bool sdcard_raw_inited = false;
struct partition_struct* sdcard_partition = 0;
struct fat_fs_struct* sdcard_fs = 0;
struct fat_dir_struct* sdcard_dd = 0;
struct fat_file_struct* sdcard_fd = 0;

struct fat_file_struct {
	struct fat_fs_struct* fs;
	struct fat_dir_entry_struct dir_entry;
	offset_t pos;
	cluster_t pos_cluster;
};

struct fat_header_struct {
    offset_t size;
    offset_t fat_offset;
    uint32_t fat_size;
    uint16_t sector_size;
    uint16_t cluster_size;
    offset_t cluster_zero_offset;
    offset_t root_dir_offset;
#if FAT_FAT32_SUPPORT
    cluster_t root_dir_cluster;
#endif
};

struct fat_fs_struct {
    struct partition_struct* partition;
    struct fat_header_struct header;
    cluster_t cluster_free;
};


volatile bool task_readNextBlock = false;
volatile bool task_writeNextBlock = false;

// Файловые буферы чтения
uint8_t sdcard_buf_1[512];
uint8_t sdcard_buf_2[512];


// Битовая маска готовности данных в буферах. Если нулевой бит установлен, то первый буфер содержит прочитанные с карты данные, если сброшен - первый буфер готов к чтению данных с карты.
// Аналогично первый бит отвечает за второй буфер
volatile uint8_t fio_readyFlags;

// Тут хранится смещение последнего прочитанного байта в файле
//volatile offset_t fio_offset;

struct dir_data_struct dirData;
char currentDirectory[128];

// Буфер, содержащий имена файлов директории
char fio_data[DIR_DATA_SIZE];

volatile uint8_t fio_error_mask = 0;
volatile uint16_t fio_error_cnt = 0;
volatile bool fio_eof;


extern bool trol_fast_read_mode;

volatile uint8_t fio_buf_part_pos;
volatile uint8_func fio_get_next_byte_handler;
volatile uint8_proc fio_put_next_byte_handler;
volatile void_proc fio_read_eof_handler;

uint8_t fioNextFromBuf1Wait();

void fioNextToBuf1Wait(uint8_t b);
void fioNextToBuf1Start(uint8_t b);
void fioNextToBuf2Start(uint8_t b);
void fioNextToBuf1End(uint8_t b);
void fioNextToBuf2End(uint8_t b);


void readNext();
static void writeNext();
static bool isVisibleFile(const char* name);


char* GetDirectoryData() {
	return fio_data;
}

struct dir_data_struct* FioGetDirData() {
	return &dirData;
}



struct fat_dir_entry_struct sdcard_directory;

cluster_t fat_get_next_cluster(const struct fat_fs_struct* fs, cluster_t cluster_num);
uint32_t getDirSize();



/************************************************************************/
/* Вспомогательные методы                                               */
/************************************************************************/

// Проверяет, что директория содержит файл
static bool find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry) {
	fat_reset_dir(dd);
	while (fat_read_dir(dd, dir_entry) ) {
		if (strcmp(dir_entry->long_name, name) == 0)  {
			fat_reset_dir(dd);
			return true;
		}
	}
	fat_reset_dir(dd);
	return false;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name) {
	struct fat_dir_entry_struct file_entry;
	if (!find_file_in_dir(fs, dd, name, &file_entry)) {
		MSG("FILE NOT FOUND!");
		return 0;
	}
	return fat_open_file(fs, &file_entry);
}

void FioSetPath(const char *path) {
	strcpy(currentDirectory, path);
}

void FioSetPath_P(const char *path) {
	strcpy_P(currentDirectory, path);
}

// получает текущую директорию
char* FioGetPath() {
	return currentDirectory;
}


void FioGotoSubdir(const char *dir) {
	uint8_t len = strlen(currentDirectory);
	if (len > 1) {
		currentDirectory[len] = '/';
		currentDirectory[len+1] = 0;
	}
	strcat(currentDirectory, dir);
}


bool FioGotoBack() {
	uint8_t len = strlen(currentDirectory);
	if (len == 1) {
		return false;
	}
	for (uint8_t i = len-1; ; i--) {
		if (currentDirectory[i] == '/') {
			currentDirectory[i == 0 ? 1 : i] = 0;
			break;
		}
	}
	
	return true;
}


/**
 * Init SD-card device, opens partition and file system
 */
bool FioInit() {
	MSG("FIO-INIT");
	// setup sd card slot
//	MSG("sdcard_raw_init");
	sdcard_raw_inited = sd_raw_init();
	if (!sdcard_raw_inited) {
		MSG("MMC/SD initialization failed");
		return false;
	}
    // open first partition
	MSG("partition_open");
	
	sdcard_partition = partition_open(sd_raw_read, sd_raw_read_interval, sd_raw_write, sd_raw_write_interval, 0);
	if (!sdcard_partition) {
        // If the partition did not open, assume the storage device is a "superfloppy", i.e. has no MBR.
		MSG("partition_open-2");
		sdcard_partition = partition_open(sd_raw_read,
				sd_raw_read_interval,
				sd_raw_write, sd_raw_write_interval,
				-1);
	}
	if (!sdcard_partition) {
		MSG("opening partition failed");
		return false;
	}
	// TODO тут бы проверить тип файловой системы !!!
	MSG_HEX("partition-type ", sdcard_partition->type, 1);
	MSG_HEX("partition-offset ", sdcard_partition->offset, 4);
	MSG_HEX("partition-length ", sdcard_partition->length, 4);

	MSG("fat_open");
	sdcard_fs = fat_open(sdcard_partition);
	if (!sdcard_fs) {
		MSG("opening filesystem failed");
		return false;
	}

	// open root directory
	MSG("fat_get_dir_entry_of_path");
/*
	char *path = currentDirectory;
	while (strlen(path) > 0) {
		uint8_t subDirLen = 0;
		for (uint8_t i = 0; ; i++) {
			if (path[i] == 0) {
				break;
			}
			subDirLen++;			
			if (path[i] == '/') {
				if (subDirLen > 1) {
					subDirLen--;	// убираем конечный слеш если это не корень
				}
				break;
			}
		}
MSG_DEC("len = ", subDirLen);		
		if (subDirLen == 0) {
			break;
		}
		char subDir[32];
		for (uint8_t i = 0; i < subDirLen; i++) {
			subDir[i] = path[i];
		}
		subDir[subDirLen] = 0;
		// если это не корневой каталог, то пропускаем следующий слеш
		if (subDirLen > 1) {
			subDirLen++;
		}
//		if (!strncpy(subDir, path, subDirLen)) {
//			MSG("STRNCPY error");
//		}
MSG("---");
		uart_puts(subDir);
MSG("\n---");
//MSG_DEC("len? ", strlen(subDir));
		if (!fat_get_dir_entry_of_path(sdcard_fs, subDir, &sdcard_directory) ) {
			MSG("/ FAIL!");
			currentDirectory[0] = '/';
			currentDirectory[1] = 0;
			fat_get_dir_entry_of_path(sdcard_fs, currentDirectory, &sdcard_directory);
			break;
		}
		path += subDirLen;
	}
*/	
	if (!fat_get_dir_entry_of_path(sdcard_fs, currentDirectory, &sdcard_directory) ) {
		MSG("/ FAIL!");
		currentDirectory[0] = '/';
		currentDirectory[1] = 0;
		fat_get_dir_entry_of_path(sdcard_fs, currentDirectory, &sdcard_directory);

		// TODO !!!
	}


//	MSG("fat_open_dir");
	sdcard_dd = fat_open_dir(sdcard_fs, &sdcard_directory);
	if (!sdcard_dd) {
		MSG("opening root directory failed");
		return false;
	}
	
	dirData.dirSize = getDirSize();
	MSG_DEC("DIR_SIZE ", dirData.dirSize);
	
	dirData.offset = 0;
	dirData.dataSize = 0;

//	MSG("OK!");
	return true;
}


void FioClose() {
	MSG("FIO-CLOSE");

	if (!sdcard_raw_inited) {
		MSG("sdrd_CLOSE WRONG");
		return;
	}
	if (sdcard_dd) {
		fat_close_dir(sdcard_dd);
		sdcard_dd = 0;
	}
	if (sdcard_fs) {
		fat_close(sdcard_fs);
		sdcard_fs = 0;
	}
	if (sdcard_partition) {
		partition_close(sdcard_partition);
		sdcard_partition = 0;
	}
	fio_read_eof_handler = NULL;
	sdcard_raw_inited = false;
}


bool FioOpenFile(const char* fileName) {
	if (!sdcard_raw_inited) {
		FioInit();
	}
	sdcard_fd = open_file_in_dir(sdcard_fs, sdcard_dd, fileName);
	if (!sdcard_fd) {
		MSG("error opening\n");
		return false;
	}
	MSG_DEC("file is open, size= ", FioGetFileSize());

	fio_readyFlags = 0;
	fio_eof = false;
	
	fio_get_next_byte_handler = &fioNextFromBuf1Wait;
	fio_buf_part_pos = 0;

	task_readNextBlock = true;
	task_writeNextBlock = false;
	trol_fast_read_mode = false;
	
	fio_read_eof_handler = NULL;
	
	readNext();
	
	return true;
}

static uint8_t fioNextAsync() {
	fat_read_file(sdcard_fd, sdcard_buf_1, 1);
	return sdcard_buf_1[0];
}


bool FioOpenFileAsync(const char* fileName){
	if (!sdcard_raw_inited) {
		FioInit();
	}
	sdcard_fd = open_file_in_dir(sdcard_fs, sdcard_dd, fileName);
	if (!sdcard_fd) {
		MSG("error opening\n");
		return false;
	}
	MSG_DEC("file is open, size= ", FioGetFileSize());
	
	fio_get_next_byte_handler = &fioNextAsync;

	task_readNextBlock = false;
	task_writeNextBlock = false;
	trol_fast_read_mode = false;
	
	fio_read_eof_handler = NULL;
	
	return true;	
}


bool FioCreateFile(const char* fileName) {
	struct fat_dir_entry_struct file_entry;
//	uint8_t fat_create_file(struct fat_dir_struct* parent, const char* file, struct fat_dir_entry_struct* dir_entry);
//MSG_DEC("Create ", fat_create_file(sdcard_dd, fileName, &file_entry));
	if (!fat_create_file(sdcard_dd, fileName, &file_entry)) {
		MSG("error create");
		return false;
	}
	MSG("file is created");
	
	if (!sd_raw_sync()) {
		MSG("sync err cf!");
	}
	
	if (!FioOpenFile(fileName)) {
		MSG("can't open created file");
		return false;		
	}
	
	int32_t offset = 0;
//	if (!fat_resize_file(sdcard_fd, offset)) {
//		// ignore error
//		MSG("can't resize file");
//	}
	if (!fat_seek_file(sdcard_fd, &offset, FAT_SEEK_SET)) {
		MSG("error seeking on ");
#if DEBUG		
		uart_putc('\n');
#endif
		return false;
	}
	
	if (!sd_raw_sync()) {
		MSG("sync err cf2!");
	}

	fio_readyFlags = 0;
	fio_put_next_byte_handler = &fioNextToBuf1Wait;
	fio_buf_part_pos = 0;
	
	task_readNextBlock = false;
	task_writeNextBlock = true;
	return true;
}

cluster_t fat_append_clusters(struct fat_fs_struct* fs, cluster_t cluster_num, cluster_t count);

bool FioAllocSize(uint32_t bytes) {
	MSG_DEC("ADD BYTES ", bytes);
	uint16_t clusterSize = sdcard_fd->fs->header.cluster_size;
	MSG_DEC("CLUSTER SIZE ", clusterSize);
	cluster_t clusters = bytes / clusterSize;
	if (bytes % clusterSize) {
		clusters++;
	}
	if (bytes) {
		MSG_DEC("ADD ", clusters);
		cluster_t result = fat_append_clusters(sdcard_fs, 0, clusters);
		MSG_DEC("ADDED ", result);
		return result;
	}
	return true;
}

/************************************************************************/
/* Возвращает размер открытого файла                                    */
/************************************************************************/
uint32_t FioGetFileSize() {
	return sdcard_fd->dir_entry.file_size;
}


/************************************************************************/
/* Закрывает файл                                                       */
/************************************************************************/
void FioCloseFile() {
	MSG("close file");
	// если файл открыт на запись, то сначала сохраняем изменения
	FioFlush();
	if (sdcard_fd) {
		fat_close_file(sdcard_fd);
		sdcard_fd = 0;
	}
	task_readNextBlock = false;
	task_writeNextBlock = false;
}


bool FioFileExists(const char* fileName) {
	sdcard_fd = open_file_in_dir(sdcard_fs, sdcard_dd, fileName);
	bool result = sdcard_fd;
	fat_close_file(sdcard_fd);
	return result;
}


/************************************************************************/
/* Возвращает следующий прочитанный байт из буфера                      */
/************************************************************************/
uint8_t FioNextByte() {
	return (fio_get_next_byte_handler)();
}



/************************************************************************/
/* Заполняет буфер(ы) чтения, если он/они готовы                        */
/************************************************************************/
void readNext() {
	if (!task_readNextBlock || fio_eof) {
		return;
	}

	intptr_t cnt;
	// если первый блок готов к записи в него данных (не читается в данный момент)
	if (!(fio_readyFlags & _BV(0))) {
//MSG_DEC("RN1 ", sdcard_fd->pos);		
		cnt = fat_read_file(sdcard_fd, sdcard_buf_1, sizeof(sdcard_buf_1));
//		fio_offset += cnt;
		fio_readyFlags |= _BV(0);
		if (cnt < sizeof(sdcard_buf_1)) {
			fio_eof = true;
			if (fio_read_eof_handler) {
				fio_read_eof_handler();
				fio_read_eof_handler = NULL;
			}
			return;
		}
	}
	// если второй блок готов к записи в него данных (не читается в данный момент)
	if (!(fio_readyFlags & _BV(1))) {
//MSG_DEC("RN2 ", sdcard_fd->pos);				
		cnt = fat_read_file(sdcard_fd, sdcard_buf_2, sizeof(sdcard_buf_2));
//		fio_offset += cnt;
		fio_readyFlags |= _BV(1);
		if (cnt < sizeof(sdcard_buf_2)) {
			fio_eof = true;
			if (fio_read_eof_handler) {
				fio_read_eof_handler();
				fio_read_eof_handler = NULL;
			}
		}
	}
}


// добавляет байт в очередь на сохранение
void FioPutByte(uint8_t byte) {
	return (fio_put_next_byte_handler)(byte);
}

// сохраняет на диск накопившиеся данные
static void writeNext() {
	if (!task_writeNextBlock) {
		return;
	}
	// если первый блок заполнен и готов к записи на карту
	if (fio_readyFlags & _BV(0)) {
		if (fat_write_file(sdcard_fd, sdcard_buf_1, sizeof(sdcard_buf_1)) != sizeof(sdcard_buf_1)) {
			MSG("WRITE_ERROR");
		}
		if (!sd_raw_sync()) {
			MSG("sync err 1!");
		}
		fio_readyFlags &= ~_BV(0);
		//if ( cnt < sizeof(sdcard_buf_1) ) {}
	}
	// если второй блок заполнен и готов к записи на карту
	if (fio_readyFlags & _BV(1)) {
		if (fat_write_file(sdcard_fd, sdcard_buf_2, sizeof(sdcard_buf_2)) != sizeof(sdcard_buf_2)) {
			MSG("WRITE_ERROR");
		}
		if (!sd_raw_sync()) {
			MSG("sync err 2!");
		}
		fio_readyFlags &= ~_BV(1);
		//if (cnt < sizeof(sdcard_buf_2)) {}
	}
//	MSG_DEC("WRITE_DONE ", readyFlags);	
}


// Записать все накопившиеся байты, сбросить буферы на диск
void FioFlush() {
	if (!sd_raw_sync()) {
		MSG("sync err f!");
	}
//MSG_DEC("BWP ", FioGetFilePos());	
	// если есть полностью заполненные буферы, сохраняем сначала их
	writeNext();
	// если мы не в режиме записи файла
	if (!task_writeNextBlock) {
		return;
	}
	// осталось сохранить текущий блок	
	if (fio_put_next_byte_handler == &fioNextToBuf1Start) {
		if (fat_write_file(sdcard_fd, sdcard_buf_1, fio_buf_part_pos) != fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-1s");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf2Start) {
		if (fat_write_file(sdcard_fd, sdcard_buf_2, fio_buf_part_pos) != fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-2s");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf1End) {
		if (fat_write_file(sdcard_fd, sdcard_buf_1, 0x100+fio_buf_part_pos) != 0x100+fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-1e");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf2End) {
		if (fat_write_file(sdcard_fd, sdcard_buf_2, 0x100+fio_buf_part_pos) != 0x100+fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-2e");
		}
	}
	if (!sd_raw_sync()) {
		MSG("sync err f2!");
	}
//MSG_DEC("AWP ", FioGetFilePos());
	fio_readyFlags = 0;
	fio_buf_part_pos = 0;
	fio_get_next_byte_handler = &fioNextFromBuf1Wait;
	fio_put_next_byte_handler = &fioNextToBuf1Wait;
}


// Задает размер файла
bool FioResizeFileTo(uint32_t size) {
	return fat_resize_file(sdcard_fd, size);
}



// Задает размер файла по текущей позиции
uint32_t FioResizeFile() {
	FioFlush();
	fat_resize_file(sdcard_fd, sdcard_fd->pos);
	return sdcard_fd->pos;
}


// Получает текущую позицию в файле
void FioGetFilePos(uint32_t *offset) {
	*offset = sdcard_fd->pos;
}

// Задает позицию от начала файла
bool FioSeekFromStart(uint32_t offset) {
	int32_t signedOffset = offset;
	bool result = fat_seek_file(sdcard_fd, &signedOffset, FAT_SEEK_SET);
	if (fio_get_next_byte_handler != &fioNextAsync) {
		fio_readyFlags = 0;
		MSG_DEC("FIO OFFSETs ", sdcard_fd->pos);
		fio_get_next_byte_handler = &fioNextFromBuf1Wait;
		fio_buf_part_pos = 0;
		fio_eof = false;
	}
	return result;
}


// Задает позицию от конца файла
bool FioSeekFromEnd(uint32_t offset) {
	int32_t signedOffset = -offset;
	bool result = fat_seek_file(sdcard_fd, &signedOffset, FAT_SEEK_END);
	if (fio_get_next_byte_handler != &fioNextAsync) {
		fio_readyFlags = 0;
		MSG_DEC("FIO OFFSETe ", sdcard_fd->pos);
		fio_get_next_byte_handler = &fioNextFromBuf1Wait;
		fio_buf_part_pos = 0;
		fio_eof = false;
	}
	return result;
}



void FioOnIdle() {
	readNext();
	writeNext();
}

/************************************************************************/
/* Проходится по директории и получает количество элементов в ней       */
/************************************************************************/
uint32_t getDirSize() {
	uint32_t result = 0;
	struct fat_dir_entry_struct dir_entry;
	while (fat_read_dir(sdcard_dd, &dir_entry) ) {
		if (isVisibleFile(dir_entry.long_name)) {
			result++;
		}
	}
	fat_reset_dir(sdcard_dd);
	return result;
}

#if DEBUG
void debugMsg_(const char *text, uint32_t offset, const char *item) {
	uart_puts_p(text);
	uart_putc(' ');
	uart_putc(':');
	uart_putc(' ');
	uart_putdw_dec(offset);
	uart_putc(' ');
	uart_puts(item);
	uart_putc('\n');
}

	#define debugMsg(text, offset, item)	debugMsg_(PSTR(text), offset, item);
#else
	#define debugMsg(text, offset, item)
#endif

/************************************************************************/
/* Читает элементы текущей директории начиная с указанной позиции       */
/************************************************************************/
void FioReadDirFrom(uint32_t offset) {
	if (!sdcard_raw_inited) {
		FioInit();
	}
	// если в буфере уже есть данные для этой позиции, ничего не делаем
	if (offset == dirData.offset && offset != 0) {
		return;
	}
MSG_DEC("read dir from ", offset);
	struct fat_dir_entry_struct file_entry;
	
	uint16_t pos = 0;		// позиция в буфере
	
	// если надо перейти назад
	if (offset < dirData.offset || offset == 0) {
		fat_reset_dir(sdcard_dd);
		for (uint32_t i = 0; i < offset; i++) {
			if (!fat_read_dir(sdcard_dd, &file_entry)) {
MSG_DEC("stop-1 ", i);
				dirData.dataSize = 0;
				return;
			}
debugMsg("SKIP1", i, file_entry.long_name);
			// игнорируем скрытые файлы
			if (!isVisibleFile(file_entry.long_name)) {
				i--;
			}
		}
		dirData.dataSize = 0;
	// если надо перейти немного вперед, так, что возможно использовать уже прочитанные данные
	} else if (offset > dirData.offset && offset < dirData.offset + dirData.dataSize) {
MSG("skip some records:");		
		// определяем смещение первой актуальной записи в буфере
		uint16_t firstPos = 0;		
		for (uint32_t itemOffset = dirData.offset; itemOffset < offset; itemOffset++) {
			// пропускаем имя и завершающий ноль
			while (fio_data[firstPos++] != 0);
//uart_putc(sdcard_ls_data[firstPos-1]);
MSG("");
		}
MSG("");
		// копируем данные в начало буфера и инкрементируем позицию
		uint32_t deltaOffset = offset - dirData.offset;
		uint16_t count = dirData.dataSize - deltaOffset;		// сколько элементов будет скопированы и переиспользованы
		for (uint16_t i = 0; i < count; i++) {
			while (fio_data[firstPos] != 0) {
				fio_data[pos++] = fio_data[firstPos++];
			}
			fio_data[pos++] = fio_data[firstPos++];
		}
		
		dirData.dataSize = count;	
	// если надо перейти вперед
	} else if (offset > dirData.offset) {	// иначе переходим вперед если надо
		for (uint32_t i = dirData.offset + dirData.dataSize; i < offset; i++) {	// !!!!!!  i <= offset дает ошибку при возврате назад из каталога
			if (!fat_read_dir(sdcard_dd, &file_entry)) {
				dirData.dataSize = 0;
MSG_DEC("stop-2 ", i);
				return;
			}
debugMsg("SKIP2", i, file_entry.long_name);			
			// игнорируем скрытые файлы
			if (!isVisibleFile(file_entry.long_name)) {
				i--;
			}
		}
		dirData.dataSize = 0;
	}

	dirData.offset = offset;
	
	while (true) {
		if (!fat_read_dir(sdcard_dd, &file_entry)) {
			break;
		}
debugMsg(">", dirData.offset + dirData.dataSize, file_entry.long_name);		
		if (!isVisibleFile(file_entry.long_name)) {
			continue;
		}
		// если в буфере не хватает места для следующего файла
		uint8_t fnLen = strlen(file_entry.long_name);
		if (file_entry.attributes & FAT_ATTRIB_DIR) {
			fnLen++;
		}
		//if (pos + fnLen >= sizeof(sdcard_ls_data) - 1) {
			//return;
		//}
		if (file_entry.attributes & FAT_ATTRIB_DIR) {
			fio_data[pos++] = '>';
			fnLen--;
		}

		strcpy(fio_data + pos, file_entry.long_name);
		pos += fnLen;
		fio_data[pos++] = 0;
		dirData.dataSize++;
		
		if (pos + 34 >= sizeof(fio_data) - 1) {
			return;
		}
	}
}


/************************************************************************/
/* Возвращает номер поддиректории в директории                          
   Если не найдено, возвращает 0xffffffff                               */
/************************************************************************/
uint32_t FioGetDirectoryIndex(const char *path) {
	fat_reset_dir(sdcard_dd);
	
	struct fat_dir_entry_struct file_entry;
	uint32_t index = 0;
	while (true) {
		if (!fat_read_dir(sdcard_dd, &file_entry)) {
			fat_reset_dir(sdcard_dd);
			dirData.offset = 0;
			dirData.dataSize = 0;
			return 0xffffffff;
		}
		if (!isVisibleFile(file_entry.long_name)) {
			continue;
		}
		if (strcmp(path, file_entry.long_name) == 0) {
			fat_reset_dir(sdcard_dd);
			dirData.offset = 0;
			dirData.dataSize = 0;
			return index;
		}
		index++;
	}
}



static bool isVisibleFile(const char* name) {
	return name[0] != '.';
}

char *GetOpenFileName() {
	return sdcard_fd->dir_entry.long_name;
}


bool FioReadDiskInfo() {
	bool result;
	if (sdcard_raw_inited) {
		FioClose();
	}
	if (FioInit()) {
		struct sd_raw_info* info = (struct sd_raw_info*)sdcard_buf_1;
		result = sd_raw_get_info(info);
		if (!sdcard_partition->has_mbr) {
			info->format = SD_RAW_FORMAT_SUPERFLOPPY;
		}
		uint32_t size = (uint32_t)(fat_get_fs_size(sdcard_fs) / 1024 / 1024);	// размер ФС в МБ
		uint32_t free = (uint32_t)(fat_get_fs_free(sdcard_fs) / 1024 / 1024);	// свободно МБ
		memcpy(sdcard_buf_2+0, &size, 4);
		memcpy(sdcard_buf_2+4, &free, 4);
		memcpy(sdcard_buf_2+8, &sdcard_fs->header.cluster_size, 2);
		memcpy(sdcard_buf_2+10, &sdcard_fs->header.sector_size, 2);
		memcpy(sdcard_buf_2+12, &sdcard_fs->partition->type, 1);
		// 
	} else {
		result = false;
	}
	FioClose();
	return result;
}


struct sd_raw_info* FioGetDiskInfo() {
	return (struct sd_raw_info*)sdcard_buf_1;
}


uint32_t FioGetFsFreeSpace() {
	uint32_t result;
	memcpy(&result, sdcard_buf_2+4, 4);
	return result;
}


uint32_t FioGetFsSize() {
	uint32_t result;
	memcpy(&result, sdcard_buf_2, 4);
	return result;
}


uint8_t FioReadByte() {
	readNext();
	return FioNextByte();
}

void FioClearError() {
	fio_error_mask = 0;
	fio_error_cnt = 0;
}


uint8_t FioGetErrorMask() {
	return fio_error_mask;
}


uint16_t FioGetErrorCount() {
	return fio_error_cnt;
}

void FioEnableFastReadonlyMode() {
	trol_fast_read_mode = true;
}


bool FioIsFileOpen() {
	return sdcard_fd;
}


void FioSetWriteMode() {
	fio_readyFlags = 0;
	fio_put_next_byte_handler = &fioNextToBuf1Wait;
	fio_buf_part_pos = 0;
	
	task_readNextBlock = false;
	task_writeNextBlock = true;
	
}


void FioSetReadMode() {
	fio_readyFlags = 0;
	fio_get_next_byte_handler = &fioNextFromBuf1Wait;
	fio_buf_part_pos = 0;

	task_readNextBlock = true;
	task_writeNextBlock = false;
}


uint16_t FioGetClusterSize() {
	uint16_t result;
	memcpy(&result, sdcard_buf_2+8, 2);
	return result;
}

uint16_t FioGetSectorSize() {
	uint16_t result;
	memcpy(&result, sdcard_buf_2+10, 2);
	return result;
}


uint8_t FioGetPartitionType() {
	uint8_t result;
	memcpy(&result, sdcard_buf_2+12, 1);
	return result;
}


/**
 * Возвращает степень занятости буферов записи (0..255)
 *    0   - места в буферах много, вычислительных ресурсов, вероятно, достаточно для других задач
 *    255 - нет места по крайней мере в одном буфере, необходимо срочно выполнить запись на карту, система не успевает записывать
 */ 
uint8_t FioGetWriteBusyValue() {
	// если первый буфер переполнен
	if (fio_readyFlags & _BV(0)) {
		return 0xff;
	}
	// если второй буфер переполнен
	if (fio_readyFlags & _BV(1)) {
		return 0xff;
	}
	
	if (fio_put_next_byte_handler == &fioNextToBuf1Start || fio_put_next_byte_handler == &fioNextToBuf2Start) {
		return fio_buf_part_pos >> 1;
	} else if (fio_put_next_byte_handler == &fioNextToBuf1End || fio_put_next_byte_handler == &fioNextToBuf2End) {
		return 0x80 + (fio_buf_part_pos >> 1);
	}
	return 0xff;
}


#endif // SIMULATION