#include <cmsis_device.h>
#include <ExceptionHandlers.h>
#include <config.h>
#include <init.h>

#define SYSTICK_MAX_COUNT	((1 << 24) - 1)

#ifdef SYSTICK_ENABLED

#ifndef RTOS_ENABLED
static struct __attribute__((__packed__)) _SYSTICK_ {
	uint64_t timestamp;
	uint32_t lastdec;
	uint32_t ticksec;		/* tick_per_sec */
	uint32_t hz;
	uint32_t ratio;
} _systick = {
	.timestamp = 0,
};

static uint64_t SysTick_GetTickUS(void)
{
	uint64_t time = _systick.timestamp;
	uint32_t lastdec = _systick.lastdec;
	uint32_t now = SYSTICK_MAX_COUNT - SysTick->VAL;

	if (now >= lastdec)
		time += now - lastdec;
	else
		time += now + SYSTICK_MAX_COUNT - lastdec;

	_systick.lastdec = now;
	_systick.timestamp = time;

	return _systick.timestamp / _systick.ratio;
}

static void SysTick_Delay(int ms)
{
	uint64_t end = SysTick_GetTickUS() + (uint64_t)ms * 1000;

	while (SysTick_GetTickUS() < end) {
			;
	};
}

int SysTick_Init(unsigned int clock, int hz)
{
	uint32_t ticksec = clock / (uint32_t)hz;

	if ((ticksec - 1UL) > SysTick_LOAD_RELOAD_Msk)
	    return -1;

	_systick.ticksec = ticksec;
	_systick.ratio = clock / (uint32_t)(hz * hz);
	_systick.hz = (uint32_t)hz;

	NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);

	SysTick->VAL = 0UL;
	SysTick->LOAD = (uint32_t)(SYSTICK_MAX_COUNT);
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

	return 0;
}

static SysTime_Op SysTick_Op = {
	.Delay = SysTick_Delay,
	.GetTickUS = SysTick_GetTickUS,
};

void SysTick_Register(unsigned int clock, int hz)
{
	SysTick_Init(clock, hz);
	SysTime_Register(&SysTick_Op);
}
#else
#include <rtos.h>

static void rtos_Delay(int ms)
{
	vTaskDelay((TickType_t)ms);
}

static uint64_t rtos_GetTickUS(void)
{
	return (uint64_t)(xTaskGetTickCount() * 1000);
}

static SysTime_Op SysTick_Op = {
	.Delay = rtos_Delay,
	.GetTickUS = rtos_GetTickUS,
};

void SysTick_Register(unsigned int clock, int hz)
{
	SysTick_Config(clock/(uint32_t)hz);
	SysTime_Register(&SysTick_Op);
}
#endif /* RTOS_ENABLED */
#endif /* SYSTICK_ENABLED */
