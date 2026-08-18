#ifndef _COM_CONFIG_H_
#define _COM_CONFIG_H_  
#include "main.h"
//连接状态
typedef enum
{
    REMOTE_CONNECTED=0,
    REMOTE_DISCONNECTED,
}Remote_State;
//飞行状态
typedef enum
{
    IDLE=0,
    NORMAL,
    FIX_HEIGHT,
    FAIL,
}Flight_State;
//飞行数据
typedef struct
{

    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown; //0不关机  1关机
    uint8_t fix_height;
}Remote_data;
//油门解锁状态
typedef enum 
{
    FREE=0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
}Thr_state;

//陀螺仪数据
typedef struct 
{
    int16_t gyro_x;//往右飞为正，表示横滚角
    int16_t gyro_y;//往前飞为正，表示俯仰角
    int16_t gyro_z;//往左飞为正，表示偏航角
}Gyro_struct;
typedef struct 
{
    int16_t accel_x;//往前为正
    int16_t accel_y;//往左为正
    int16_t accel_z;//朝上加速度为正
}Accel_struct;
typedef struct 
{
    Gyro_struct gyro;
    Accel_struct accel;
}Gyro_Accel_struct;
//解算得到欧拉角
typedef struct 
{
    float yaw;
    float pitch;
    float roll;
}Euler_struct;

#endif
