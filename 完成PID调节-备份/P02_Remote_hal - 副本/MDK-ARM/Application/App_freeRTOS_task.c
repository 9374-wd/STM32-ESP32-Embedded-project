#include "App_freeRTOS_task.h"  

//STM32F103C8T6 SRAM ==>20K ==>12K给操作系统 
/********************************任务栈空间大小和优先级定义********************************** */
//定义任务优先级 数值越小--优先级越小--最大为4，最小不推荐0，系统空闲任务
//任务栈空间最小推荐值
#define POWERTASK_STACK_SIZE 128
#define POWERTASK_PRIORITY 4

#define COMM_TASK_STACK_SIZE 128
#define COMM_TASK_PRIORITY 3

#define KEY_TASK_STACK_SIZE 128
#define KEY_TASK_PRIORITY 2 

#define JOYSTICK_TASK_STACK_SIZE 128
#define JOYSTICK_TASK_PRIORITY 2
/********************************任务函数声明*********************************************** */
//电源管理任务
void power_task(void *args);
//通讯任务
void comm_task(void *args);
//按键检测任务
void key_task(void *args);
//摇杆任务
void joystick_task(void *args);
/********************************任务句柄声明*********************************************** */

TaskHandle_t power_task_handle;
TaskHandle_t comm_task_handle;
TaskHandle_t key_task_handle;
TaskHandle_t joystick_task_handle;
/********************************任务使用参数宏********************************************* */
//定义任务执行周期
#define POWER_TASK_PERIOD 10000
#define COMM_TASK_PERIOD 6
#define KEY_TASK_PERIOD 20
#define JOYSTICK_TASK_PERIOD 20
/**
 * 启动FreeRTOS任务
 */
void App_freeRTOS_start(void)
{ 
    //1.创建电源管理任务
    xTaskCreate(power_task, "power_task", POWERTASK_STACK_SIZE, NULL, POWERTASK_PRIORITY, &power_task_handle);
    //2.创建通讯任务
    xTaskCreate(comm_task, "comm_task", COMM_TASK_STACK_SIZE, NULL, COMM_TASK_PRIORITY, &comm_task_handle);
    //3.创建按键任务
    xTaskCreate(key_task, "key_task", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &key_task_handle);
    //4.创建摇杆任务
    xTaskCreate(joystick_task, "joystick_task", JOYSTICK_TASK_STACK_SIZE, NULL, JOYSTICK_TASK_PRIORITY, &joystick_task_handle);
    //5.启动调度器
    vTaskStartScheduler();
}
/**
 * 电源管理任务
 */
void power_task(void *args)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    while(1)
    {

        //每10s执行一次==>启动电源 ，避免自动关机
        vTaskDelayUntil(&lastWakeTime, POWER_TASK_PERIOD);
        //启动电源
        Int_IP5305T_Start();
    }
}
/***
 * 通讯任务
 */


void comm_task(void *args)
{
    // 获取当前的基准时间
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        App_transmit_data();

        // 6ms执行一次
        vTaskDelayUntil(&xLastWakeTime, COMM_TASK_PERIOD);
    }
}
void key_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        App_process_key_data();

        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_PERIOD);
    }
}
void joystick_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //初始化ADC遥感
    Int_joystick_init();
    while (1)
    {
        App_process_joystick_data();
        vTaskDelayUntil(&xLastWakeTime, JOYSTICK_TASK_PERIOD);
    }
}

 

