#ifndef _SYSTIME_H_
#define _SYSTIME_H_

#include <cmsis_device.h>

typedef struct {
	void	 (*Delay)(int ms);
	uint64_t (*GetTimeUS)(void);
} SysTime_Op;

void SysTime_Delay(int ms);
uint64_t SysTime_GetTime(void);
uint64_t SysTime_GetTimeUS(void);
void SysTime_Register(SysTime_Op *op);

#endif
