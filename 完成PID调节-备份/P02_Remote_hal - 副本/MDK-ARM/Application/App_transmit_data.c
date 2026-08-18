#include"App_transmit_data.h"
extern Remote_data remote_data;
uint8_t com_buff[TX_PLOAD_WIDTH]={0};
/**
 * 自动切换SI24R1的模式，=将采集完成的数据打包发送到飞机
 */
void App_transmit_data(void)
{
        // 调用SI24R1的接口 发送数据
        // 1. 进入TX模式
        Int_SI24R1_TX_Mode();
        // 2. 发送数据  唯一性：帧头校验指定给对应的设备 可靠性：结尾添加校验和
        //帧头校验3个字节，数据本身10个字节，帧尾校验和4字节 共17字节
        uint32_t sum=0;
        com_buff[0]=FRAME_HEAD_CHECK_1;
        com_buff[1]=FRAME_HEAD_CHECK_2;
        com_buff[2]=FRAME_HEAD_CHECK_3;

        //高八位在前 
        com_buff[3]=(remote_data.thr>>8)&0xff;
        com_buff[4]=remote_data.thr&0xff;

        com_buff[5]=(remote_data.yaw>>8)&0xff;
        com_buff[6]=remote_data.yaw&0xff;

        com_buff[7]=(remote_data.pit>>8)&0xff;
        com_buff[8]=remote_data.pit&0xff;

        com_buff[9]=(remote_data.rol>>8)&0xff;
        com_buff[10]=remote_data.rol&0xff;
        //添加临界区
        taskENTER_CRITICAL();
        com_buff[11]=remote_data.shutdown;
        remote_data.shutdown=0;
        com_buff[12]=remote_data.fix_height;
        remote_data.fix_height=0;
        taskEXIT_CRITICAL();
        for (uint8_t i = 0; i < 13; i++)
        {
            sum+=com_buff[i];
        }
        com_buff[13]=(sum>>24)&0xff;
        com_buff[14]=(sum>>16)&0xff;
        com_buff[15]=(sum>>8)&0xff;
        com_buff[16]=sum&0xff;
        debug_printf(":%d,%d,%d,%d,%d,%d\n",remote_data.thr,remote_data.yaw,remote_data.pit,remote_data.rol,remote_data.shutdown,remote_data.fix_height);
        Int_SI24R1_TxPacket(com_buff);

        // 3. 恢复到RX模式
        Int_SI24R1_RX_Mode();
}
