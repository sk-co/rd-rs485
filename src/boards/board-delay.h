#ifndef __DELAY_BOARD_H__
#define __DELAY_BOARD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void Delay( float s );
void DelayMs( uint32_t ms );
void DelayMks( uint16_t mks );
uint32_t GetSysMs();

#ifdef __cplusplus
}
#endif

#endif // __DELAY_BOARD_H__
