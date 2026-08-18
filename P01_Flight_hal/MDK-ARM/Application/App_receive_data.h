#ifndef _APP_RECEIVE_DATA_H_
#define _APP_RECEIVE_DATA_H_
#include "Int_SI24R1.h" 
#include "Com_config.h"
#include "Int_VL53L1X.h"
//定义帧头校验
#define FRAME_HEAD_CHECK_1 'w'
#define FRAME_HEAD_CHECK_2 'c'
#define FRAME_HEAD_CHECK_3 'y'
//最大重试次数
#define MAX_RETRY_TIMES 10

/**
 * 接收遥控器发送的遥控数据==再解析为结构体；
 * uint8_t 0:校验通过  1：未收到数据/校验失败
 */
uint8_t App_receive_data(void);
/**
 * 处理连接状态
 */
void App_process_remote_state(uint8_t res);
/**
 * 处理飞机飞行状态
 */
void App_process_flight_state(void);
#endif 

