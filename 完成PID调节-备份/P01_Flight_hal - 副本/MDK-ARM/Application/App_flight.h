#ifndef _APP_FLIGHT_H_
#define _APP_FLIGHT_H_  

#include "Int_mpu6050.h"
#include "Com_debug.h"
#include "Com_filter.h"
#include "math.h"
#include "Com_imu.h"
#include "Com_pid.h"
#include "Int_motor.h"

/**
 * 飞控任务初始化
 * MPU6050  启动电机
 */
void App_flight_init(void);
/**
 * 根据陀螺仪计算的数据得到欧拉角
 */
void App_flight_get_euler_angle(void);

/**
 * 根据欧拉角 计算出PID的目标值
 */
void App_flight_pid_process(void);
/**
 * 跟踪PID输出控制电机
 */
void App_flight_control_motor(void);
#endif
