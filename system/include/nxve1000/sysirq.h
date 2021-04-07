#ifndef _SYSIRQ_H_
#define _SYSIRQ_H_

typedef void (*ISR_Handler)(void *argument);

typedef struct {
	ISR_Handler	func;
	void *argument;
} ISR_CB;

#endif
