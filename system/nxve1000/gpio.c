#include <cmsis_device.h>
#include <io.h>
#include <config.h>

#ifdef GPIO_ENABLED

#define GPIO_REG_ALTFN0		(GPIO_PHY_BASE + 0x20)
#define GPIO_REG_ALTFN1		(GPIO_PHY_BASE + 0x24)

#define GPIO_PAD_MAX		27

int GPIO_SetAlt(int pad, GPIO_ALTFUNC fn)
{
	uint32_t addr = GPIO_REG_ALTFN0;
	uint32_t val;
	int bit = pad;

	if (fn > GPIO_ALTFUNC_1)
		return -1;

	if (pad > GPIO_PAD_MAX)
		return -1;

	if (pad > 15) {
		addr = GPIO_REG_ALTFN1;
		bit = pad - 16;
	}

	bit *= 2;
	val = readl(addr) & ~(3ul << bit);
	val |= (uint32_t)(fn << bit);
	writel(val, addr);

	return 0;
}

#endif
