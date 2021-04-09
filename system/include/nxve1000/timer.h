#ifndef _TIMER_H_
#define _TIMER_H_

#include <sysirq.h>

int  TIMER_Init(int ch, unsigned int infreq, unsigned int tfreq, int hz);
/* register to system time */
void TIMER_Register(int ch, unsigned int infreq, unsigned int tfreq, int hz);
/* register to timer interrupt handler */
void TIMER_ISR_Register(int ch, ISR_Handler handler, void *argument);

#endif
