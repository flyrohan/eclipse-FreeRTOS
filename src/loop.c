#include <stdlib.h>
#include <config.h>

#ifdef LOOP_ENABLED

#define CRTLC_EXIT() do { \
	int c = Getc();	\
	if (isCtrlc(c))	\
		return 0;	\
	} while (0)

/*
 * $> loop <sleep:ms> <count>
 */
int do_loop(int argc, char * const argv[])
{
    int count = 10;
	int ms = 0;

	if (argc < 2)
		return -1;

	ms = (int)strtoul(argv[1], NULL, 10);

	if (argc > 2 && argv[2])
		count = (int)strtoul(argv[2], NULL, 10);

	for (int i = 0; i < count; i++)	{
		uint32_t tick = (uint32_t)SysTime_GetTick();

		if (Tstc())
			CRTLC_EXIT();	
		Printf("[%03d] %d : %d\r\n", i, tick, ms);
		if (ms)
			SysTime_Delay(ms);
	};

	return 0;
}

CMD_DEFINE(loop, do_loop);
#endif
