#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include <cmsis_device.h>

int  SysTick_Init(unsigned int clock, int hz);
void SysTick_Register(unsigned int clock, int hz);

#endif
