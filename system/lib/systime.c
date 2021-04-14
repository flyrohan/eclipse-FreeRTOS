#include <stdarg.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef SYSTEM_TIME_MODULE

static SysTime_Op *time_op;

void SysTime_Delay(int us)
{
	if (time_op->Delay)
		time_op->Delay(us);
}

uint64_t SysTime_GetTimeUS(void)
{
	if (time_op->GetTimeUS)
		return time_op->GetTimeUS();

	return 0;
}

uint64_t SysTime_GetTime(void)
{
	return SysTime_GetTimeUS() / 1000;
}

void SysTime_Register(SysTime_Op *op)
{
	time_op = op;
}
#endif
