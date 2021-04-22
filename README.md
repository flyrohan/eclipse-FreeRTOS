# eclipse-FreeRTOS

Base Code
	- based on eclipse-firmware

	FreeRTOS					: v10.4.1
	CMSIS						: 5.7.0
	arm-cmsis-rtos-validator	: xpack

--------------------------------------------------------------------------------
1. Base Porting FreeRTOS with ARMDS (skip)
--------------------------------------------------------------------------------

	Get source files from Arm Development Studio
		- ARMDS RTE is not include header's source files

	[Arm Development Studio]
	A. Create CMSIS project
		- New -> Project -> 'C Project' -> 'CMSIS C/C++ Project' -> [Arm Compiler 6] 
			-> Project Name  : 'CMSIS' 
			-> Select Device : ARM -> ARM Coretex M0 -> ARMCM0

		[File]
			- SRC		: /RTE		- RTE_Components.h
			- Scatter	: cmsis_rte_ARMCM0.sct
			- Config	: CMSIS.rteconfig	

	B. Get FreeRTOS source with 'CMSIS.rteconfig'
		: Note

		- Startup code
			[CMSIS]		: [x] CORE
						
			[Device]	: [x] Startup
						: /RTE					- RTE_Components.h
						: /RTE/Device/ARMCM0	- startup_ARMCM0.s, system_ARMCM0.c
							
		- FreeRTOS code

			[RTOS]	: [x] Config 
						: /RTE/RTOS				- FreeTROSConfig.h

			[RTOS]	: [x] Core 
						: /RTE/RTOS				- freertos_evr.c
						: /RTE/RTOS				- list.c
						: /RTE/RTOS				- port.c
						: /RTE/RTOS				- queue.c
						: /RTE/RTOS				- task.c

			[RTOS]	: [x] Coroutines 
						: /RTE/RTOS				- croutine.c

			[RTOS]	: [x] Event Groups 
						: /RTE/RTOS				- evebt_groups.c

			[RTOS]	: [x] Heap 
						: /RTE/RTOS				- heap_4.c
						: /RTE/RTOS				- list.c

			[RTOS]	: [x] Message Buffer 
						: /RTE/RTOS				- stream_buffer.c

			[RTOS]	: [x] Stream Buffer 
						: /RTE/RTOS				- stream_buffer.c

			[RTOS]	: [x] Timers
						: /RTE/RTOS				- timers.c
						
	C. Porting FreeRTOS requirement functions 

			- freertos_implement.c
			Functions:
				vApplicationGetIdleTaskMemory
				vApplicationGetTimerTaskMemory
				vApplicationStackOverflowHook

	D. main.c

		- Add main.c

--------------------------------------------------------------------------------
2. Base Porting FreeRTOS with FreeRTOS github 
--------------------------------------------------------------------------------

	Porting FreeRTOS with FreeRTOS git to firmware source
	------------------------------------------------------------------------
	FreeRTOS V10.4.3
	: Current CMSIS-FreeRTOS lastest is V10.3.1 and develop is v10.4.1
	------------------------------------------------------------------------

	Get FreeRTOS-Kernel
		$> https://github.com/FreeRTOS/FreeRTOS-Kernel.git
		$> git checkout V10.4.3 

	A. header file
		$> cp -a include <PATH>/include 

	B. source file
		$> cp -a *.c <PATH>/
			croutine.c
			event_groups.c
			list.c
			queue.c
			stream_buffer.c
			tasks.c
			timers.c

		$> cp portable/GCC/ARM_CM0/port.c <PATH>/
			port.c

		$> cp ortable/GCC/ARM_CM0/portmacro.h <PATH>/include
			portmacro.h

		$> cp portable/MemMang/heap_4.c <PATH>/
			heap_4.c

	C. FreeRTOS porting	

		A. kernel/include/FreeRTOSConfig.h
		- FreeRTOS configuration

		B. FreeRTOS require functions
			[freertos_implement.c]

			- vApplicationGetIdleTaskMemory
			- vApplicationGetTimerTaskMemory
			- vApplicationStackOverflowHook
				: called by vApplicationStackOverflowHook
 				: terminates the task that caused the stackoverflow. ****

--------------------------------------------------------------------------------
3. Porting FreeRTOS to Firmware
--------------------------------------------------------------------------------
	1. ADD or Remove 'kernel' direcotry to Eclipse
			[Properties] -> C/C++ General -> Paths and Symbols
			: Source Location
		
	2. Timer with systick

			- inclue "FreeRTOSConfig.h" to replace SysTick_Handler weak function
			  in "exception_handlers.c"

			[ vectors_nxve1000.c]
			: #include <FreeRTOSConfig.h> <- ADD

			[FreeRTOSConfig.h]
			-> #define SysTick_Handler	xPortSysTickHandler


	3. FreeRTOSConfig.h
		- Set configCPU_CLOCK_HZ
		#define	SystemCoreClock				SYSTEM_CLOCK <--- 48000000
		#define configCPU_CLOCK_HZ         	(SystemCoreClock)
		#define configTICK_RATE_HZ          ((TickType_t)1000) -> ((TickType_t)SYSTEM_TICK_HZ)

		- Disable not support features for the CortexM0 
		#define configENABLE_FPU           	1 -> 0
		#define configENABLE_TRUSTZONE     	1 -> 0

		- Add configure for the 'WFI' in idle status
		#define configUSE_TICKLESS_IDLE   	1

--------------------------------------------------------------------------------
4. Porting CMSIS to FreeRTOS
		
	- https://github.com/ARM-software/CMSIS_5.git			: CMSIS_5 5.7.0
	- https://github.com/ARM-software/CMSIS-FreeRTOS.git	: FreeRTOS V10.4.1
--------------------------------------------------------------------------------

	A. CMSIS_5 source : new CMSIS
		: Get CMSIS_5's header files

		- https://github.com/ARM-software/CMSIS_5.git
		- tag : git checkout -b 5.7.0
		------------------------------------------------------------------------
		CMSIS_5 5.7.0 <--- tag 5.7.0 
		------------------------------------------------------------------------

			- RTOS2/Include/cmsis_os2.h
			- RTOS2/Include/os_tick.h

	B. CMSIS + FreeRTOS
		- https://github.com/ARM-software/CMSIS-FreeRTOS.git
		- tag : $> git checkout develop 
		------------------------------------------------------------------------
		FreeRTOS V10.4.1 <--- branch develop
		------------------------------------------------------------------------

		a. Header Files
			[CMSIS/RTOS2/FreeRTOS]
				Include1/cmsis_os.h
				Include/freertos_mpool.h
				Include/freertos_os2.h
		
		b. Source Files
			[CMSIS/RTOS2/FreeRTOS]
				Source/cmsis_os1.c
				Source/cmsis_os2.c
				Source/os_systick.c

--------------------------------------------------------------------------------
4. Porting arm-cmsis-rtos-validator 
	
	- https://github.com/xpacks/arm-cmsis-rtos-validator.git
--------------------------------------------------------------------------------

	Get FreeRTOS-Kernel
		$> https://github.com/xpacks/arm-cmsis-rtos-validator.git
		$> git checkout -b xapck remotes/origin/xpack 


	A. Source Files
		Source/cmsis_rv.c
		Source/RV_Framework.c
		Source/RV_GenWait.c
		Source/RV_MailQueue.c	<--- #define osFeature_MailQ 1 (cmsis_os.h)
		Source/RV_MemoryPool.c	<--- #define osFeature_Pool  1 (cmsis_os.h)
		Source/RV_MsgQueue.c
		Source/RV_Mutex.c
		Source/RV_Report.c
		Source/RV_Semaphore.c
		Source/RV_Signal.c
		Source/RV_Thread.c
		Source/RV_Timer.c
		Source/RV_WaitFunc.c

	B. Header Files
		Source/config/RV_Config.h
		Include/cmsis_rv.h
		Include/RV_Framework.h
		Include/RV_Report.h
		Include/RV_Typedefs.h
		cmsis-plus/include/RTE_Components.h	<--- testsuite enable
	
	C. Porting

		[RV_WaitFunc.f]
		#define osKernelSysTickFrequency	48000000

	D. Set source build macros to each source files
		- RV_Config.h
			#include "RTE_Components.h"	<--- ADD

		- RTE_Components.h

		ex. #ifdef RTE_RV_THREAD
