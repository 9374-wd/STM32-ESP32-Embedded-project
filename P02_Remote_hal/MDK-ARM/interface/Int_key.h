#ifndef _INT_KEY_H_
#define _INT_KEY_H_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum
{
    KEY_NONE=0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEFT_X,
    KEY_RIGHT_X,
    KEY_RIGHT_X_LONG,
}Key_type;

/**
 * 获取当前按键是否被按下
 * @return 
 * KEY_NONE:没有按键被按下
 * KEY_UP:向上按键被按下
 * KEY_DOWN:向下按键被按下
 * KEY_LEFT:向左按键被按下
 * KEY_RIGHT:向右按键被按下
 * KEY_LEFT_X:左X按键被按下
 * KEY_RIGHT_X:右X按键被按下
 */
Key_type Int_Key_Get(void);
#endif  
