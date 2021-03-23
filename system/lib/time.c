#include <stdarg.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef SYSTIME_ENABLED

static SysTime_Op *time_op;

void SysTime_Delay(int us)
{
	if (time_op->Delay)
		time_op->Delay(us);
}

uint64_t SysTime_GetTick(void)
{
	if (time_op->GetTick)
		return time_op->GetTick();

	return 0;
}

void SysTime_Register(SysTime_Op *op)
{
	time_op = op;
}
#endif
