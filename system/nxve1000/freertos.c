#include <config.h>

#ifdef RTOS_ENABLED

#include <freertos_implement.h>

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/*-----------------------------------------------------------*/
#ifndef CMSIS_ENABLED
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
#endif

/*-----------------------------------------------------------*/
#if ( configUSE_TICKLESS_IDLE == 1 )
static uint32_t ulTimerCountsForOneTick = 0;
/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
static uint32_t xMaximumPossibleSuppressedTicks = 0;
/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
static uint32_t ulStoppedTimerCompensation = 0;

#define portMAX_24_BIT_NUMBER		( 0xffffffUL )
#define portMAX_32_BIT_NUMBER		( 0xffffffffUL )

extern void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime);
#endif

#if defined(SYSTEM_TIME_MODULE) && (SYSTEM_TIME_MODULE == SYSTEM_TIME_SYSTICK)
#if ( configUSE_TICKLESS_IDLE == 1 )
/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portMISSED_COUNTS_FACTOR    		  ( 22UL )

void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
	uint32_t ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickDecrements;
    TickType_t xModifiableIdleTime;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
    {
    	xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
    }

    /* Stop the SysTick momentarily.  The time the SysTick is stopped for
     * is accounted for as best it can be, but using the tickless mode will
     * inevitably result in some tiny drift of the time maintained by the
     * kernel with respect to calendar time. */
    portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;

    /* Calculate the reload value required to wait xExpectedIdleTime
     * tick periods.  -1 is used because this code will execute part way
     * through one of the tick periods. */
    ulReloadValue = portNVIC_SYSTICK_CURRENT_VALUE_REG + ( ulTimerCountsForOneTick * ( xExpectedIdleTime - 1UL ) );

    if( ulReloadValue > ulStoppedTimerCompensation )
    {
        ulReloadValue -= ulStoppedTimerCompensation;
    }

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __asm volatile ( "cpsid i" ::: "memory" );
    __asm volatile ( "dsb" );
    __asm volatile ( "isb" );

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    if( eTaskConfirmSleepModeStatus() == eAbortSleep )
    {
        /* Restart from whatever is left in the count register to complete
         * this tick period. */
        portNVIC_SYSTICK_LOAD_REG = portNVIC_SYSTICK_CURRENT_VALUE_REG;

        /* Restart SysTick. */
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

        /* Reset the reload register to the value required for normal tick
         * periods. */
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;

        /* Re-enable interrupts - see comments above the cpsid instruction()
         * above. */
        __asm volatile ( "cpsie i" ::: "memory" );
    }
    else
    {
        /* Set the new reload value. */
        portNVIC_SYSTICK_LOAD_REG = ulReloadValue;

        /* Clear the SysTick count flag and set the count value back to
         * zero. */
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

        /* Restart SysTick. */
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
         * set its parameter to 0 to indicate that its implementation contains
         * its own wait for interrupt or wait for event instruction, and so wfi
         * should not be executed again.  However, the original expected idle
         * time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;
        configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

        if( xModifiableIdleTime > 0 )
        {
            __asm volatile ( "dsb" ::: "memory" );
            __asm volatile ( "wfi" );
            __asm volatile ( "isb" );
        }

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

        /* Re-enable interrupts to allow the interrupt that brought the MCU
         * out of sleep mode to execute immediately.  see comments above
         * __disable_interrupt() call above. */
        __asm volatile ( "cpsie i" ::: "memory" );
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );

        /* Disable interrupts again because the clock is about to be stopped
         * and interrupts that execute while the clock is stopped will increase
         * any slippage between the time maintained by the RTOS and calendar
         * time. */
        __asm volatile ( "cpsid i" ::: "memory" );
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );

        /* Disable the SysTick clock without reading the
         * portNVIC_SYSTICK_CTRL_REG register to ensure the
         * portNVIC_SYSTICK_COUNT_FLAG_BIT is not cleared if it is set.  Again,
         * the time the SysTick is stopped for is accounted for as best it can
         * be, but using the tickless mode will inevitably result in some tiny
         * drift of the time maintained by the kernel with respect to calendar
         * time*/
        portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT );

        /* Determine if the SysTick clock has already counted to zero and
         * been set back to the current reload value (the reload back being
         * correct for the entire expected idle time) or if the SysTick is yet
         * to count to zero (in which case an interrupt other than the SysTick
         * must have brought the system out of sleep mode). */
        if( ( portNVIC_SYSTICK_CTRL_REG & portNVIC_SYSTICK_COUNT_FLAG_BIT ) != 0 )
        {
            uint32_t ulCalculatedLoadValue;

            /* The tick interrupt is already pending, and the SysTick count
             * reloaded with ulReloadValue.  Reset the
             * portNVIC_SYSTICK_LOAD_REG with whatever remains of this tick
             * period. */
            ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL ) - ( ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG );

            /* Don't allow a tiny value, or values that have somehow
             * underflowed because the post sleep hook did something
             * that took too long. */
            if( ( ulCalculatedLoadValue < ulStoppedTimerCompensation ) || ( ulCalculatedLoadValue > ulTimerCountsForOneTick ) )
            {
                ulCalculatedLoadValue = ( ulTimerCountsForOneTick - 1UL );
            }

            portNVIC_SYSTICK_LOAD_REG = ulCalculatedLoadValue;

            /* As the pending tick will be processed as soon as this
             * function exits, the tick value maintained by the tick is stepped
             * forward by one less than the time spent waiting. */
            ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
        }
        else
        {
            /* Something other than the tick interrupt ended the sleep.
             * Work out how long the sleep lasted rounded to complete tick
             * periods (not the ulReload value which accounted for part
             * ticks). */
            ulCompletedSysTickDecrements = ( xExpectedIdleTime * ulTimerCountsForOneTick ) - portNVIC_SYSTICK_CURRENT_VALUE_REG;

            /* How many complete tick periods passed while the processor
             * was waiting? */
            ulCompleteTickPeriods = ulCompletedSysTickDecrements / ulTimerCountsForOneTick;

            /* The reload value is set to whatever fraction of a single tick
             * period remains. */
            portNVIC_SYSTICK_LOAD_REG = ( ( ulCompleteTickPeriods + 1UL ) * ulTimerCountsForOneTick ) - ulCompletedSysTickDecrements;
        }

        /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
         * again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
         * value. */
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
        vTaskStepTick( ulCompleteTickPeriods );
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;

        /* Exit with interrpts enabled. */
        __asm volatile ( "cpsie i" ::: "memory" );
    }
}
#endif /* configUSE_TICKLESS_IDLE */

static void rtos_TimerInit(void)
{
	#if ( configUSE_TICKLESS_IDLE == 1 )
    ulTimerCountsForOneTick = ( configCPU_CLOCK_HZ / configTICK_RATE_HZ );
    xMaximumPossibleSuppressedTicks = portMAX_24_BIT_NUMBER / ulTimerCountsForOneTick;
    ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR;
	#endif /* configUSE_TICKLESS_IDLE */

	SysTick_Register(SYSTEM_CLOCK, SYSTEM_TICK_HZ);
}
#else /* SYSTEM_TIME_MODULE == SYSTEM_TIME_SYSTICK */

#if ( configUSE_TICKLESS_IDLE == 1 )
#define	TIMER_CH_OFFSET			(0x100)
#define TIMER_REG_TCTRL			(TIMER_PHY_BASE + (TIMER_CH_OFFSET * TIMER_CH) + 0x08)
#define TIMER_REG_TCNTB			(TIMER_PHY_BASE + (TIMER_CH_OFFSET * TIMER_CH) + 0x0C)
#define TIMER_REG_TCMPB			(TIMER_PHY_BASE + (TIMER_CH_OFFSET * TIMER_CH) + 0x10)
#define TIMER_REG_TCNTO			(TIMER_PHY_BASE + (TIMER_CH_OFFSET * TIMER_CH) + 0x14)
#define TIMER_REG_TINTS			(TIMER_PHY_BASE + (TIMER_CH_OFFSET * TIMER_CH) + 0x18)

#define __TIMER_START			((*((volatile uint32_t *)TIMER_REG_TCTRL)) = ((1UL << 0UL) | (0UL << 1UL) | (1UL << 3UL)))
#define __TIMER_STOP			((*((volatile uint32_t *)TIMER_REG_TCTRL)) &= ~(1UL << 0UL))
#define __TIMER_IRQ_DISABLE		((*((volatile uint32_t *)TIMER_REG_TINTS)) = 0x0)
#define __TIMER_IRQ_ENABLE		((*((volatile uint32_t *)TIMER_REG_TINTS)) |= (1UL << 0UL))
#define __TIMER_IRQ_CLEAR		((*((volatile uint32_t *)TIMER_REG_TINTS)) |= (1UL << 5UL))
#define __TIMER_CURRENT_VAL		(*((volatile uint32_t *)TIMER_REG_TCNTB) - *((volatile uint32_t *)TIMER_REG_TCNTO))
#define __TIMER_MANUALUPDATE	((*((volatile uint32_t *)TIMER_REG_TCTRL)) |= (1UL << 1UL))
#define __TIMER_TCNTB			(*((volatile uint32_t *)TIMER_REG_TCNTB))
#define __TIMER_TCMPB			(*((volatile uint32_t *)TIMER_REG_TCMPB))
#define __TIMER_TINTS			(*((volatile uint32_t *)TIMER_REG_TINTS))

#define TIMER_TCON_MANUALUPDATE   	( 1UL << 1UL )
#define TIMER_TINTS_PEND_BIT		( 1UL << 5UL )	/* TINT_CSTAT */

#define portMISSED_COUNTS_FACTOR    configMISSED_COUNTS_FACTOR

void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
	uint32_t ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickDecrements;
    TickType_t xModifiableIdleTime;
    uint32_t uInterruptPended, uCurrentValue;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
    {
    	xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
    }

    /* Stop the Timer momentarily */
    __TIMER_STOP;

    ulReloadValue = __TIMER_CURRENT_VAL + ( ulTimerCountsForOneTick * ( xExpectedIdleTime ) );

    if( ulReloadValue > ulStoppedTimerCompensation )
        ulReloadValue -= ulStoppedTimerCompensation;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __asm volatile ( "cpsid i" ::: "memory" );
    __asm volatile ( "dsb" );
    __asm volatile ( "isb" );

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    if( eTaskConfirmSleepModeStatus() == eAbortSleep )
    {
        /* Restart from whatever is left in the count register to complete
         * this tick period. */
    	__TIMER_TCNTB = __TIMER_CURRENT_VAL;
    	__TIMER_TCMPB = __TIMER_TCNTB;
    	__TIMER_MANUALUPDATE;

        /* Restart SysTick. */
    	__TIMER_IRQ_CLEAR;
    	__TIMER_START;

        /* Reset the reload register to the value required for normal tick
         * periods. */
    	__TIMER_TCNTB = ulTimerCountsForOneTick;
    	__TIMER_TCMPB = __TIMER_TCNTB;

        /* Re-enable interrupts - see comments above the cpsid instruction()
         * above. */
        __asm volatile ( "cpsie i" ::: "memory" );
    }
    else
    {
        /* Set the new reload value. */
    	__TIMER_TCNTB = ulReloadValue;
    	__TIMER_TCMPB = __TIMER_TCNTB;
    	__TIMER_MANUALUPDATE;

        /* Restart SysTick. */
    	__TIMER_IRQ_CLEAR;
    	__TIMER_START;

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
         * set its parameter to 0 to indicate that its implementation contains
         * its own wait for interrupt or wait for event instruction, and so wfi
         * should not be executed again.  However, the original expected idle
         * time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;
        configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

        if( xModifiableIdleTime > 0 )
        {
            __asm volatile ( "dsb" ::: "memory" );
            __asm volatile ( "wfi" );
            __asm volatile ( "isb" );
        }

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

        uCurrentValue = __TIMER_CURRENT_VAL;
        uInterruptPended = (__TIMER_TINTS & TIMER_TINTS_PEND_BIT) != 0 ? 1 : 0;

        /* Re-enable interrupts to allow the interrupt that brought the MCU
         * out of sleep mode to execute immediately.  see comments above
         * __disable_interrupt() call above. */
        __asm volatile ( "cpsie i" ::: "memory" );
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );

        /* Disable interrupts again because the clock is about to be stopped
         * and interrupts that execute while the clock is stopped will increase
         * any slippage between the time maintained by the RTOS and calendar
         * time. */
        __asm volatile ( "cpsid i" ::: "memory" );
        __asm volatile ( "dsb" );
        __asm volatile ( "isb" );

        /* Disable the SysTick clock without reading the
         * portTIMER_CTRL_REG register to ensure the
         * TIMER_TINTS_PEND_BIT is not cleared if it is set.  Again,
         * the time the SysTick is stopped for is accounted for as best it can
         * be, but using the tickless mode will inevitably result in some tiny
         * drift of the time maintained by the kernel with respect to calendar
         * time*/
         __TIMER_STOP;

        if( uInterruptPended )
        {
            uint32_t ulCalculatedLoadValue;

            /* The tick interrupt is already pending, and the SysTick count
             * reloaded with ulReloadValue.  Reset the
             * portTIMER_COUNT_REG with whatever remains of this tick
             * period. */
            ulCalculatedLoadValue = ( ulTimerCountsForOneTick ) - ( ulReloadValue - uCurrentValue );

            /* Don't allow a tiny value, or values that have somehow
             * underflowed because the post sleep hook did something
             * that took too long. */
            if( ( ulCalculatedLoadValue < ulStoppedTimerCompensation ) || ( ulCalculatedLoadValue > ulTimerCountsForOneTick ) )
            {
                ulCalculatedLoadValue = ( ulTimerCountsForOneTick );
            }

            __TIMER_TCNTB = ulCalculatedLoadValue;
            __TIMER_TCMPB = __TIMER_TCNTB;

            /* As the pending tick will be processed as soon as this
             * function exits, the tick value maintained by the tick is stepped
             * forward by one less than the time spent waiting. */
            ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
        }
        else
        {
            /* Something other than the tick interrupt ended the sleep.
             * Work out how long the sleep lasted rounded to complete tick
             * periods (not the ulReload value which accounted for part
             * ticks). */
        	ulCompletedSysTickDecrements = ( xExpectedIdleTime * ulTimerCountsForOneTick ) - uCurrentValue;

            /* How many complete tick periods passed while the processor
             * was waiting? */
            ulCompleteTickPeriods = ulCompletedSysTickDecrements / ulTimerCountsForOneTick;

            /* The reload value is set to whatever fraction of a single tick
             * period remains. */
            __TIMER_TCNTB = ( ( ulCompleteTickPeriods + 1UL ) * ulTimerCountsForOneTick ) - ulCompletedSysTickDecrements;
            __TIMER_TCMPB = __TIMER_TCNTB;
        }

        /* Restart SysTick so it runs from portTIMER_COUNT_REG
         * again, then set portTIMER_COUNT_REG back to its standard
         * value. */
        __TIMER_MANUALUPDATE;
        __TIMER_IRQ_CLEAR;
		__TIMER_START;

        vTaskStepTick( ulCompleteTickPeriods );

        __TIMER_TCNTB = ulTimerCountsForOneTick;
        __TIMER_TCMPB = __TIMER_TCNTB;

        /* Exit with interrpts enabled. */
        __asm volatile ( "cpsie i" ::: "memory" );
    }
}
#endif /* configUSE_TICKLESS_IDLE */

static uint64_t __SysTime_GetTimeUS(void)
{
	return (uint64_t)(xTaskGetTickCount() * 1000);
}

static void __SysTime_Delay(int ms)
{
	vTaskDelay((TickType_t)ms);
}

static SysTime_Op Timer_Op = {
	.GetTimeUS = __SysTime_GetTimeUS,
	.Delay = __SysTime_Delay,
};

static void rtos_TimerInit(void)
{
	#if ( configUSE_TICKLESS_IDLE == 1 )
    ulTimerCountsForOneTick = ( TIMER_CLOCK_HZ / configTICK_RATE_HZ );
    xMaximumPossibleSuppressedTicks = portMAX_32_BIT_NUMBER / ulTimerCountsForOneTick;
    ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR;
	#endif /* configUSE_TICKLESS_IDLE */

	TIMER_Register(TIMER_CH, SYSTEM_CLOCK, SYSTEM_TICK_HZ,
				TIMER_MODE_PERIODIC, &Timer_Op);
	TIMER_CallbackISR(TIMER_CH, (ISR_Callback)xPortSysTickHandler, NULL);
}
#endif

/*-----------------------------------------------------------*/
/* Replace Cortex-M0 timer interrupt enable function.
 * vPortSetupTimerInterrupt is defined at port.c with 'weak'
 * and enable Cortex-M0 SYSTICK
 */
extern void vPortSetupTimerInterrupt(void);
void vPortSetupTimerInterrupt(void)
{
	rtos_TimerInit();
}
/*-----------------------------------------------------------*/
void prvTaskExitErrorHook(void)
{
	vTaskDelete(NULL);
}

/*-----------------------------------------------------------*/
#include <rtos.h>

void __attribute__((weak)) HAL_RunRTOS(void)
{
#ifdef CMSIS_ENABLED
	osKernelInitialize();
	osKernelStart();
#else
	vTaskStartScheduler();
#endif
}

#endif /* RTOS_ENABLED */
