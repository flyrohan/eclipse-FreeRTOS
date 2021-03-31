# eclipse-FreeRTOS

Base Code
	- based on eclipse-firmware

--------------------------------------------------------------------------------
1. Base Porting FreeRTOS
--------------------------------------------------------------------------------

	1-1. Porting FreeRTOS with ARMDS

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


	1-2. Porting FreeRTOS with FreeRTOS git to firmware source

		Get FreeRTOS-LTS
			$> git clone https://github.com/FreeRTOS/FreeRTOS-LTS.git
			$> git submodule init && git submodule update

		A. header file
			$> cp -a FreeRTOS/FreeRTOS-Kernel/include kernel/ 

		B. source file
			$> cp -a FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/*.c kernel/
				croutine.c
				event_groups.c
				list.c
				queue.c
				stream_buffer.c
				tasks.c
				timers.c

			$> cp FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c kernel/
				port.c

			$> cp FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM0/portmacro.h kernel/include
				portmacro.h

			$> cp FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/portable/MemMang/heap_4.c kernel/
				heap_4.c

		C. FreeRTOS porting	

			A. kernel/include/FreeRTOSConfig.h
			- FreeRTOS configuration

			B. FreeRTOS require functions
				[freertos_implement.c]

				- vApplicationGetIdleTaskMemory
				- vApplicationGetTimerTaskMemory
				- vApplicationStackOverflowHook

					- vApplicationGetIdleTaskMemory
					- vApplicationGetTimerTaskMemory
					- vApplicationStackOverflowHook

--------------------------------------------------------------------------------
2. Porting FreeRTOS to Firmware
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
