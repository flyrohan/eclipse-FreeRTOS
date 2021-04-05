#include <stdbool.h>
#include <cmsis_device.h>
#include <timer.h>
#include <systime.h>
#include <io.h>
#include <config.h>
#include <ExceptionHandlers.h>

#ifdef TIMER_ENABLED

#define TIMER_CLOCK_HZ			1000000			/* 1Mhz */
#define	TIMER_MAX_COUNT			-(1UL)
#define TIMER_MUX_SEL			0 				/* bypass */

#define	TIMER_CH_OFFSET			(0x100)

#define TCFG0_PRESCALER_MASK	0xff
#define TCFG1_MUX_MASK          0x7
#define TCON_START				BIT(0)
#define TCON_MANUALUPDATE   	BIT(1)
#define TCON_INVERT         	BIT(2)
#define TCON_AUTORELOAD    		BIT(3)

#define TINT_STATUS    			BIT(5)
#define TINT_ENABLE    			BIT(0)

typedef struct {
  __IOM uint32_t TCFG0;			/* 0x00 */
  __IOM uint32_t TCFG1;			/* 0x04 */
  __IOM uint32_t TCON;			/* 0x08 */
  __IOM uint32_t TCNTB;			/* 0x0C */
  __IOM uint32_t TCMPB;			/* 0x10 */
  __IOM uint32_t TCNTO;			/* 0x14 */
  __IOM uint32_t TINT_CSTAT;	/* 0x18 */
} Timer_Reg;

/*
 * Timer HW
 */
static struct __attribute__((__packed__)) _TIMER_ {
	Timer_Reg *base;
	uint64_t timestamp;
	uint32_t lastdec;
	int irqno;
} _timer = {
	.timestamp = 0,
	.lastdec = 0,
	.irqno = TIMER0_IRQn,
};

#ifndef RTOS_ENABLED
static uint64_t TIMER_GetTickUS(void)
{
	uint64_t time = _timer.timestamp;
	uint32_t lastdec = _timer.lastdec;
	uint32_t now = TIMER_MAX_COUNT - readl(&_timer.base->TCNTO);

	if (now >= lastdec)
		time += now - lastdec;
	else
		time += now + TIMER_MAX_COUNT - lastdec;

	_timer.lastdec = now;
	_timer.timestamp = time;

	return _timer.timestamp;
}

static void TIMER_Delay(int ms)
{
	uint64_t end = TIMER_GetTickUS() + (uint64_t)ms * 1000;

	while (TIMER_GetTickUS() < end) {
			;
	};
}

static SysTime_Op SysTick_Op __attribute__((unused)) = {
	.Delay = TIMER_Delay,
	.GetTickUS = TIMER_GetTickUS,
};
#else
#include <rtos.h>

void __attribute__((weak)) Timer_IRQHandler (void) { }

void Timer_Handler(void)
{
	writel(TINT_STATUS | TINT_ENABLE, &_timer.base->TINT_CSTAT);

	/* Call xPortSysTickHandler in FreeRTOS */
	Timer_IRQHandler();
}

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
#endif

static void TIMER_Config(int mux, int scale, unsigned int count)
{
	writel(_mask(_timer.base->TCFG1, TCFG1_MUX_MASK) | (uint32_t)mux, &_timer.base->TCFG1);
	writel(_mask(_timer.base->TCFG0, TCFG0_PRESCALER_MASK) | (uint32_t)(scale - 1), &_timer.base->TCFG0);
	writel(count, &_timer.base->TCNTB);
	writel(count, &_timer.base->TCMPB);
}

static void TIMER_Start(bool irqenb)
{
	if (irqenb)
    	writel(TINT_STATUS | TINT_ENABLE, &_timer.base->TINT_CSTAT);

	writel((readl(&_timer.base->TCON) | TCON_MANUALUPDATE), &_timer.base->TCON);
	writel(TCON_AUTORELOAD | TCON_START, &_timer.base->TCON);
}

static void TIMER_Stop(void)
{
	writel(0x0, &_timer.base->TINT_CSTAT);
	writel(_mask(_timer.base->TCON, TCON_START), &_timer.base->TCON);
}

int TIMER_Init(int ch, unsigned int clock, int hz __attribute__((unused)))
{
	unsigned int count;
	bool irqenb;
	int scale;
	
	if (ch > 7)
		return -1;

	_timer.base = (void *)(TIMER_PHY_BASE + (TIMER_CH_OFFSET * ch));
	_timer.irqno = TIMER0_IRQn + ch;

#ifdef RTOS_ENABLED
	irqenb = true;
	count = (unsigned int)hz;
	scale = (int)clock / TIMER_CLOCK_HZ;

	NVIC_SetPriority(_timer.irqno, 0);
	NVIC_EnableIRQ(_timer.irqno);
#else
	irqenb = false;
	count = TIMER_MAX_COUNT;
	scale = (int)clock / TIMER_CLOCK_HZ;
#endif

	TIMER_Stop();
	TIMER_Config(TIMER_MUX_SEL, scale, count);
	TIMER_Start(irqenb);

	return 0;
}

void TIMER_Register(int ch, unsigned int clock, int hz)
{
	TIMER_Init(ch, clock, hz);
	SysTime_Register(&SysTick_Op);
}

#endif
