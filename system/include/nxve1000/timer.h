#ifndef _TIMER_H_
#define _TIMER_H_

#include <stddef.h>
#include <stdbool.h>
#include <sysirq.h>

typedef enum {
	TIMER_MODE_SYSTIMER,	/* max-count timer and register to system time, No IRQ */
	TIMER_MODE_MAXCOUNT,	/* max-count timer, No IRQ, */
	TIMER_MODE_PERIODIC,	/* periodic timer with IRQ */
	TIMER_MODE_FREERUN,		/* free-running PWM-timer, No IRQ, */
} TIMER_MODE_t;

int  TIMER_Init(int ch, unsigned int infreq, unsigned int tfreq, TIMER_MODE_t mode);
void TIMER_Frequency(int ch, int hz, int duty, bool invert);
void TIMER_Start(int ch);
void TIMER_Stop(int ch);

uint64_t TIMER_GetTickUS(int ch);
void TIMER_Delay(int ch, int ms);

void TIMER_CallbackISR(int ch, ISR_Callback cb, void *argument);

/* register to system timer */
void TIMER_Register(int ch, unsigned int infreq, unsigned int tfreq, int hz, TIMER_MODE_t mode);

#endif

