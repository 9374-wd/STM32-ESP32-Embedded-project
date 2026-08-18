#include "Int_led.h"
/**
 * 打开LED灯
 */
void Int_led_turn_on(LED_Struct *led)
{
    //直接修改引脚电平为低电平
    HAL_GPIO_WritePin(led->port,led->pin,GPIO_PIN_RESET);
}
/**
 * 关闭LED灯
 */
void Int_led_turn_off(LED_Struct *led)
{
    //直接修改引脚电平为高电平
    HAL_GPIO_WritePin(led->port,led->pin,GPIO_PIN_SET);
}
/**
 * 翻转电平
 */
void Int_led_toggle(LED_Struct *led)
{
    HAL_GPIO_TogglePin(led->port,led->pin);
}
