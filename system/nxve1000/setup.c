/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <cmsis_device.h>
#include <system.h>
#include <init.h>
#include <config.h>

#define GPIO_UART_TX	24
#define GPIO_UART_RX	25
#define EFLASH_FIX_SPEEAD

static void SYS_eFlashInit(void)
{
#ifdef EFLASH_FIX_SPEEAD
	/* eflash timing: TRCK0, TRCK1, TWCMH, TWCK0, ERASE_READY_DOWN_ENB */
	*(uint32_t *)0x42010018 = 0x0; // 0x9
	*(uint32_t *)0x4201001c = 0x0; // 0x9
	*(uint32_t *)0x42010028 = 0x0; // 0xD
	*(uint32_t *)0x4201002c = 0x0; // 0x9
	*(uint32_t *)0x4201004C = 0x1;
#endif
}

static void SYS_GpioInit(void)
{
	GPIO_SetAlt(GPIO_UART_TX, GPIO_ALTFUNC_1);
	GPIO_SetAlt(GPIO_UART_RX, GPIO_ALTFUNC_1);
}

SYSTEM_INIT(HAL_SystemInit);
void __attribute__((weak)) HAL_SystemInit(void)
{
	System_Remap();

	PLL_SetFrequency(SYSTEM_CLOCK);

	SYS_eFlashInit();
	SYS_GpioInit();

#if defined(CONSOLE_ENABLED) && defined(UART_ENABLED)
	UART_ConsoleRegister(UART_CHANNEL, SYSTEM_CLOCK);
#endif

#if defined(SYSTEM_TIME_ENABLED) && !defined(RTOS_ENABLED)
	#if defined(SYSTICK_ENABLED)
	SysTick_Register(SYSTEM_CLOCK, SYSTEM_TICK_HZ);
	#elif defined(TIMER_ENABLED)
	TIMER_Register(0, SYSTEM_CLOCK, SYSTEM_TICK_HZ);
	#endif
#endif
}

