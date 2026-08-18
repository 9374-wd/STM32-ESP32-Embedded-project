#ifndef __INT_IP5305T_H__
#define __INT_IP5305T_H__
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"   
/**
 * 启动IP5305T电源管理芯片，避免自动关机
 */
void Int_IP5305T_Start(void);
#endif // __INT_IP5305T_H__ 
