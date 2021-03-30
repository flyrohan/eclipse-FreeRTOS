#include <cmsis_device.h>
#include <ExceptionHandlers.h>
#include <time.h>
#include <rtos.h>
#include <config.h>

#ifdef RTOS_ENABLED

#ifdef RTOS_TICK_SYSTICK
void HAL_IncTick(void)
{
	SysTick_Handler();
}
#endif

static void	 rtos_Delay(int ms)
{
	vTaskDelay((TickType_t)ms);
}

static uint64_t rtos_GetTickUS(void)
{
	return (uint64_t)(xTaskGetTickCount() * 1000);
}

static SysTime_Op Tick_Op = {
	.Delay = rtos_Delay,
	.GetTickUS = rtos_GetTickUS,
};

void HAL_RTOSInit(uint32_t ticks)
{
#ifdef RTOS_TICK_SYSTICK
	SysTick_Config(ticks);
#else
	#error "Not implemented RTOS Tick..."
#endif

	SysTime_Register(&Tick_Op);
}

#endif
