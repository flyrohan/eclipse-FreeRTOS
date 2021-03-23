#include <cmsis_device.h>
#include <system.h>
#include <io.h>
#include <config.h>

#ifdef REMAP_ENABLED
#define REMAP_OFFSET	0x0c

void System_Remap(void)
{
	writel(0x1, (SYSREG_PHY_BASE + REMAP_OFFSET));
}

#endif
