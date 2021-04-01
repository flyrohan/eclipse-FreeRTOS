#ifndef _RTOS_H_
#define _RTOS_H_

#include <config.h>
#include <freertos_implement.h>

#ifdef CMSIS_ENABLED
#include <cmsis_os.h>
#else
typedef enum  {
  osPriorityIdle          = -3,          ///< priority: idle (lowest)
  osPriorityLow           = -2,          ///< priority: low
  osPriorityBelowNormal   = -1,          ///< priority: below normal
  osPriorityNormal        =  0,          ///< priority: normal (default)
  osPriorityAboveNormal   = +1,          ///< priority: above normal
  osPriorityHigh          = +2,          ///< priority: high
  osPriorityRealtime      = +3,          ///< priority: realtime (highest)
  osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} osPriority;
#endif

void HAL_RunRTOS(void);

#endif
