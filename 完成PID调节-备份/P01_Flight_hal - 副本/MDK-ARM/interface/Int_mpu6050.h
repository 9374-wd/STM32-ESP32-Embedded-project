#ifndef _INT_MPU6050_H_
#define _INT_MPU6050_H_
#include "i2c.h"
#include "Com_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
//从设备地址
#define MPU6050_ADDR 0x68

//读写地址
#define MPU6050_ADDR_READ 0xD1
#define MPU6050_ADDR_WRITE 0xD0
#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG			0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75
/**
 * 初始化MPU6050
 */
void Int_MPU6050_init(void);
/**
 * 读取三轴角速度
 */
void Int_mpu6050_Get_Gyro(Gyro_struct *gyro);
/**
 * 读取三轴加速度
 */
void Int_mpu6050_Get_Accel(Accel_struct *acc);
/**
 * 获取总体六轴数据
 */
void Int_MPU6050_GET_DATA(Gyro_Accel_struct *data);
#endif // _INT_MPU6050_H_


