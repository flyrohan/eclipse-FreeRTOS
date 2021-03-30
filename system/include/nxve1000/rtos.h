#ifndef _RTOS_H_
#define _RTOS_H_

#include <freertos_implement.h>

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

void HAL_RTOSInit(uint32_t ticks);
void HAL_IncTick(void);

#endif
