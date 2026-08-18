#include "Int_motor.h"
/**
 * @brief 设置电机转速
 * 传入参数为比较值（占空比）默认200
 * @param   0-1000 0-100%
 */
void Int_motor_set_speed(Motor_Struct *motor)
{
    if(motor->speed>1000)
    {
        debug_printf("motor speed is too high\r\n");
        return;
    }
    __HAL_TIM_SET_COMPARE(motor->tim,motor->channel,motor->speed);
}
/**
 * 启动电机， 传入具体电机的结构体
 */
void Int_motor_Start(Motor_Struct *motor)
{
    __HAL_TIM_SET_COMPARE(motor->tim,motor->channel,0);
    HAL_TIM_PWM_Start(motor->tim,motor->channel);
}
