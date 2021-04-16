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
		ALIVE_IRQHandler,				// The Alive handler 	(0)
		ALIVE_TIMER_IRQHandler,			// The AliveTimer handler	(1)
		WDT_IRQHandler,					// The WDT handler		(2)
		GPIO_IRQHandler,				// The GPIO handler		(3)
		I2C0_IRQHandler,				// The I2C0 handler		(4)
		I2C1_IRQHandler,				// The I2C1 handler		(5)
		SPI0_IRQHandler,				// The SPI0	handler		(6)
		SPI1_IRQHandler,				// The SPI1	handler		(7)
		SPI2_IRQHandler,				// The SPI2	handler		(8)
		UART0_IRQHandler,				// The UART0 handler	(9)
		UART1_IRQHandler,				// The UART1 handler	(10)
		DeviceInterrupt_Handler,		// (11)
		DeviceInterrupt_Handler,		// (12)
		DMA_IRQHandler,					// The DMA handler		(13)
		Timer0_IRQHandler,             	// Timer0 handler (14)
		Timer1_IRQHandler,             	// Timer1 handler (15)
		Timer2_IRQHandler,             	// Timer2 handler (16)
		Timer3_IRQHandler,             	// Timer3 handler (17)
		Timer4_IRQHandler,             	// Timer4 handler (18)
		Timer5_IRQHandler,             	// Timer5 handler (19)
		Timer6_IRQHandler,             	// Timer6 handler (20)
		Timer7_IRQHandler,             	// Timer7 handler (21)
		USB_IRQHandler,					// The GPIO handler		(22)
		PLL_IRQHandler,					// The PLL handler		(23)
		ADC_IRQHandler,					// The ADC handler		(24)
		DeviceInterrupt_Handler,		// (25)
		DeviceInterrupt_Handler,		// (26)
		DeviceInterrupt_Handler,		// (27)
		LVD_IRQHandler,					// The LVD handler		(28)
		DeviceInterrupt_Handler,		// (29, 30, 31)
    // TODO: rename and add more vectors here
    };

// ----------------------------------------------------------------------------
#include <stddef.h>

static ISR_Hnadler_t *isr_handler_t[28] = { };

void ISR_Register(int irq, ISR_Hnadler_t *handler)
{
	isr_handler_t[irq] = handler;
}

void ISR_UnRegister(int irq)
{
	isr_handler_t[irq] = NULL;
}

#define _doISR(_irq)	do {	\
		if (isr_handler_t[_irq]) \
			isr_handler_t[_irq]->func(_irq, isr_handler_t[_irq]->argument);	\
	} while (0)

// ----------------------------------------------------------------------------

// Processor ends up here if an unexpected interrupt occurs or a specific
// handler is not present in the application code.

void __attribute__ ((section(".after_vectors")))
Default_Handler(void)
{
	 _doISR((int)__get_IPSR() - 16);
}

// ----------------------------------------------------------------------------
void __attribute__ ((weak, alias ("Default_Handler")))
ALIVE_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
ALIVE_TIMER_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
WDT_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
GPIO_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
I2C0_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
I2C1_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
SPI0_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
SPI1_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
SPI2_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
UART0_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
UART1_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
DMA_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer0_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer1_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer2_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer3_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer4_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer5_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer6_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
Timer7_IRQHandler(void);

void __attribute__ ((weak, alias ("Default_Handler")))
USB_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
PLL_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
ADC_IRQHandler (void);

void __attribute__ ((weak, alias ("Default_Handler")))
LVD_IRQHandler (void);
