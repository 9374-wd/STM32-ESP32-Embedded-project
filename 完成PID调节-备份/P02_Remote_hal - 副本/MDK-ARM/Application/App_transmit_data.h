#ifndef _APP_TRANSMIT_DATA_H_
#define _APP_TRANSMIT_DATA_H_
#include "Int_SI24R1.h"
#include "App_process_data.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
//定义帧头校验
#define FRAME_HEAD_CHECK_1 'w'
#define FRAME_HEAD_CHECK_2 'c'
#define FRAME_HEAD_CHECK_3 'y'

/**
 * 自动切换SI24R1的模式，=将采集完成的数据打包发送到飞机
 */
void App_transmit_data(void);
#endif
