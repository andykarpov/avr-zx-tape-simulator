#include "avr/eeprom.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define FILENAME "eeprom.bin"
#define SIZE	512

extern char simulator_cwd[1024];

static FILE *open(uint16_t pos) {
	initCwd();
	char fn[1024];
	strcpy(fn, simulator_cwd);
	strcat(fn, "/");
	strcat(fn, FILENAME);
	FILE* f = fopen(fn, "r+");
	if (!f) {
		f = fopen(fn, "w+");
		fclose(f);
		f = fopen(fn, "r+");
	}
	if (!f) {
		printf("can't open file %s (%s)\n", fn, strerror(errno));
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)))
			fprintf(stdout, "Current working dir: %s\n", cwd);
		exit(1);
	}
	
	fseek(f, 0, SEEK_END);
	uint32_t size = ftell(f);
	
	if (size != SIZE) {
		printf("RESIZE FILE (%i)\n", size);
		uint8_t b0 = 0;
		for (uint16_t i = 0; i < SIZE; i++) {
			fwrite(&b0, 1, 1, f);
		}
	}	
	fseek(f, pos, SEEK_SET);
	return f;
}

uint8_t eeprom_read_byte(const uint8_t *offset) {
	FILE *f = open((int)offset);
	uint8_t result = 0;
	fread(&result, 1, 1, f);
	fclose(f);
//	printf("read byte %i: %i\n", (int)offset, result);
	return result;
}

uint16_t eeprom_read_word(const uint16_t *offset) {
//	printf("read word %i\n", (int)offset);
	FILE *f = open((int)offset);
	uint16_t result = 0;
	fread(&result, 2, 1, f);
	fclose(f);
	return result;
}

uint32_t eeprom_read_dword(const uint32_t *offset) {
//	printf("read dword %i\n", (int)offset);
	FILE *f = open((int)offset);
	uint32_t result = 0;
	fread(&result, 4, 1, f);
	fclose(f);
	return result;	
}

void eeprom_read_block(const void *ptr, const uint8_t *offset, uint16_t size) {
//	printf("read block %i (%i)\n", (int)offset, size);
	FILE *f = open((int)offset);
	fread((void*)ptr, 4, 1, f);
	fclose(f);
}

void eeprom_write_byte(const uint8_t *offset, uint8_t val) {
//	printf("write byte %i: %i\n", (int)offset, val);
	FILE *f = open((int)offset);
	fwrite(&val, 1, 1, f);
	fclose(f);
}

void eeprom_write_word(const uint16_t *offset, uint16_t val) {
//	printf("write word %i: %i\n", (int)offset, val);
	FILE *f = open((int)offset);
	fwrite(&val, 2, 1, f);
	fclose(f);	
}

void eeprom_write_dword(const uint32_t *offset, uint32_t val) {
//	printf("write dword %i: %i\n", (int)offset, val);
	FILE *f = open((int)offset);
	fwrite(&val, 4, 1, f);
	fclose(f);	
}

void eeprom_write_block(const void *ptr, const uint8_t *offset, uint16_t size) {
//	printf("write block %i (%i)\n", (int)offset, size);
	FILE *f = open((int)offset);
	fwrite(ptr, size, 1, f);
	fclose(f);
}

void eeprom_update_byte(const uint8_t *offset, uint8_t val) {
	uint8_t v = eeprom_read_byte(offset);
	if (v != val) {
		eeprom_write_byte(offset, val);
	}
}

void eeprom_update_word(const uint16_t *offset, uint16_t val) {
	uint16_t v = eeprom_read_word(offset);
	if (v != val) {
		eeprom_write_word(offset, val);
	}	
}

void eeprom_update_dword(const uint32_t *offset, uint32_t val) {
	uint32_t v = eeprom_read_dword(offset);
	if (v != val) {
		eeprom_write_dword(offset, val);
	}
}

void eeprom_update_block(const void *ptr, const uint8_t *offset, uint16_t size) {
	// TODO 
	eeprom_write_block(ptr, offset, size);
}

