/*-----------------------------------------------------------------------------
 *      Name:         RV_Config.h 
 *      Purpose:      RV Config header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#ifndef __RV_CONFIG_H
#define __RV_CONFIG_H

#include "RTE_Components.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> Common Test Settings
// <o> Print Output Format <0=> Plain Text <1=> XML
// <i> Set the test results output format to plain text or XML
#ifndef PRINT_XML_REPORT
#define PRINT_XML_REPORT            0 
#endif
// <o> Buffer size for assertions results
// <i> Set the buffer size for assertions results buffer
#define BUFFER_ASSERTIONS           128  
// </h>
  
#define TC_TIMER_CH		(1)
#define TC_TEST_IRQ		(TIMER0_IRQn + TC_TIMER_CH)
#define TC_TEST_SETUP	do { \
		TIMER_Init(TC_TIMER_CH, SYSTEM_CLOCK, TIMER_MODE_FREERUN); \
		TIMER_CallbackISR(TC_TIMER_CH, (ISR_Callback)TST_IRQHandler, NULL); \
	} while (0)


#endif /* __RV_CONFIG_H */
