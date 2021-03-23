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
        DeviceInterrupt_Handler,                  // Device specific
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
