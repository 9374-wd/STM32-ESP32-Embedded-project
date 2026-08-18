#include "Int_key.h"
/**
 * 获取当前按键是否被按下
 * @return 
 * KEY_NONE:没有按键被按下
 */
Key_type Int_Key_Get(void)
{
    if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin)==GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin)==GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
              return KEY_UP;
        }
      
    }
    else if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin)==GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin)==GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_DOWN;
        }
    }
    else if (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin)==GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin)==GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT;
        }
    }
    else if (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin)==GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin)==GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_RIGHT;
        }
    }
    else if (HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin)==GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin)==GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port,KEY_LEFT_X_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_LEFT_X;
        }
    }
    //右上按键，区分长短按
    else if (HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin)==GPIO_PIN_RESET)
    {
        //此时按下，==>长按为超过一秒
        TickType_t count1=xTaskGetTickCount();
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin)==GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_RIGHT_X_GPIO_Port,KEY_RIGHT_X_Pin)==GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            if(xTaskGetTickCount() - count1 > 1000)
            {
                return KEY_RIGHT_X_LONG;
            }
            else
            {
                return KEY_RIGHT_X;
            }
        }
    }
   return KEY_NONE; 
}
