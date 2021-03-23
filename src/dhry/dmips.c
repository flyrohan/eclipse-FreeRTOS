#include <stdlib.h>
#include <config.h>

#ifdef DMIPS_ENABLED

#define _MHZ(_hz)	(_hz/1000000)

extern void DMIPS(int number_of_runs, int cpu_mhz);

static int do_dmips(int argc, char * const argv[])
{
	int number_of_runs = 400000;

	if (argc > 1 && argv[1])
		number_of_runs = strtol(argv[1], NULL, 10);

	DMIPS(number_of_runs, _MHZ(SYSTEM_CLOCK));

	return 0;
}

CMD_DEFINE(dmips, do_dmips);
#endif
