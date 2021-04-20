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

#include <config.h>
#include <rtos.h>

#define _MHZ(_hz)	(_hz/1000000)

#ifdef RTOS_ENABLED
#define CLI_STACK_DEPTH		(512)			/* 256 * sizeof( StackType_t ) */
#define CLI_PRIORITY		osPriorityLow	/* osPriorityLow, osPriorityHigh */

static void RunCLI(void)
{
	Printf("\r\n\r\n***** FREERTOS %ld MHZ *****\r\n\r\n", _MHZ(SYSTEM_CLOCK));
#ifdef CLI_ENABLED
	CLI_RunLoopThread(CLI_STACK_DEPTH, CLI_PRIORITY);
#endif
	HAL_RunRTOS();
}
#else
static void RunCLI(void)
{
	Printf("\r\n\r\n***** FIRMWARE %ld MHZ *****\r\n\r\n", _MHZ(SYSTEM_CLOCK));

	#ifdef CLI_ENABLED
	CLI_RunLoop();
	#else
	#ifdef DMIPS_ENABLED
	extern void DMIPS(int number_of_runs, int cpu_mhz);
	do {
		DMIPS(400000, _MHZ(SYSTEM_CLOCK));
	} while (1);
	#endif
	#endif
}
#endif

int main (void)
{
	RunCLI();

	while (1) {
		Printf("\r\n\r\n!!!!! EXIT FIRMWARE !!!!!\r\n\r\n")
		;
	}

	return 0;
}
