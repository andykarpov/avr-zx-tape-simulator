#include "avrdef.h"

#define SLEEP_MODE_IDLE         (0)
#define SLEEP_MODE_ADC          _BV(SM0)
#define SLEEP_MODE_PWR_DOWN     _BV(SM1)
#define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
#define SLEEP_MODE_STANDBY      (_BV(SM1) | _BV(SM2))
#define SLEEP_MODE_EXT_STANDBY  (_BV(SM0) | _BV(SM1) | _BV(SM2))

#define set_sleep_mode(mode)
