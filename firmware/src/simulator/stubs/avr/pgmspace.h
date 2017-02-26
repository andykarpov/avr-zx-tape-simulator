#include "avrdef.h"

#define PROGMEM
#define PGM_P const char *
#define PSTR(s) (s)


#define pgm_read_byte(address_short)    *(address_short)
#define pgm_read_word(address_short)    *(address_short)
#define pgm_read_byte_near(address_short)	*(address_short)