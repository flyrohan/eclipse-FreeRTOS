#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <rtos.h>
#include <config.h>

#ifdef LOOP_ENABLED

#define CRTLC_EXIT() do { \
	int c = Getc();	\
	if (isCtrlc(c))	\
		return 0;	\
	} while (0)

#ifdef RTOS_ENABLED

static struct loop_t {
	int ms, count;
} _loop_t = {
	.ms = 1000,
	.count = 10,
};

#ifdef CMSIS_ENABLED
static osThreadId Loop_Handle = NULL;

static void Loop_Thread(void const *argument)
{
	struct loop_t *lt = (struct loop_t *)argument;
	osThreadId id = Loop_Handle;
	int count = 1;

	do {
		Printf("[%d] %d (%d ms), %d\r\n",
				osThreadGetId(), osKernelGetTickCount(), lt->ms, count);
		if (lt->ms)
			osDelay((uint32_t)lt->ms);
	} while (count++ < _loop_t.count);

	Loop_Handle = NULL;
	osThreadTerminate(id);
}

CMD_TYPE int do_loop(int argc, char * const argv[])
{
	if (argc > 1 && argv[1]) {
		if (!strcmp(argv[1], "exit")) {
			if (Loop_Handle)
				osThreadTerminate(Loop_Handle);
			Loop_Handle = NULL;
			return 0;
		}
		_loop_t.ms = strtol(argv[1], NULL, 10);
	}

	if (argc > 2 && argv[2])
		_loop_t.count = (int)strtol(argv[2], NULL, 10);

	Printf("loop delay: %d ms, count %d (0x%x)\n", _loop_t.ms, _loop_t.count, Loop_Handle);

	if (Loop_Handle)
		osThreadTerminate(Loop_Handle);

	osThreadDef(Loop_Thread, osPriorityLow, 0, configMINIMAL_STACK_SIZE);
	osThreadDef_t *thread_def = (osThreadDef_t *)osThread(Loop_Thread);
	thread_def->attr.name = "Loop-Task";

	Loop_Handle = osThreadCreate(osThread(Loop_Thread), (void *)&_loop_t);

	return 0;
}
#else

static TaskHandle_t Loop_Handle = NULL;

static void Loop_Thread(void *argument)
{
	struct loop_t *lt = (struct loop_t *)argument;
	int count = 1;

	do {
		Printf("[%d] %d (%d ms), %d\r\n",
				xTaskGetCurrentTaskHandle(), xTaskGetTickCount(), lt->ms, count);
		if (lt->ms)
			vTaskDelay((TickType_t)lt->ms);
	} while (count++ < _loop_t.count);

	Loop_Handle = NULL;
	vTaskDelete(NULL);
}

CMD_TYPE int do_loop(int argc, char * const argv[])
{
	if (argc > 1 && argv[1]) {
		if (!strcmp(argv[1], "exit")) {
			if (Loop_Handle)
				vTaskDelete(Loop_Handle);
			Loop_Handle = NULL;
			return 0;
		}
		_loop_t.ms = strtol(argv[1], NULL, 10);
	}

	if (argc > 2 && argv[2])
		_loop_t.count = (int)strtol(argv[2], NULL, 10);

	Printf("loop delay: %d ms, count %d (0x%x)\n", _loop_t.ms, _loop_t.count, Loop_Handle);

	if (Loop_Handle)
		vTaskDelete(Loop_Handle);

	return xTaskCreate(Loop_Thread,
			 	 	 "Loop-Task",
					(( unsigned short )128),
					(void *)&_loop_t,
					(UBaseType_t)osPriorityLow,
					&Loop_Handle);
}
#endif /* CMSIS_ENABLED */

#else

CMD_TYPE int do_loop(int argc, char * const argv[])
{
    int count = 10;
	int ms = 0;

	if (argc < 2)
		return -1;

	ms = (int)strtoul(argv[1], NULL, 10);

	if (argc > 2 && argv[2])
		count = (int)strtoul(argv[2], NULL, 10);

	for (int i = 0; i < count; i++)	{
		uint32_t tick = (uint32_t)SysTime_GetTime();

		if (Tstc())
			CRTLC_EXIT();	
		Printf("[%03d] %d : %d\r\n", i, tick, ms);
		if (ms)
			SysTime_Delay(ms);
	};

	return 0;
}
#endif

/*
 * $> loop <sleep:ms> <count>
 */
CMD_DEFINE(loop, do_loop);
#endif
