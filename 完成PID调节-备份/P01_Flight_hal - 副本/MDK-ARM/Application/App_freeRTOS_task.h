#ifndef _APP_FREERTOS_TASK_H_
#define _APP_FREERTOS_TASK_H_
#include "Int_IP5305T.h"    
#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Int_motor.h"
#include "Int_led.h"
#include "Com_config.h"
#include "Int_SI24R1.h"
#include "App_receive_data.h"
#include "App_flight.h" 
#include "Int_mpu6050.h"
/**
 * 启动FreeRTOS任务
 */
void App_freeRTOS_start(void);
#endif  
