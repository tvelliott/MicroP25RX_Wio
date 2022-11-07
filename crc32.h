
#ifndef __CRC32_H__
#define __CRC32_H__
#include <stdint.h>
uint32_t crc32_range( unsigned char *ucBuffer, int ulCount );
extern uint32_t crc32_val;
#endif
