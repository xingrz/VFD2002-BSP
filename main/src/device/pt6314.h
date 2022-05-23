#ifndef __DEVICE_PT6314__
#define __DEVICE_PT6314__

#include <stdint.h>

void pt6314_init(void);
void pt6314_clear(void);
void pt6314_print(uint8_t offset, char *str);

#endif  // __DEVICE_PT6314__
