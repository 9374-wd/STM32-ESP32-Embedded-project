#include "App_freeRTOS_task.h"  

//STM32F103C8T6 SRAM ==>20K ==>12K给操作系统 
//内存管理  --->C语言中的结构体通常保存在堆中，不会自动垃圾回收====>始终使用同一个结构体，不断循环使用

//LED结构体
LED_Struct left_top_led={.port=LED1_GPIO_Port,.pin=LED1_Pin};
LED_Struct right_top_led={.port=LED2_GPIO_Port,.pin=LED2_Pin};
LED_Struct right_bottom_led={.port=LED3_GPIO_Port,.pin=LED3_Pin};
LED_Struct left_bottom_led={.port=LED4_GPIO_Port,.pin=LED4_Pin};
//表示当前连接状态
Remote_State remote_state=REMOTE_DISCONNECTED;
//表示当前飞行模式
Flight_State flight_state=IDLE;
//扩展获取接收的遥控数据
Remote_data remote_data={.thr=0,.yaw=500,.pit=500,.rol=500,.fix_height=0,.shutdown=0};
//按下定高的时候fix的高度
uint16_t fix_height=0;
/********************************任务栈空间大小和优先级定义********************************** */
//定义任务优先级 数值越小--优先级越小--最大为4，最小不推荐0，系统空闲任务
//任务栈空间最小推荐值
#define POWER_TASK_STACK_SIZE 128
#define POWER_TASK_PRIORITY 4

#define FLIGHT_TASK_STACK_SIZE 128
#define FLIGHT_TASK_PRIORITY 3

#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1

#define COMM_TASK_STACK_SIZE 128
#define COMM_TASK_PRIORITY 2
/********************************任务使用参数宏********************************************* */

//定义任务执行周期
#define POWER_TASK_PERIOD 10000
#define FLIGHT_TASK_PERIOD 6
#define LED_TASK_PERIOD 100
#define COMM_TASK_PERIOD 10
/********************************任务函数声明*********************************************** */

void power_task(void *args);//电源管理任务
void flight_task(void *args);//飞行任务
void led_task(void *args);//led任务
void comm_task(void *args);//通讯任务
/********************************任务句柄声明*********************************************** */
TaskHandle_t power_task_handle;
TaskHandle_t flight_task_handle;
TaskHandle_t led_task_handle;
TaskHandle_t comm_task_handle;

/**
 * 启动FreeRTOS任务
 */
void App_freeRTOS_start(void)
{ 
    //1.创建电源管理任务
    xTaskCreate(power_task, "power_task", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);
    //2.创建飞行控制任务
    xTaskCreate(flight_task,"flight_task",FLIGHT_TASK_STACK_SIZE,NULL,FLIGHT_TASK_PRIORITY,&flight_task_handle);
    //3.创建LED灯控任务
    xTaskCreate(led_task,"led_task",LED_TASK_STACK_SIZE,NULL,LED_TASK_PRIORITY,&led_task_handle);
    //4.创建通讯任务
    xTaskCreate(comm_task, "comm_task", COMM_TASK_STACK_SIZE, NULL, COMM_TASK_PRIORITY, &comm_task_handle);
    //5.启动调度器
    vTaskStartScheduler();
}
void power_task(void *args)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    while(1)
    {

        // //每10s执行一次==>启动电源 ，避免自动关机
        // vTaskDelayUntil(&lastWakeTime, POWER_TASK_PERIOD);
        // //启动电源
        // Int_IP5305T_Start();
        //使用直接任务通知的接收方式 10s处理一次
        //一直等任务通知，直到收到通知res=1 或者res=0超时
       uint32_t res= ulTaskNotifyTake(pdTRUE,POWER_TASK_PERIOD);
       if (res!=0)
       {
            //收到通知
            Int_IP5305T_shutdown();
       }
       else
       {
            //不需要关机，正常执行启动
            Int_IP5305T_Start();
       }  
    }
}
void flight_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //MPU6050初始化
    App_flight_init();
    uint8_t count=0;
    while(1)

    {
        //1.获取MPU6050采集数据计算得到欧拉角
        App_flight_get_euler_angle();
        //2.根据当前的欧拉角 进行PID计算控制
        App_flight_pid_process();
        //判断定高
        if (flight_state==FIX_HEIGHT)
        {
            //才需要计算PID，激光测距仪数据采集20ms一次
            count++;
            if (count>=4)
            {
                App_flight_fix_height_pid_process();
               count=0;
            }
        }
        //3.根据PID计算结果，对电机进行控制
        App_flight_control_motor();
        // //4.打印激光测距仪得到的距离值
        // uint16_t distance=Int_VL53L1X_GetDistance();
        // debug_printf("distance:%d\r\n",distance);
        vTaskDelayUntil(&xLastWakeTime, FLIGHT_TASK_PERIOD);

    }
}
void led_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t count=0;
    while(1)
    {
        count++;
        //前两个灯表示连接状态
        //1.判断当前连接状态
        if (remote_state==REMOTE_CONNECTED)
        {
            //点亮前两个灯
            Int_led_turn_on(&left_top_led);
            Int_led_turn_on(&right_top_led);
        }
        else if(remote_state==REMOTE_DISCONNECTED)
        {
            //熄灭前两个灯
            Int_led_turn_off(&left_top_led);
            Int_led_turn_off(&right_top_led);
        }
        //后两个灯表示飞行状态
        if (flight_state==IDLE)
        {
            if (count%5==0)
            { 
                //灯慢闪烁 500ms
                //循环5次，一次是函数执行周期100ms，5次共500ms
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
            
        }
        else if (flight_state==NORMAL)
        {
            //灯快闪烁 200ms
            if (count%2==0)
            {
                //循环2次，一次是函数执行周期100ms，2次共200ms
                Int_led_toggle(&left_bottom_led);
                Int_led_toggle(&right_bottom_led);
            }
            
        }
        else if(flight_state==FIX_HEIGHT)
        {
            //后两个灯常亮
            Int_led_turn_on(&left_bottom_led);
            Int_led_turn_on(&right_bottom_led);
        }
        else if(flight_state==FAIL)
        {
            //后两个灯常灭
            Int_led_turn_off(&left_bottom_led);
            Int_led_turn_off(&right_bottom_led);
        }

        //将count计数重置
        if(count==10)
        {
            count=0;
        }
        vTaskDelayUntil(&xLastWakeTime, LED_TASK_PERIOD);
    }
}

void comm_task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        //1.接收数据
       uint8_t res= App_receive_data();
       //2.根据接收数据返回值 处理飞机连接状态
       App_process_remote_state(res);
       //3.处理关机命令
        if (remote_data.shutdown==1)
        {
            //使用该方式，功能可以实现但项目结构奇怪，可以使用FREERTOS的任务通知功能
            //Int_IP5305T_shutdown();
            xTaskNotifyGive(power_task_handle);
        }
        
        //4.处理飞机的飞行状态
        App_process_flight_state();
        //执行通讯任务
        vTaskDelay(COMM_TASK_PERIOD);
    }
}
