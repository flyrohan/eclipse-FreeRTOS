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
#include <FreeRTOSConfig.h>
#include "cortexm/ExceptionHandlers.h"

// ----------------------------------------------------------------------------

void __attribute__((weak))
Default_Handler(void);

// Forward declaration of the specific IRQ handlers. These are aliased
// to the Default_Handler, which is a 'forever' loop. When the application
// defines a handler (with the same name), this will automatically take
// precedence over these weak definitions
//
// TODO: Rename this and add the actual routines here.

void __attribute__ ((weak, alias ("Default_Handler")))
DeviceInterrupt_Handler(void);

// ----------------------------------------------------------------------------
#if !defined(__ARMCC_VERSION)
extern unsigned int _estack;
#endif

typedef void
(* const pHandler)(void);

#if defined(__ARMCC_VERSION)
extern int __main(void);									/* Entry point for C run-time initialization */
extern unsigned int Image$$ARM_LIB_STACKHEAP$$ZI$$Limit; 	/* for (default) One Region model */
#endif

// ----------------------------------------------------------------------------

// The vector table.
// This relies on the linker script to place at correct location in memory.
#if !defined(__ARMCC_VERSION)
__attribute__ ((section(".isr_vector"),used))
#else
__attribute__ ((section("isr_vector"),used))
#endif
pHandler __isr_vectors[] =
  { //
#if !defined(__ARMCC_VERSION)
		(pHandler) &_estack,                      // The initial stack pointer
        Reset_Handler,                            // The reset handler
#else
	    (pHandler)&Image$$ARM_LIB_STACKHEAP$$ZI$$Limit,
	    (pHandler)__main, /* Initial PC, set to entry point  */
#endif
        NMI_Handler,                              // The NMI handler
        HardFault_Handler,                        // The hard fault handler

        0, 0, 0,				  // Reserved
        0,                                        // Reserved
        0,                                        // Reserved
        0,                                        // Reserved
        0,                                        // Reserved
        SVC_Handler,                              // SVCall handler
        0,					  // Reserved
        0,                                        // Reserved
        PendSV_Handler,                           // The PendSV handler
        SysTick_Handler,                          // The SysTick handler

        // ----------------------------------------------------------------------
        // NXVE1000 vectors
		DeviceInterrupt_Handler,	// (0)
		DeviceInterrupt_Handler,	// (1)
		DeviceInterrupt_Handler,	// (2)
		DeviceInterrupt_Handler,	// (3)
		DeviceInterrupt_Handler,	// (4)
		DeviceInterrupt_Handler,	// (5)
		DeviceInterrupt_Handler,	// (6)
		DeviceInterrupt_Handler,	// (7)
		DeviceInterrupt_Handler,	// (8)
		DeviceInterrupt_Handler,	// (9)
		DeviceInterrupt_Handler,	// (10)
		DeviceInterrupt_Handler,	// (11)
		DeviceInterrupt_Handler,	// (12)
		DeviceInterrupt_Handler,	// (13)
		Timer0_Handler,             // Timer0 handler (14)
		Timer1_Handler,             // Timer1 handler (15)
		Timer2_Handler,             // Timer2 handler (16)
		Timer3_Handler,             // Timer3 handler (17)
		Timer4_Handler,             // Timer4 handler (18)
		Timer5_Handler,             // Timer5 handler (19)
		Timer6_Handler,             // Timer6 handler (20)
		Timer7_Handler,             // Timer7 handler (21)
        DeviceInterrupt_Handler,	// Device specific
    // TODO: rename and add more vectors here
    };

// ----------------------------------------------------------------------------

// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.

void __attribute__ ((section(".after_vectors")))
Default_Handler(void)
{
  while (1)
    {
    }
}
// ----------------------------------------------------------------------------
#include <stddef.h>

static ISR_Hnadler_t isr_handler_t[28] = { };

void ISR_Register(int irq, ISR_Handler handler, void *argument)
{
	isr_handler_t[irq].func = handler;
	isr_handler_t[irq].argument = argument;
}

void ISR_UnRegister(int irq)
{
	isr_handler_t[irq].func = NULL;
	isr_handler_t[irq].argument = NULL;
}

// ----------------------------------------------------------------------------
#define _doISR(_irq)	do {	\
		if (isr_handler_t[_irq].func) \
			isr_handler_t[_irq].func(_irq, isr_handler_t[_irq].argument);	\
	} while (0)

void __attribute__ ((section(".after_vectors"),weak))
Timer0_Handler(void) { _doISR(TIMER0_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer1_Handler(void) { _doISR(TIMER1_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer2_Handler(void) { _doISR(TIMER2_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer3_Handler(void) { _doISR(TIMER3_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer4_Handler(void) { _doISR(TIMER4_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer5_Handler(void) { _doISR(TIMER5_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer6_Handler(void) { _doISR(TIMER6_IRQn); }

void __attribute__ ((section(".after_vectors"),weak))
Timer7_Handler(void) { _doISR(TIMER7_IRQn); }
