#ifndef _TIMER_H_
#define _TIMER_H_

#include <stddef.h>
#include <sysirq.h>

typedef enum {
	TIMER_MODE_ONESHOT,		/* Enable IRQ */
	TIMER_MODE_PERIODIC,	/* Enable IRQ */
	TIMER_MODE_FREERUN,		/* Not Enable ISR */
} TIMER_MODE_t;

int  TIMER_Init(int ch, unsigned int infreq, unsigned int tfreq, int hz, TIMER_MODE_t mode);
void TIMER_Frequency(int ch, int mux, int scale, unsigned int count, unsigned int cmp);
void TIMER_Start(int ch);
void TIMER_Stop(int ch);

uint64_t TIMER_GetTickUS(int ch);
void TIMER_Delay(int ch, int ms);

void TIMER_CallbackISR(int ch, ISR_Callback cb, void *argument);
void TIMER_Register(int ch, unsigned int infreq, unsigned int tfreq,
					int hz, TIMER_MODE_t mode, SysTime_Op *op);

#endif
