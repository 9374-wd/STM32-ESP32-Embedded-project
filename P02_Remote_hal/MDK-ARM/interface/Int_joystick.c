#include "Int_joystick.h"
uint16_t adc_buf[4]={0};
/**
 * 初始化ADC遥感
 */
void Int_joystick_init(void)
{
    //直接使用HAL库函数，开启ADC
    //16位数据的地址值  其实是32位，32位的地址值也是32位
    HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buf,4);
}
/**
 * 获取ADC遥感值 保存到 结构体
 * @return 
 */

void Int_joystick_get(Joystick_Struct *joystick)
{
    //DMA不依赖CPU计算  所以读取的数据实时保存在DMA
    joystick->thr=adc_buf[0];
    joystick->yaw=adc_buf[1];
    joystick->pit=adc_buf[2];
    joystick->rol=adc_buf[3];
}

