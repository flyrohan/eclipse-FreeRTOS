#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <cmsis_device.h>

//////////////////////////////////////////////////////////////////////////////////////
// RTOS(FreeRTOS) Enable
//////////////////////////////////////////////////////////////////////////////////////
#define RTOS_ENABLED
//#define CMSIS_ENABLED

//////////////////////////////////////////////////////////////////////////////////////
// Apps Enable
//////////////////////////////////////////////////////////////////////////////////////
#define LOOP_ENABLED
//#define DMIPS_ENABLED

//////////////////////////////////////////////////////////////////////////////////////
// Library and System Interface Enable
//////////////////////////////////////////////////////////////////////////////////////
#define SYSTEM_TIME_SYSTICK 	0
#define SYSTEM_TIME_TIMER 		1
#define SYSTEM_TIME_MODULE		SYSTEM_TIME_TIMER /* set SYSTEM_TIME_SYSTICK or SYSTEM_TIME_TIMER */

#define CLI_ENABLED
#define CMD_ENABLED
#define CONSOLE_ENABLED

//////////////////////////////////////////////////////////////////////////////////////
// Modules Enable
//
// - SYSTICK_ENABLED
// - TIMER_ENABLED
// - REMAP_ENABLED
// - PLL_ENABLED
// - UART_ENABLED
// - GPIO_ENABLED
//////////////////////////////////////////////////////////////////////////////////////
#define TIMER_ENABLED
#define REMAP_ENABLED
#define PLL_ENABLED
#define UART_ENABLED
#define GPIO_ENABLED

//////////////////////////////////////////////////////////////////////////////////////
// System Configure
//////////////////////////////////////////////////////////////////////////////////////
#define SYSTEM_CLOCK			48000000
#define SYSTEM_TICK_HZ			1000
#define TIMER_CH				(0)
#define TIMER_CLOCK_HZ			1000000			/* 1Mhz */
#define TIMER_CLOCK_PERIODIC_HZ	16000000		/* 16Mhz */
#define UART_CHANNEL		0
#define CLI_BUFFER_SIZE 	32
#define CMD_BUFFER_SIZE		32

#endif
