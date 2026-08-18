#ifndef _APP_FREERTOS_TASK_H_
#define _APP_FREERTOS_TASK_H_
#include "Int_IP5305T.h"    
#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_SI24R1.h"
/**
 * 启动FreeRTOS任务
 */
void App_freeRTOS_start(void);
#endif  
