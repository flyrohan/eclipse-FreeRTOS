#ifndef _TIMER_H_
#define _TIMER_H_

#include <stddef.h>
#include <stdbool.h>
#include <sysirq.h>

typedef enum {
	TIMER_MODE_PERIODIC,	/* periodic timer with IRQ */
	TIMER_MODE_FREERUN,		/* max-count timer, No IRQ, */
	TIMER_MODE_PWM,			/* free-running PWM-timer, No IRQ, */
	TIMER_MODE_PWM_INVERT,	/* free-running Inverted PWM-timer, No IRQ, */
} TIMER_MODE_t;

int  TIMER_Init(int ch, unsigned int infreq, TIMER_MODE_t mode);
void TIMER_Frequency(int ch, int hz, int duty);
void TIMER_Start(int ch);
void TIMER_Stop(int ch);

uint64_t TIMER_GetTimeUS(int ch);
void TIMER_Delay(int ch, int ms);

void TIMER_CallbackISR(int ch, ISR_Callback cb, void *argument);

/* register to system timer */
void TIMER_Register(int ch, unsigned int infreq, int hz, TIMER_MODE_t mode, SysTime_Op *op);

#endif

