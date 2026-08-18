#ifndef _INT_MOTOR_H_
#define _INT_MOTOR_H_
#include "tim.h"
#include "Com_debug.h"
typedef struct 
{
    TIM_HandleTypeDef *tim;
    uint16_t channel;
    uint16_t speed;
}Motor_Struct;

/**
 * @brief 设置电机转速
 * 传入参数为比较值（占空比）默认200
 * @param   0-1000 0-100%
 */
void Int_motor_set_speed(Motor_Struct *motor);
/**
 * 启动电机， 传入具体电机的结构体
 */
void Int_motor_Start(Motor_Struct *motor);
#endif 
