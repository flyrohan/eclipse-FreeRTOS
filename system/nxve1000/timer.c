#include <stdbool.h>
#include <stddef.h>
#include <cmsis_device.h>
#include <timer.h>
#include <systime.h>
#include <sysirq.h>
#include <io.h>
#include <config.h>
#include <ExceptionHandlers.h>

#ifdef TIMER_ENABLED

#define TIMER_CHS				8
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
static struct TIMER_t {
	Timer_Reg *base;
	uint64_t timestamp;
	uint32_t lastdec;
	uint32_t ratio;
	int irqno;
	TIMER_MODE_t mode;
	ISR_Callback_t cb;
} _timer[TIMER_CHS] = { };

uint64_t TIMER_GetTickUS(int ch)
{
	uint64_t time = _timer[ch].timestamp;
	uint32_t lastdec = _timer[ch].lastdec;
	uint32_t now = (TIMER_MAX_COUNT - readl(&_timer[ch].base->TCNTO)) / _timer[ch].ratio;

	if (now >= lastdec)
		time += now - lastdec;
	else
		time += now + TIMER_MAX_COUNT - lastdec;

	_timer[ch].lastdec = now;
	_timer[ch].timestamp = time;

	return _timer[ch].timestamp;
}

void TIMER_Delay(int ch, int ms)
{
	uint64_t end = TIMER_GetTickUS(ch) + (uint64_t)ms * 1000;

	while (TIMER_GetTickUS(ch) < end) {
			;
	};
}

void TIMER_Frequency(int ch, int mux, int scale,
							unsigned int count, unsigned int cmp)
{
	struct TIMER_t *timer = &_timer[ch];

	writel(_mask(timer->base->TCFG1, TCFG1_MUX_MASK) |
			(uint32_t)mux, &timer->base->TCFG1);
	writel(_mask(timer->base->TCFG0, TCFG0_PRESCALER_MASK) |
			(uint32_t)(scale - 1), &timer->base->TCFG0);
	writel(count, &timer->base->TCNTB);
	writel(cmp, &timer->base->TCMPB);
}

void TIMER_Start(int ch)
{
	struct TIMER_t *timer = &_timer[ch];

	if (timer->mode != TIMER_MODE_FREERUN)
		writel(TINT_STATUS | TINT_ENABLE, &timer->base->TINT_CSTAT);

	writel((readl(&timer->base->TCON) | TCON_MANUALUPDATE), &timer->base->TCON);
	writel(TCON_AUTORELOAD | TCON_START, &timer->base->TCON);
}

void TIMER_Stop(int ch)
{
	struct TIMER_t *timer = &_timer[ch];

	writel(0x0, &timer->base->TINT_CSTAT);
	writel(_mask(timer->base->TCON, TCON_START), &timer->base->TCON);
}

static void Timer_Handler(int irq, void *argument);

int TIMER_Init(int ch, unsigned int infreq, unsigned int tfreq, int hz, TIMER_MODE_t mode)
{
	struct TIMER_t *timer = &_timer[ch];
	unsigned int count = TIMER_MAX_COUNT;
	int scale = (int)(infreq / tfreq);

	timer->base = (void *)(TIMER_PHY_BASE + (TIMER_CH_OFFSET * ch));
	timer->ratio = tfreq / ((unsigned int)hz * 1000);
	timer->irqno = TIMER0_IRQn + ch;
	timer->mode = mode;

	ISR_Register(TIMER0_IRQn + ch, Timer_Handler, NULL);
	if (timer->mode != TIMER_MODE_FREERUN) {
		count = tfreq / (unsigned  int)hz;
		NVIC_SetPriority(timer->irqno, 0);
		NVIC_EnableIRQ(timer->irqno);
	}

	TIMER_Stop(ch);
	TIMER_Frequency(ch, TIMER_MUX_SEL, scale, count, count);

	return 0;
}

static int systime_ch;
#define SysTime_Channel(_ch)		(systime_ch = _ch)
#define SysTime_GetChannel()		(systime_ch)

static uint64_t __SysTime_GetTickUS(void)
{
	return TIMER_GetTickUS(SysTime_GetChannel());
}

static void __SysTime_Delay(int ms)
{
	TIMER_Delay(SysTime_GetChannel(), ms);
}

static SysTime_Op Timer_Op = {
	.GetTickUS = __SysTime_GetTickUS,
	.Delay = __SysTime_Delay,
};

void TIMER_Register(int ch, unsigned int infreq, unsigned int tfreq, int hz,
					TIMER_MODE_t mode, SysTime_Op *op)
{
	if (ch < 0 || ch > TIMER_CHS)
		return;

	SysTime_Channel(ch);
	SysTime_Register(op ? op : &Timer_Op);

	TIMER_Init(ch, infreq, tfreq, hz, mode);
	TIMER_Start(ch);
}

static void Timer_Handler(int irq, void *argument __attribute__((unused)))
{
	struct TIMER_t *timer = &_timer[irq - TIMER0_IRQn];
	uint32_t flag = readl(&timer->base->TINT_CSTAT) | TINT_ENABLE;

	writel(TINT_STATUS | flag, &timer->base->TINT_CSTAT);
	if (timer->cb.func)
		timer->cb.func(timer->cb.argument);
}

void TIMER_CallbackISR(int ch, ISR_Callback cb, void *argument)
{
	_timer[ch].cb.func = cb;
	_timer[ch].cb.argument = argument;
}
#endif  /* TIMER_ENABLED */
