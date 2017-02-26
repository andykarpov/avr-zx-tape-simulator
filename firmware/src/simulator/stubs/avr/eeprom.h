#ifdef SIMULATION
#include "avr/io.h"
#include "avrdef.h"


#ifdef __cplusplus
extern "C" {
#endif
	
uint8_t eeprom_read_byte(const uint8_t *offset);
uint16_t eeprom_read_word(const uint16_t *offset);
uint32_t eeprom_read_dword(const uint32_t *offset);
void eeprom_read_block(const void *ptr, const uint8_t *offset, uint16_t size);

void eeprom_write_byte(const uint8_t *offset, uint8_t val);
void eeprom_write_word(const uint16_t *offset, uint16_t val);
void eeprom_write_dword(const uint32_t *offset, uint32_t val);
void eeprom_write_block(const void *ptr, const uint8_t *offset, uint16_t size);

void eeprom_update_byte(const uint8_t *offset, uint8_t val);
void eeprom_update_word(const uint16_t *offset, uint16_t val);
void eeprom_update_dword(const uint32_t *offset, uint32_t val);
void eeprom_update_block(const void *ptr, const uint8_t *offset, uint16_t size);



void eeprom_simulator_init();


#ifdef __cplusplus
}
#endif

#endif