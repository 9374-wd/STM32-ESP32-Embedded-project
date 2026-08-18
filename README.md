# STM32 & ESP32 嵌入式项目集合
> 包含四轴无人机飞控、横模车、CAN‑BootLoader、ESP32宠物喂食器等嵌入式工程。

## 📁项目目录
- **P01_Flight_hal**：STM32F103 四轴无人机飞控
  - RTOS：FreeRTOS，多任务拆分（传感器采样、遥控解析、PID控制、电机输出、关机管理）
  - 算法：串级PID姿态控制，卡尔曼滤波姿态解算
  - 外设：I2C(MPU6050、VL53L1X激光测距)、SPI、PWM电机驱动
  - 逻辑：定高模式、遥控解析、任务通知实现安全关机流程
- **P02_Remote_hal**：无线遥控单元工程
- 完成PID调节‑备份：飞控调参迭代版本
- 完成双端通讯‑备份：双向通信调试工程

## 🛠开发环境
- MCU：STM32F103C8T6 / ESP32‑S3
- IDE：Keil‑MDK5、STM32CubeMX
- RTOS：FreeRTOS
- 通信：I2C、UART、CAN总线

## ✨技术要点
1. FreeRTOS多任务调度，任务通知做事件同步，控制周期20ms
2. 串级PID实现飞行器姿态闭环控制
3. CAN总线Bootloader基础开发
4. 传感器数据滤波处理，抑制噪声
5. 分层HAL驱动，业务逻辑与底层驱动解耦
