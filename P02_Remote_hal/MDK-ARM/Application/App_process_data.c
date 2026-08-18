#include "App_process_data.h"
//遥感数据结构体
Joystick_Struct joystick={0};
Remote_data remote_data={0};
//区分遥感控制值和按键微调值
int16_t key_pit_offset=0;   //定义前为正
int16_t key_roll_offset=0;  //定义右为正
//记录遥感偏移量
int16_t thr_offset=0;
int16_t yaw_offset=0;
int16_t pit_offset=0;
int16_t rol_offset=0;
//校准摇杆函数
void App_calibrate_joystick(void)
{
    //首先清空按键的微调值
    key_pit_offset=0;
    key_roll_offset=0;
    //多次读取求平均
    int16_t thr_sum=0;
    int16_t yaw_sum=0;
    int16_t pit_sum=0;
    int16_t rol_sum=0;
    for (uint8_t i = 0; i < 10; i++)
    {
        App_process_joystick_data();
        thr_sum+=joystick.thr-0;
        yaw_sum+=joystick.yaw-500;
        pit_sum+=joystick.pit-500;
        rol_sum+=joystick.rol-500;
        vTaskDelay(10);
    }
    //零偏校准没有累加效果，会造成两次校准退回的情况
    thr_offset+=thr_sum/10;
    yaw_offset+=yaw_sum/10;
    pit_offset+=pit_sum/10;
    rol_offset+=rol_sum/10;
  
}
/***
 * @brief 处理按键数据，如果有按键按下， 进行对应的记录
 * 
 * @param 
 */
void App_process_key_data(void)
{
    //根据按键值记录，如果进行遥感的校准，将按键值清零
    Key_type key=Int_Key_Get();
    if(key==KEY_UP)
    {
        key_pit_offset+=10;
    }
    else if(key==KEY_DOWN)
    {
        key_pit_offset-=10;
    }
    else if(key==KEY_LEFT)
    {
        key_roll_offset-=10;
    }
    else if(key==KEY_RIGHT)
    {
        key_roll_offset+=10;
    }
    else if(key==KEY_LEFT_X)
    {
        remote_data.shutdown=1;
    }
    else if(key==KEY_RIGHT_X)
    {
        remote_data.fix_height=1;
    }
    else if(key==KEY_RIGHT_X_LONG)
    {
        //校准遥感  触发校准之后 THR为0， YAW PIT ROL为500
        App_calibrate_joystick();
    }
}
/***
 * @brief 处理摇杆数据，根据摇杆的输入进行处理
 * 
 * @param
 */
void App_process_joystick_data(void)
{
    taskENTER_CRITICAL();
    //1.获取摇杆数据
    Int_joystick_get(&joystick);

    //理论上，摇杆任务运行到这一行 有可能跳转到按键任务，需要临界区保护

    //2.处理范围和极性  想要使用范围0-1000，AD转换值为0-4095
    joystick.thr =1000-((joystick.thr * 1000) / 4095);
    joystick.yaw =1000-((joystick.yaw * 1000) / 4095);
    joystick.pit =1000-((joystick.pit * 1000) / 4095);
    joystick.rol =1000-((joystick.rol * 1000) / 4095);
    //处理零偏校准
    joystick.thr-=thr_offset;
    joystick.yaw-=yaw_offset;
    joystick.pit-=pit_offset;
    joystick.rol-=rol_offset;
    //处理按键微调
    joystick.pit+=key_pit_offset;
    joystick.rol+=key_roll_offset;
    //限制在0-1000之间
    joystick.thr=Com_limit(joystick.thr,0,1000);
    joystick.yaw=Com_limit(joystick.yaw,0,1000);
    joystick.pit=Com_limit(joystick.pit,0,1000);
    joystick.rol=Com_limit(joystick.rol,0,1000);
    //退出临界区
    taskEXIT_CRITICAL();

    remote_data.thr=joystick.thr;
    remote_data.yaw=joystick.yaw;
    remote_data.pit=joystick.pit;
    remote_data.rol=joystick.rol;


}
