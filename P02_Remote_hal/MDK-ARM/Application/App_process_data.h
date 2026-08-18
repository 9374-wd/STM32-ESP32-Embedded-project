#ifndef _APP_PROCESS_DATA_H_
#define _APP_PROCESS_DATA_H_    

#include "Int_joystick.h"
#include "Int_key.h"
#include "Com_debug.h"
#include "Com_tool.h"
typedef struct
{

    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
    uint8_t shutdown; //0不关机  1关机
    uint8_t fix_height;
}Remote_data;
/***
 * @brief 处理按键数据，如果有按键按下， 进行对应的记录
 * 
 * @param 
 */
void App_process_key_data(void);

/***
 * @brief 处理摇杆数据，根据摇杆的输入进行处理
 * 
 * @param
 */
void App_process_joystick_data(void);

#endif  
