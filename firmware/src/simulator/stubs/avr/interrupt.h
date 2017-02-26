#include "avrdef.h"

#ifdef __cplusplus
#  define ISR(vector, ...)            \
    extern "C" void vector (void) __VA_ARGS__; \
    void vector (void)
#else
#  define ISR(vector, ...)            \
    void vector (void)  __VA_ARGS__; \
    void vector (void)
#endif

#define sei()
#define cli()
