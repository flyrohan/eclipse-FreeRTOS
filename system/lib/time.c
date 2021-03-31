#include <stdarg.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef SYSTEM_TIME_ENABLED

static SysTime_Op *time_op;

void SysTime_Delay(int us)
{
	if (time_op->Delay)
		time_op->Delay(us);
}

uint64_t SysTime_GetTickUS(void)
{
	if (time_op->GetTickUS)
		return time_op->GetTickUS();

	return 0;
}

uint64_t SysTime_GetTick(void)
{
	return SysTime_GetTickUS() / 1000;
}

void SysTime_Register(SysTime_Op *op)
{
	time_op = op;
}
#endif
