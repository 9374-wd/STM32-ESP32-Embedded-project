#ifndef __INT_JOYSTICK_H__
#define __INT_JOYSTICK_H__  

#include "adc.h"
typedef struct 
{    
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
}Joystick_Struct;

/**
 * 初始化ADC遥感
 */
void Int_joystick_init(void);
/**
 * 获取ADC遥感值 保存到 结构体
 * @return 
 */

void Int_joystick_get(Joystick_Struct *joystick);
#endif
