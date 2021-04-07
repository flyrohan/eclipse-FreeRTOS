#ifndef _TIMER_H_
#define _TIMER_H_

#include <sysirq.h>

int  TIMER_Init(int ch, unsigned int clock, int hz);
void TIMER_Register(int ch, unsigned int clock, int hz);
void TIMER_ISR_Register(int ch, ISR_Handler handler, void *argument);

#endif
