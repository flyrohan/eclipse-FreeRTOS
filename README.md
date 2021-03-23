# eclipse-FreeRTOS

Base Code
	- based on eclipse-firmware

Porting FreeRTOS with ARMDS

	1. Get source files from Arm Development Studio
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

				- freertos_user.c
				Functions:
					vApplicationGetIdleTaskMemory
					vApplicationGetTimerTaskMemory
					vApplicationStackOverflowHook

		D. main.c

			- Add main.c

Porting FreeRTOS with FreeRTOS git 
