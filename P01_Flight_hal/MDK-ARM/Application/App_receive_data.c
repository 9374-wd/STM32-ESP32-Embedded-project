#include "App_receive_data.h"
extern Remote_data remote_data;
uint8_t rx_buf[TX_PLOAD_WIDTH]={0};
//遥控连接状态
extern Remote_State remote_state;
//重试连接次数
uint8_t retry_count =0;
//飞行状态
extern Flight_State flight_state;
//油门解锁状态值
Thr_state thr_state=FREE;
//MAX状态的进入时间
uint32_t max_enter_time=0;
//MIN状态进入时间
uint32_t min_enter_time=0;
//按下定高之后的飞行高度
extern uint16_t fix_height;
/**
 * 接收遥控器发送的遥控数据==再解析为结构体；
 * uint8_t 0:校验通过  1：未收到数据/校验失败
 */
uint8_t App_receive_data(void)
{
    
   
    memset(rx_buf,0,TX_PLOAD_WIDTH);
    Int_SI24R1_RxPacket(rx_buf);
    if(strlen((char *)rx_buf)==0)
    {
        return 1;
    }

    //1.帧头校验
    if(rx_buf[0]!=FRAME_HEAD_CHECK_1||rx_buf[1]!=FRAME_HEAD_CHECK_2||rx_buf[2]!=FRAME_HEAD_CHECK_3)
    {
        return 1;
    }

    //2.帧尾校验
    uint32_t sum=0;
    uint32_t sum_receive=0;
    for(uint8_t i=0;i<13;i++)
    {
        sum += rx_buf[i];
    }
    sum_receive=(rx_buf[13]<<24)|(rx_buf[14]<<16)|(rx_buf[15]<<8)|rx_buf[16];
    if(sum != sum_receive)
    {
        return 1;
    }

    //3.保存数据
    remote_data.thr=(rx_buf[3]<<8|rx_buf[4]);
    remote_data.yaw=(rx_buf[5]<<8|rx_buf[6]);
    remote_data.pit=(rx_buf[7]<<8|rx_buf[8]);
    remote_data.rol=(rx_buf[9]<<8|rx_buf[10]);
    remote_data.shutdown=rx_buf[11];
    remote_data.fix_height=rx_buf[12];
    // debug_printf(":%d,%d,%d,%d,%d,%d\n",remote_data.thr,remote_data.yaw,remote_data.pit,remote_data.rol,remote_data.shutdown,remote_data.fix_height);
    return 0;
}
/**
 * 处理连接状态
 */
void App_process_remote_state(uint8_t res)
{
    if(res==0)
    {
        //接收数据成功一次 即为连接成功
        remote_state=REMOTE_CONNECTED;
        retry_count=0;
    }
    else if(res==1)
    {
        //
        retry_count++;
        if (retry_count>=MAX_RETRY_TIMES)
        {
            remote_state=REMOTE_DISCONNECTED;
            retry_count=0;
        }
        
    }
}
/**
 * 处理解锁逻辑
 * 0 解锁成功 1解锁失败
 */
static uint8_t App_process_unlock(void)
{
    //1.考虑安全问题，解锁完成最终状态油门为0
    switch (thr_state)
    {
    case FREE:
        if (remote_data.thr>=900)
        {
            thr_state=MAX;
            //FREERTOS中以ms为计数单位的时间

            max_enter_time=xTaskGetTickCount();
        }
        
        break;
    case MAX:
    if(remote_data.thr<900)
    {
        if (xTaskGetTickCount()-max_enter_time>=1000)
        {
            //油门保持最高状态超过一秒进入LEAVE_MAX
            thr_state=LEAVE_MAX;
        }
        else
        {
            //保持最高状态小于1s
            thr_state=FREE;
        }
        
    }
    break;
    case LEAVE_MAX:
        if (remote_data.thr<=100)
        {
            //油门回到0进入MIN状态
            thr_state=MIN;
            min_enter_time=xTaskGetTickCount();
        }
        
    break;
    case MIN:
        //7.每次判断当前保持了多久
        if (xTaskGetTickCount()-min_enter_time<=1000)
        {
           if (remote_data.thr>100)
           {
            //保持最低状态小于1s
            thr_state=FREE;
           }
        }
        else
        {
            //保持最低状态超过1s
            thr_state=UNLOCK;
        }
    break;
    case UNLOCK:
        
    break;
    default:
        break;
    }
    if (thr_state==UNLOCK)
    {
        return 0;
    }
    return 1;

}
/**
 * 处理飞机飞行状态
 */
void App_process_flight_state(void)
{
    //使用状态机逻辑实现
    //1.轮询判断当前所处的状态
    switch (flight_state)
    {
    case IDLE:
        if (App_process_unlock()==0)
        {
            flight_state=NORMAL;
            thr_state=FREE;
        }
        
        break;
    case NORMAL:
        //判断进入定高状态
        if (remote_data.fix_height==1)
        {
            flight_state=FIX_HEIGHT;
            remote_data.fix_height=0;

            //在该瞬间记录飞机飞行的高度
            fix_height=Int_VL53L1X_GetDistance();
        }
        //判断进入失联状态
        if (remote_state==REMOTE_DISCONNECTED)
        {
            flight_state=FAIL;
        }
        
        break;
    case FIX_HEIGHT:
        //取消定高
        if (remote_data.fix_height==1)
        {
            flight_state=NORMAL;
            remote_data.fix_height=0;
        }
        //判断进入失联状态
        if (remote_state==REMOTE_DISCONNECTED)
        {
            flight_state=FAIL;
        }
        
        break;
    case FAIL:
        //7.处理失联故障 缓慢停止电机
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        flight_state=IDLE;
        break;
    default:
        break;
    }
}
