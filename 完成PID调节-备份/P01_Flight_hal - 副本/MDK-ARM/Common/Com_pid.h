#ifndef _COM_PID_H_
#define _COM_PID_H_
#include "main.h"
#define PID_PERIOD 0.006
//PID结构体
//kp ki kd 需要在初始化时确定， 目标值，测量值 需要在计算时传递
typedef struct 
{
    float kp;   //比例项 响应速度
    float ki;   //积分项 解决稳态误差无人机控制一般不使用
    float kd;   //微分项 解决超调
    float err;  //误差值
    float desire; //目标值
    float measure; //测量值
    float last_err;  //上一次的误差值
    float integral; //积分累计
    float output; //输出值
}PID_Struct;

//单次PID计算
void Com_PID_Calc(PID_Struct *pid);
//串级PID计算
void Com_PID_Calc_Chain(PID_Struct *out_pid,PID_Struct *in_pid);
/**
 * 限制电机数值在正常范围内
 */
int16_t Com_limit(float speed,int16_t max_speed,int16_t min_speed);
#endif
