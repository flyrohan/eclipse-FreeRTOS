#ifndef _SYSTIME_H_
#define _SYSTIME_H_

#include <cmsis_device.h>

typedef struct {
	void	 (*Delay)(int ms);
	uint64_t (*GetTickUS)(void);
} SysTime_Op;

void SysTime_Delay(int ms);
uint64_t SysTime_GetTick(void);
uint64_t SysTime_GetTickUS(void);
void SysTime_Register(SysTime_Op *op);

#endif
