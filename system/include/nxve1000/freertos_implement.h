#ifndef _FREERTOS_IMPLEMENT_H_
#define _FREERTOS_IMPLEMENT_H_

#include <config.h>

#ifdef RTOS_ENABLED

#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
				StackType_t **ppxIdleTaskStackBuffer,
				uint32_t *pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
				StackType_t **ppxTimerTaskStackBuffer,
				uint32_t *pulTimerTaskStackSize );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
#endif

#endif
