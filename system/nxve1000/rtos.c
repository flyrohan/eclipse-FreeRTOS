#include <cmsis_device.h>
#include <ExceptionHandlers.h>
#include <time.h>
#include <rtos.h>
#include <config.h>

#ifdef RTOS_ENABLED

static void	rtos_TimerInit(void)
{
#ifdef SYSTEM_TIME_ENABLED
	SysTick->CTRL  &= ~(SysTick_CTRL_ENABLE_Msk); /* Disable SysTick IRQ and SysTick Timer */

#if defined(SYSTICK_ENABLED)
	SysTick_Register(SYSTEM_CLOCK, SYSTEM_TICK_HZ);
#elif defined(TIMER_ENABLED)
	TIMER_Register(0, SYSTEM_CLOCK, SYSTEM_TICK_HZ);
#endif
#endif
}

void __attribute__((weak)) HAL_RunRTOS(void)
{
	rtos_TimerInit();
	vTaskStartScheduler();
}
#endif
