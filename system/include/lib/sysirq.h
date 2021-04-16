#ifndef _SYSIRQ_H_
#define _SYSIRQ_H_

#include <nxve1000.h>

typedef void (*ISR_Handler)(int irq, void *argument);
typedef void (*ISR_Callback)(void *argument);

typedef struct {
	ISR_Handler	func;
	void *argument;
} ISR_Hnadler_t;

typedef struct {
	ISR_Callback	func;
	void *argument;
} ISR_Callback_t;

void ISR_Register(int irq, ISR_Hnadler_t *handler);
void ISR_UnRegister(int irq);

#endif
