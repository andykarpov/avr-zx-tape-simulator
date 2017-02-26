/*
 * fileio.c
 * 
 * Simulation
 *
 * Created: 05.06.2015 20:40:41
 *  Author: Trol
 */ 

#include "../fileio.h"
#include "display.h"

#define ROOT_DIR	"sdcard"

#include <avr/io.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


#include "../debug.h"


#include "../general.h"


FILE* fileio_fd;
DIR* fileio_dir; 
char fileio_fname[1024];

bool sdcard_raw_inited = false;
struct partition_struct* sdcard_partition = 0;
struct fat_dir_struct* sdcard_dd = 0;



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


//extern bool trol_fast_read_mode;

volatile uint8_t fio_buf_part_pos;
volatile uint8_func fio_get_next_byte_handler;
volatile uint8_proc fio_put_next_byte_handler;
volatile void_proc fio_read_eof_handler;

uint8_t fioNextFromBuf1Wait();

void fioNextToBuf1Wait(uint8_t b);
void fioNextToBuf2Wait(uint8_t b);
void fioNextToBuf1Start(uint8_t b);
void fioNextToBuf2Start(uint8_t b);
void fioNextToBuf1End(uint8_t b);
void fioNextToBuf2End(uint8_t b);

bool isDirExist(const char *str);

void readNext();
static void writeNext();
static bool isVisibleFile(const char* name);


char* GetDirectoryData() {
	return fio_data;
}

struct dir_data_struct* FioGetDirData() {
	return &dirData;
}


uint32_t getDirSize();



/************************************************************************/
/* Вспомогательные методы                                               */
/************************************************************************/



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
	// "/1"
	for (uint8_t i = len-1; ; i--) {
		if (currentDirectory[i] == '/') {
			currentDirectory[i == 0 ? 1 : i] = 0;
			break;
		}
	}
	return true;
}

extern char simulator_cwd[1024];


/**
 * Init SD-card device, opens partition and file system
 */
bool FioInit() {
	initCwd();
	
	char dir[1024];
getcwd(dir, sizeof(dir));
printf("dir0=%s\n", dir);
	
	chdir(simulator_cwd);
getcwd(dir, sizeof(dir));
printf("dir1=%s\n", dir);
	
	MSG("FIO-INIT");
	sdcard_raw_inited = true;
	if (!sdcard_raw_inited) {
		MSG("MMC/SD initialization failed");
		return false;
	}

	chdir(ROOT_DIR);
getcwd(dir, sizeof(dir));
printf("dir2=%s\n", dir);
printf("currentDirectory = %s\n", currentDirectory);
//	if (!isDirExist(currentDirectory)) {
//printf("doesn't exist!\n");		
//		currentDirectory[0] = 0;
//	}
strcpy(dir, simulator_cwd);
strcat(dir, "/");
strcat(dir, ROOT_DIR);
strcat(dir, currentDirectory);
	if (!isDirExist(dir)) {
printf("doesn't exist (%s)!\n", dir);		
		currentDirectory[0] = 0;
		strcpy(dir, simulator_cwd);
		strcat(dir, "/");
		strcat(dir, ROOT_DIR);
		strcat(dir, currentDirectory);

	}
printf("cd = %s\n", dir);
	if (chdir(dir)) {
		MSG_STR("can't change dir, %s", currentDirectory);
		return false;
	}
getcwd(dir, sizeof(dir));
printf("dir3=%s\n", dir);


	dirData.dirSize = getDirSize();
	MSG_DEC("DIR_SIZE ", dirData.dirSize);
	
	dirData.offset = 0;
	dirData.dataSize = 0;
	fileio_dir = NULL;

//	MSG("OK!");
	return true;
}


void FioClose() {
	MSG("FIO-CLOSE");

	if (!sdcard_raw_inited) {
		MSG("sdrd_CLOSE WRONG");
		return;
	}
	if (fileio_dir != NULL) {
		closedir(fileio_dir);
	}
	if (fileio_fd != NULL) {
		fclose(fileio_fd);
	}
	fio_read_eof_handler = NULL;
	sdcard_raw_inited = false;
	fileio_dir = NULL;
}


bool FioOpenFile(const char* fileName) {
	if (!sdcard_raw_inited) {
		FioInit();
	}
	fileio_fd = fopen(fileName, "rb");
	strcpy(fileio_fname, fileName);
	if (!fileio_fd) {
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
	//trol_fast_read_mode = false;
	
	fio_read_eof_handler = NULL;
	
	readNext();
	
	return true;
}

static uint8_t fioNextAsync() {
	sdcard_buf_1[0] = fgetc(fileio_fd);
	return sdcard_buf_1[0];
}


bool FioOpenFileAsync(const char* fileName){
	if (!sdcard_raw_inited) {
		FioInit();
	}
	fileio_fd = fopen(fileName, "rb");
	strcpy(fileio_fname, fileName);
	if (!fileio_fd) {
		MSG("error opening\n");
		return false;
	}
	MSG_DEC("file is open, size= ", FioGetFileSize());
	
	fio_get_next_byte_handler = &fioNextAsync;

	task_readNextBlock = false;
	task_writeNextBlock = false;
	//trol_fast_read_mode = false;
	
	fio_read_eof_handler = NULL;
	
	return true;	
}


bool FioCreateFile(const char* fileName) {
	printf("file='%s'\n", fileName);
	fileio_fd = fopen(fileName, "wb+");
	strcpy(fileio_fname, fileName);
	if (!fileio_fd) {
		printf("error %s\n", strerror(errno));
		MSG("error create");
		return false;
	}
	
	fio_readyFlags = 0;
	fio_put_next_byte_handler = &fioNextToBuf1Wait;
	fio_buf_part_pos = 0;
	
	task_readNextBlock = false;
	task_writeNextBlock = true;
	return true;
}


bool FioAllocSize(uint32_t bytes) {
	return true;
}

/************************************************************************/
/* Возвращает размер открытого файла                                    */
/************************************************************************/
uint32_t FioGetFileSize() {
	size_t pos = ftell(fileio_fd);
	fseek(fileio_fd, 0, SEEK_END);
	uint32_t size = ftell(fileio_fd);
	fseek(fileio_fd, pos, SEEK_SET);

	return size;
}


/************************************************************************/
/* Закрывает файл                                                       */
/************************************************************************/
void FioCloseFile() {
	MSG("close file");
	// если файл открыт на запись, то сначала сохраняем изменения
	FioFlush();
	if (fileio_fd) {
		fclose(fileio_fd);
		fileio_fd = 0;
	}
	task_readNextBlock = false;
	task_writeNextBlock = false;
}


bool FioFileExists(const char* fileName) {
	return access(fileName, F_OK) != -1;
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
		cnt = fread(sdcard_buf_1, 1, sizeof(sdcard_buf_1), fileio_fd);
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
		cnt = fread(sdcard_buf_2, 1, sizeof(sdcard_buf_2), fileio_fd);
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
		if (fwrite(sdcard_buf_1, 1, sizeof(sdcard_buf_1), fileio_fd) != sizeof(sdcard_buf_1)) {
			MSG("WRITE_ERROR");
		}
		fio_readyFlags &= ~_BV(0);
	}
	// если второй блок заполнен и готов к записи на карту
	if (fio_readyFlags & _BV(1)) {
		if (fwrite(sdcard_buf_2, 1, sizeof(sdcard_buf_2), fileio_fd) != sizeof(sdcard_buf_2)) {
			MSG("WRITE_ERROR");
		}
		fio_readyFlags &= ~_BV(1);
	}
}


// Записать все накопившиеся байты, сбросить буферы на диск
void FioFlush() {
//	if (!sd_raw_sync()) {
//		MSG("sync err f!");
//	}
//MSG_DEC("BWP ", FioGetFilePos());	
	// если есть полностью заполненные буферы, сохраняем сначала их
	writeNext();
	// если мы не в режиме записи файла
	if (!task_writeNextBlock) {
		return;
	}
	// осталось сохранить текущий блок	
	if (fio_put_next_byte_handler == &fioNextToBuf1Start) {
		if (fwrite(sdcard_buf_1, 1, fio_buf_part_pos, fileio_fd) != fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-1s");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf2Start) {
		if (fwrite(sdcard_buf_2, 1, fio_buf_part_pos, fileio_fd) != fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-2s");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf1End) {
		if (fwrite(sdcard_buf_1, 1, 0x100+fio_buf_part_pos, fileio_fd) != 0x100+fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-1e");
		}
	} else if (fio_put_next_byte_handler == &fioNextToBuf2End) {
		if (fwrite(sdcard_buf_2, 1, 0x100+fio_buf_part_pos, fileio_fd) != 0x100+fio_buf_part_pos) {
			MSG("FLUSH_WRITE_ERROR-2e");
		}
	}
//MSG_DEC("AWP ", FioGetFilePos());
	fio_readyFlags = 0;
	fio_buf_part_pos = 0;
	fio_get_next_byte_handler = &fioNextFromBuf1Wait;
	fio_put_next_byte_handler = &fioNextToBuf1Wait;
}


// Задает размер файла
bool FioResizeFileTo(uint32_t size) {
	// TODO
	return true;
}



// Задает размер файла по текущей позиции
uint32_t FioResizeFile() {
	FioFlush();
	// TODO
	return FioGetFileSize();
}


// Получает текущую позицию в файле
void FioGetFilePos(uint32_t *offset) {
	*offset = ftell(fileio_fd);
}

// Задает позицию от начала файла
bool FioSeekFromStart(uint32_t offset) {
	bool result = fseek(fileio_fd, offset, SEEK_SET) == 0;
	if (fio_get_next_byte_handler != &fioNextAsync) {
		fio_readyFlags = 0;
		fio_get_next_byte_handler = &fioNextFromBuf1Wait;
		fio_buf_part_pos = 0;
		fio_eof = false;
	}
	return result;
}


// Задает позицию от конца файла
bool FioSeekFromEnd(uint32_t offset) {
	bool result = fseek(fileio_fd, offset, SEEK_END) == 0;
	if (fio_get_next_byte_handler != &fioNextAsync) {
		fio_readyFlags = 0;
		fio_get_next_byte_handler = &fioNextFromBuf1Wait;
		fio_buf_part_pos = 0;
		fio_eof = false;
	}
	return result;
}



void FioOnIdle() {
	readNext();
	writeNext();
	usleep(250);
}

DIR *opendir_local(const char *path) {
	char dir[1024];
	strcpy(dir, simulator_cwd);
	strcat(dir, "/");
	strcat(dir, ROOT_DIR);
	strcat(dir, currentDirectory);
	printf("open dir %s  (from %s)\n", dir, currentDirectory);	

	return opendir(dir);
}

/************************************************************************/
/* Проходится по директории и получает количество элементов в ней       */
/************************************************************************/
uint32_t getDirSize() {
	uint32_t result = 0;

	DIR *dir = opendir_local(currentDirectory);
	if (dir != NULL) {
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL) {
			if (isVisibleFile(ent->d_name)) {
				result++;
			}
		}
		closedir(dir);
	} 
	
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

	uint16_t pos = 0;		// позиция в буфере
	
	struct dirent *ent;
	// если надо перейти назад
	if (offset < dirData.offset || offset == 0) {
MSG_DEC("offset = ", offset);		
		
		if (fileio_dir != NULL) {
			closedir(fileio_dir);
		}
		fileio_dir = opendir_local(currentDirectory);
		
		for (uint32_t i = 0; i < offset; i++) {
			ent = readdir(fileio_dir);
			if (!ent) {
MSG_DEC("stop-1 ", i);
				dirData.dataSize = 0;
				return;
			}
//debugMsg("SKIP1", i, file_entry.long_name);
			// игнорируем скрытые файлы
			if (!isVisibleFile(ent->d_name)) {
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
printf("@ %i %i\n", offset, dirData.offset);
		for (uint32_t i = dirData.offset + dirData.dataSize; i < offset; i++) {	// !!!!!!  i <= offset дает ошибку при возврате назад из каталога
			ent = readdir(fileio_dir);
printf("> %s\n", ent->d_name);
			if (!ent) {
				dirData.dataSize = 0;
MSG_DEC("stop-2 ", i);
				return;
			}
//debugMsg("SKIP2", i, file_entry.long_name);			
			// игнорируем скрытые файлы
			if (!isVisibleFile(ent->d_name)) {
				i--;
			}
		}
		dirData.dataSize = 0;
	}
MSG("...");
	dirData.offset = offset;
	
	while (true) {
		ent = readdir(fileio_dir);
		if (!ent) {
			break;
		}
//debugMsg(">", dirData.offset + dirData.dataSize, file_entry.long_name);		
		if (!isVisibleFile(ent->d_name)) {
			continue;
		}
		// если в буфере не хватает места для следующего файла
		uint8_t fnLen = strlen(ent->d_name);
		if (ent->d_type & DT_DIR) {
			fnLen++;
		}
		//if (pos + fnLen >= sizeof(sdcard_ls_data) - 1) {
			//return;
		//}
		if (ent->d_type & DT_DIR) {
			fio_data[pos++] = '>';
			fnLen--;
		}

		strcpy(fio_data + pos, ent->d_name);
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
	if (fileio_dir) {
		closedir(fileio_dir);
	}
	fileio_dir = opendir_local(currentDirectory);
	
	uint32_t index = 0;
	struct dirent *ent;
	while (true) {
		ent = readdir(fileio_dir);
		if (!ent) {
			closedir(fileio_dir);
			fileio_dir = opendir_local(currentDirectory);
			dirData.offset = 0;
			dirData.dataSize = 0;
			return 0xffffffff;
		}
		if (!isVisibleFile(ent->d_name)) {
			continue;
		}
		if (strcmp(path, ent->d_name) == 0) {
			closedir(fileio_dir);
			fileio_dir = opendir_local(currentDirectory);
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
	return fileio_fname;
}


bool FioReadDiskInfo() {
	bool result;
	if (sdcard_raw_inited) {
		FioClose();
	}
	if (FioInit()) {
		struct sd_raw_info* info = (struct sd_raw_info*)sdcard_buf_1;
		
		info->manufacturer = 123;
		strcpy((char*)info->oem, "KS");
		strcpy((char*)info->product, "Name");

		info->revision = 0x32;
		info->serial = 0x1234;
		info->manufacturing_year = 15;
		info->manufacturing_month = 5;
		info->capacity = 4*1024*1024;
		info->flag_copy = 0;
		info->flag_write_protect = 0;
		info->flag_write_protect_temp = 0;
		info->format = SD_RAW_FORMAT_HARDDISK;
		
		
		result = true;//sd_raw_get_info(info);
//		if (!sdcard_partition->has_mbr) {
//			info->format = SD_RAW_FORMAT_SUPERFLOPPY;
//		}
		uint32_t size = 1024*4;//(uint32_t)(fat_get_fs_size(sdcard_fs) / 1024 / 1024);	// размер ФС в МБ
		uint32_t free = 1024*2;//(uint32_t)(fat_get_fs_free(sdcard_fs) / 1024 / 1024);	// свободно МБ
		memcpy(sdcard_buf_2+0, &size, 4);
		memcpy(sdcard_buf_2+4, &free, 4);
		uint16_t clusterSize = 512;
		uint16_t sectorSize = 512;
		uint16_t partitionType = 1;
		memcpy(sdcard_buf_2+8, &clusterSize, 2);//&sdcard_fs->header.cluster_size, 2);
		memcpy(sdcard_buf_2+10, &sectorSize, 2);//&sdcard_fs->header.sector_size, 2);
		memcpy(sdcard_buf_2+12, &partitionType, 2) ;//&sdcard_fs->partition->type, 1);
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
	//trol_fast_read_mode = true;
}


bool FioIsFileOpen() {
	return fileio_fd != 0;
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

bool isDirExist(const char *str) {
	struct stat s;
	int err = stat(str, &s);
	if (err != -1) {
		 if(S_ISDIR(s.st_mode)) {
			 return true;
		 }
	}
	return false;
}


uint8_t fioNextFromBuf1Start();
uint8_t fioNextFromBuf1End();
uint8_t fioNextFromBuf2Start();
uint8_t fioNextFromBuf2End();
uint8_t fioNextFromBuf1Wait();
uint8_t fioNextFromBuf2Wait();

uint8_t fioNextFromBuf1Start() {
	uint8_t result = sdcard_buf_1[fio_buf_part_pos];
	fio_buf_part_pos++;
	if (fio_buf_part_pos == 0) {
		fio_get_next_byte_handler = &fioNextFromBuf1End;
	}
//	MSG_HEX("r1 ", result, 1);
	return result;
}


uint8_t fioNextFromBuf1End() {
	uint8_t result = sdcard_buf_1[0x100 + fio_buf_part_pos];
	fio_buf_part_pos++;
	if (fio_buf_part_pos == 0) {
		fio_readyFlags &= ~_BV(0);
		fio_get_next_byte_handler = &fioNextFromBuf2Wait;
	}
//	MSG_HEX("R1 ", result, 1);
	return result;
}


uint8_t fioNextFromBuf2Start() {
	uint8_t result = sdcard_buf_2[fio_buf_part_pos];
	fio_buf_part_pos++;
	if (fio_buf_part_pos == 0) {
		fio_get_next_byte_handler = &fioNextFromBuf2End;
	}
//	MSG_HEX("r2 ", result, 1);
	return result;
}


uint8_t fioNextFromBuf2End() {
	uint8_t result = sdcard_buf_2[0x100 + fio_buf_part_pos];
	fio_buf_part_pos++;
	if (fio_buf_part_pos == 0) {
		fio_readyFlags &= ~_BV(1);
		fio_get_next_byte_handler = &fioNextFromBuf1Wait;
	}
//	MSG_HEX("R2 ", result, 1);
	return result;
}

uint8_t fioNextFromBuf1Wait() {
	if (fio_readyFlags & _BV(0)) {
		fio_get_next_byte_handler = &fioNextFromBuf1Start;
//MSG_HEX("rw1 ", sdcard_buf_1[fio_buf_part_pos], 1);		
		return sdcard_buf_1[fio_buf_part_pos++];
	}
MSG("rw1-----------------------------");	
	fio_error_mask |= FIO_ERROR_READ_NOT_READY;
	fio_error_cnt++;
	//print("ERROR!");
	return 0x80;	
}

uint8_t fioNextFromBuf2Wait() {
	if (fio_readyFlags & _BV(1)) {
		fio_get_next_byte_handler = &fioNextFromBuf2Start;
MSG_HEX("rw2 ", sdcard_buf_2[fio_buf_part_pos], 1);				
		return sdcard_buf_2[fio_buf_part_pos++];
	}
MSG("rw2-----------------------------");	
	fio_error_mask |= FIO_ERROR_READ_NOT_READY;
	fio_error_cnt++;
	//print("ERROR!");
	return 0x80;
}

void fioNextToBuf1Start(uint8_t b) {
	sdcard_buf_1[fio_buf_part_pos++] = b;
	if (fio_buf_part_pos == 0) {
		fio_put_next_byte_handler = &fioNextToBuf1End;
	}
}

void fioNextToBuf2Start(uint8_t b) {
	sdcard_buf_2[fio_buf_part_pos++] = b;
	if (fio_buf_part_pos == 0) {
		fio_put_next_byte_handler = &fioNextToBuf2End;
	}
}

void fioNextToBuf1End(uint8_t b) {
	sdcard_buf_1[0x100 + fio_buf_part_pos] = b;
	if (++fio_buf_part_pos == 0) {		
		fio_readyFlags |= _BV(0);
		fio_put_next_byte_handler = &fioNextToBuf2Wait;
	}
}

void fioNextToBuf2End(uint8_t b) {
	sdcard_buf_2[0x100 + fio_buf_part_pos] = b;
	if (++fio_buf_part_pos == 0) {		
		fio_readyFlags |= _BV(1);
		fio_put_next_byte_handler = &fioNextToBuf1Wait;
	}
}

void fioNextToBuf1Wait(uint8_t b) {
	if (fio_readyFlags & _BV(0)) {
		fio_error_mask |= FIO_ERROR_WRITE_NOT_READY;		  
		fio_error_cnt++;
	} else {
		fio_put_next_byte_handler = fioNextToBuf1Start;
		sdcard_buf_1[0] = b;
		fio_buf_part_pos = 1;
	}
}


/****************************************************************/
//
/****************************************************************/
void fioNextToBuf2Wait(uint8_t b) {
	if (fio_readyFlags & _BV(1)) {
		fio_error_mask |= FIO_ERROR_WRITE_NOT_READY;		  
		fio_error_cnt++;
	} else {
		fio_put_next_byte_handler = fioNextToBuf2Start;
		sdcard_buf_2[0] = b;
		fio_buf_part_pos = 1;
	}	
}

uint16_t FioReadWord() {
	readNext();
	uint8_t lo = FioNextByte();
	uint8_t hi = FioNextByte();
	return (hi << 8) + lo;
}

uint32_t FioReadDword() {
	uint16_t lo = FioReadWord();
	uint16_t hi = FioReadWord();
	return (hi << 16) + lo;
}

void FioPutWord(uint16_t w) {
	FioPutByte(w & 0xff);
	FioPutByte((w >> 8) & 0xff);
}

void FioPutDword(uint32_t dw) {
	FioPutByte(dw & 0xff);
	FioPutByte((dw >> 8) & 0xff);
	FioPutByte((dw >> 16) & 0xff);
	FioPutByte((dw >> 24) & 0xff);
}


