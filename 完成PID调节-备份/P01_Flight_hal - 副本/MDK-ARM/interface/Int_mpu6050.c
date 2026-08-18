#include"Int_mpu6050.h"

//保存偏移量的值
int32_t acc_x_offset=0;
int32_t acc_y_offset=0;
int32_t acc_z_offset=0;

int32_t gyro_x_offset=0;
int32_t gyro_y_offset=0;
int32_t gyro_z_offset=0;

int32_t acc_x_sum=0;
int32_t acc_y_sum=0;
int32_t acc_z_sum=0;

int32_t gyro_x_sum=0;
int32_t gyro_y_sum=0;
int32_t gyro_z_sum=0;
/**
 * 写寄存器
 * @param reg 寄存器地址
 * @param data 数据
 */
void Int_MPU6050_Write_Reg(uint8_t reg,uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1,MPU6050_ADDR_WRITE,reg,I2C_MEMADD_SIZE_8BIT,&data,1,1000);

}
/**
 * 读寄存器
 * @param reg 寄存器地址
 * @param data 数据
 */
void Int_MPU6050_Read_Reg(uint8_t reg,uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c1,MPU6050_ADDR_READ,reg,I2C_MEMADD_SIZE_8BIT,data,1,1000);
}
/**
 * 在初始化MPU6050完成之后，进行零篇偏校准
 */
void Int_MPU6050_caculate_offset(void)
{
    //1.等待飞机停放平稳，标准：前后两次加速度插值小于200，连续100次
    Accel_struct current_accel={0};
    Accel_struct last_accel={0};
    uint8_t count=0;
    Int_mpu6050_Get_Accel(&last_accel);
    while(count<100)
    {
        Int_mpu6050_Get_Accel(&current_accel);
        if(abs(current_accel.accel_x-last_accel.accel_x)<300&&abs(current_accel.accel_y-last_accel.accel_y)<300&&abs(current_accel.accel_z-last_accel.accel_z)<300)
        {
            count++;
        }
        else
        {
            count=0;
        }
        last_accel=current_accel;
        vTaskDelay(6);
    }
    //2.飞机已经平稳
    Gyro_Accel_struct gyro_accel_data={0};
    for (uint8_t i = 0; i < 100; i++)
    {
        //重新读取加速度和角速度
        Int_MPU6050_GET_DATA(&gyro_accel_data);
        acc_x_sum+=(gyro_accel_data.accel.accel_x-0);
        acc_y_sum+=(gyro_accel_data.accel.accel_y-0);
        acc_z_sum+=(gyro_accel_data.accel.accel_z-16384);

        gyro_x_sum+=(gyro_accel_data.gyro.gyro_x-0);
        gyro_y_sum+=(gyro_accel_data.gyro.gyro_y-0);
        gyro_z_sum+=(gyro_accel_data.gyro.gyro_z-0);
        //每次测量数据需要添加延时，多次测量取平均才有意义
        vTaskDelay(6);
    }
    acc_x_offset=acc_x_sum/100;
    acc_y_offset=acc_y_sum/100;
    acc_z_offset=acc_z_sum/100;

    gyro_x_offset=gyro_x_sum/100;
    gyro_y_offset=gyro_y_sum/100;
    gyro_z_offset=gyro_z_sum/100;
}
/**
 * 初始化MPU6050
 */
void Int_MPU6050_init(void)
{
   //1.重启芯片，重置所有寄存器的值=>写电源管理寄存器1 =>DEVICE_RESET
   Int_MPU6050_Write_Reg(0x6B, 0x80);
   uint8_t data=0;
   //2.重置完成后0x6B寄存器的值为0x40表示当为低功耗模式
   while (data!=0x40)
   {
    Int_MPU6050_Read_Reg(0x6B,&data);
   }
   //3.唤醒MPU6050,进入到正常工作状态
   Int_MPU6050_Write_Reg(0x6B,0x00); 
   //4.选择合适的量程吗，够用的范围内选择越小越好，=》精度高
   //4.1填写角速度量程为 ±2000°/s
   Int_MPU6050_Write_Reg(0x1b,3<<3);
   //4.2填写加速度量程为±2g
   Int_MPU6050_Write_Reg(0x1C,0X00);
   //4.3关闭中断使能，因为用不到
   Int_MPU6050_Write_Reg(0x38,0x00);
   //4.4用户配置寄存器  不适应FIFO队列，不适应I2C扩展
   Int_MPU6050_Write_Reg(0x6A,0X00);
   //4.5设置采样频率，陀螺仪监控三轴加速度和三轴角速度->默认1000HZ  1ms采样一次
   //基本逻辑：采样率>后续数据的使用频率否则失真==>香农定理 采样率>2倍使用频率，设置为500hz
   //设置采样分频为2,寄存器填写2-1=1
   Int_MPU6050_Write_Reg(0x19,0x01);
   //4.6设置低通滤波的值为184Hz=>
   Int_MPU6050_Write_Reg(0x1A,1);
   //4.7配置使用的时钟源是添加PPL的
   Int_MPU6050_Write_Reg(0x6B,0X01);
   //4.8使能加速度/角速度传感器
   Int_MPU6050_Write_Reg(0x6C,0X00);
   //4.9进行零偏校准
   Int_MPU6050_caculate_offset();
}
/**
 * 读取三轴角速度
 */
void Int_mpu6050_Get_Gyro(Gyro_struct *gyro)
{
    //存储角速度的寄存器地址从0x43开始高8位在前XYZ的顺序
    uint8_t height=0;
    uint8_t low=0;
    //读取X轴角速度
    Int_MPU6050_Read_Reg(MPU6050_GYRO_XOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_GYRO_XOUT_L,&low);
    gyro->gyro_x=(height<<8|low)-gyro_x_offset;
    //读取Y轴角速度
    Int_MPU6050_Read_Reg(MPU6050_GYRO_YOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_GYRO_YOUT_L,&low);
    gyro->gyro_y=(height<<8|low)-gyro_y_offset;

    //读取Z轴角速度
    Int_MPU6050_Read_Reg(MPU6050_GYRO_ZOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_GYRO_ZOUT_L,&low);
    gyro->gyro_z=(height<<8|low)-gyro_z_offset;
}
/**
 * 读取三轴加速度
 */
void Int_mpu6050_Get_Accel(Accel_struct *acc)
{
    uint8_t height=0;
    uint8_t low=0;
    //读取X轴加速度
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_XOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_XOUT_L,&low);
    acc->accel_x=(height<<8|low)-acc_x_offset;

    //读取Y轴加速度
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_YOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_YOUT_L,&low);
    acc->accel_y=(height<<8|low)-acc_y_offset;

    //读取Z轴加速度
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_ZOUT_H,&height);
    Int_MPU6050_Read_Reg(MPU6050_ACCEL_ZOUT_L,&low);
    acc->accel_z=(height<<8|low)-acc_z_offset;
}
/**
 * 获取总体六轴数据
 */
void Int_MPU6050_GET_DATA(Gyro_Accel_struct *data)
{
    Int_mpu6050_Get_Gyro(&data->gyro);
    Int_mpu6050_Get_Accel(&data->accel);
}
