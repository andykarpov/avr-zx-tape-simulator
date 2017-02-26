#ifndef _AVRDEF_H_
#define _AVRDEF_H_

#include <stdlib.h>


#define _BV(bit) (1 << (bit))


#ifndef strcat_P
	#define strcat_P strcat
#endif

#ifndef strcpy_P
	#define strcpy_P strcpy
#endif

#ifndef strlen_P
	#define strlen_P strlen
#endif

#ifndef sprintf_P
	#define sprintf_P sprintf
#endif

#ifndef memcpy_P
	#define memcpy_P memcpy
#endif


char* itoa(int value, char *str, int radix);
char *ltoa(long N, char *str, int base);

#endif // _AVRDEF_H_