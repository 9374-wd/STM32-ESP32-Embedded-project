#include "App_flight.h" 
/****
 * 1.对于本项目使用的串级PID原理介绍：
 * (1).本项目使用两层串级PID，外环是角度环，控制飞机保持目标角度，内环是速度环，控制飞机保持目标速度。
 * (2).外环：
 * 外环的输入值：外环的输入值是误差角度，该值由测量值-目标值得到。测量值是经姿态解算得到的欧拉角，目标值是由遥控器发来的遥控数据换算得到的。
 * 外环的输出值：外环的输出值是角速度控制量，该值由PID计算得到。
 * (3).内环：
 * 内环的输入值：内环的输入值是误差速度，该值由测量值-目标值得到。测量值是经陀螺仪得到的角速度，而目标值是外环的输出值。
 * 内环的输出值：内环的输出值是具体作用电机的占空比控制量，该值由PID计算得到。
 * (4)形象解释：角度环  输入：角度差；   输出：目标角速度→ 我要飞机转多快
 *             速度环  输入：速度差；   输出：目标占空比→ 电机用多大力度
 * (5).最终作用于电机，必须是两两一组：
 * 俯仰角：前电机+后电机-
 * 横滚角：左电机-右电机+
 * 偏航角：对角线为一组 左上——右下(A1+) 左下——右上(B2-)
 * (6).串级PID的控制流程：
 * 1.先计算出外环的控制量，该值是PID计算得到的。
 * 2.再计算出内环的控制量，该值是PID计算得到的。
 * 3.最后根据外环的控制量和内环的控制量，计算出最终的控制量，该值是外环的控制量+内环的控制量。
 * 4.根据最终的控制量，控制电机转速。
 * 5.重复1-4步，直到飞机飞行。
 */
/***************************************************************************************************** */
Gyro_Accel_struct gyro_accel_data={0};
Euler_struct euler_angle={0};
Gyro_struct last_gyro={0};
float gyro_z_sum1=0;
extern Remote_data remote_data;
extern Flight_State flight_state;
//按下定高之后的飞行高度
extern uint16_t fix_height;
extern TaskHandle_t comm_task_handle;
/********************************PID 调控结构体声明********************************************** */
//pid调参是先调节内环再调节外环

//俯仰角PID结构体-->后续需要专业的PID调参
PID_Struct pitch_pid={.kp=-7.00,.ki=0.00,.kd=0.00};
//Y轴角速度结构体（对应俯仰角的内环） 
PID_Struct gyro_y_pid={.kp=3.00,.ki=0.00,.kd=0.50};

//横滚角PID结构体-->后续需要专业的PID调参
PID_Struct roll_pid={.kp=-7.00,.ki=0.00,.kd=0.00};
//X轴角速度结构体（对应横滚角的内环） 
PID_Struct gyro_x_pid={.kp=3.00,.ki=0.00,.kd=0.50};

//偏航角PID结构体--可以只通过比例部分实现.>>>>>并不在平稳飞行的范围，只需要保证飞机不在空中旋转即可->明显返回就行
PID_Struct yaw_pid={.kp=-3.00,.ki=0.00,.kd=0.00};
//Z轴角速度结构体（对应偏航角的内环） 
PID_Struct gyro_z_pid={.kp=-5.00,.ki=0.00,.kd=0.00};

//定高的PID结构体
PID_Struct height_pid={.kp=-0.60,.ki=0.00,.kd=-0.20};
//电机结构体
Motor_Struct left_top_motor={.tim=&htim3,.channel=TIM_CHANNEL_1,.speed=0};
Motor_Struct left_bottom_motor={.tim=&htim4,.channel=TIM_CHANNEL_4,.speed=0};
Motor_Struct right_top_motor={.tim=&htim2,.channel=TIM_CHANNEL_2,.speed=0};
Motor_Struct right_bottom_motor={.tim=&htim1,.channel=TIM_CHANNEL_3,.speed=0};
/***************************************************************************************************** */

/**
 * 飞控任务初始化
 * MPU6050  启动电机
 */
void App_flight_init(void)
{
    Int_MPU6050_init();

    Int_motor_Start(&left_top_motor);
    Int_motor_Start(&left_bottom_motor);
    Int_motor_Start(&right_top_motor);
    Int_motor_Start(&right_bottom_motor);

    /* 电机启动后立即将速度置零，防止上电误转 */
    left_top_motor.speed = 0;
    left_bottom_motor.speed = 0;
    right_top_motor.speed = 0;
    right_bottom_motor.speed = 0;

    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&right_bottom_motor);

    //初始化激光测距仪
    Int_VL53L1X_Init();
}
/**
 * 根据陀螺仪计算的数据得到欧拉角
 */
void App_flight_get_euler_angle(void)
{
    //1.使用MPU6050的硬件接口，得到六轴数据
    Int_MPU6050_GET_DATA(&gyro_accel_data);


    //2.对角速度进行低通滤波
    //对准确性要求没有那么高，但是要求计算迅速（实时性）
    //ouput=加权系数*last_output +(1-加权系数)*本次测量值。
   gyro_accel_data.gyro.gyro_x =Common_Filter_LowPass(gyro_accel_data.gyro.gyro_x,last_gyro.gyro_x);
   gyro_accel_data.gyro.gyro_y =Common_Filter_LowPass(gyro_accel_data.gyro.gyro_y,last_gyro.gyro_y);
   gyro_accel_data.gyro.gyro_z =Common_Filter_LowPass(gyro_accel_data.gyro.gyro_z,last_gyro.gyro_z); 

   last_gyro.gyro_x=gyro_accel_data.gyro.gyro_x;
   last_gyro.gyro_y=gyro_accel_data.gyro.gyro_y;
   last_gyro.gyro_z=gyro_accel_data.gyro.gyro_z;
    //先打印
     // debug_printf(":%d,%d,%d\n",gyro_accel_data.gyro.gyro_x,gyro_accel_data.gyro.gyro_y,gyro_accel_data.gyro.gyro_z);
     
    //3.对波动变化较大的加速度使用更强大的卡尔曼滤波
    gyro_accel_data.accel.accel_x=Common_Filter_KalmanFilter(&kfs[0],gyro_accel_data.accel.accel_x);
    gyro_accel_data.accel.accel_y=Common_Filter_KalmanFilter(&kfs[1],gyro_accel_data.accel.accel_y);
    gyro_accel_data.accel.accel_z=Common_Filter_KalmanFilter(&kfs[2],gyro_accel_data.accel.accel_z);
    //打印三轴加速度    
    //  debug_printf(":%d,%d,%d\n",gyro_accel_data.accel.accel_x,gyro_accel_data.accel.accel_y,gyro_accel_data.accel.accel_z);

    // //4.通过加速度和角速度计算飞机倾斜的角度--》姿态解算
    // //使用互补解算计算欧拉角，->优先使用加速度解算，俯仰角和横滚角，偏航角只能通过角速度积分
    // euler_angle.pitch=atan2(gyro_accel_data.accel.accel_x*1.0,gyro_accel_data.accel.accel_z)/3.14159*180;
    // euler_angle.roll=atan2(gyro_accel_data.accel.accel_y*1.0,gyro_accel_data.accel.accel_z)/3.14159*180;
    // //16位ADC转换为°/s=》量程是±2000°/s
    // gyro_z_sum1+=(gyro_accel_data.gyro.gyro_z*2000.0/32768.0)*0.006;
    // euler_angle.yaw=gyro_z_sum1;

    //也可以使用移植的四元数姿态解算
    Common_IMU_GetEulerAngle(&gyro_accel_data,&euler_angle,0.006);
    // debug_printf(":%.2f,%.2f,%.2f\n",euler_angle.pitch,euler_angle.roll,euler_angle.yaw);
}
/**
 * 根据欧拉角 计算出PID的目标值
 */
void App_flight_pid_process(void)
{
    //俯仰角
    //1.需要赋值目标值和测量值
    //外环的目标角度 =>如果是平稳飞行 值为0，如果需要遥控飞行，目标角度就是遥控器的值
    //数值转换 （0-1000  500为中间点）控制倾角范围在±10度
    pitch_pid.desire=(remote_data.pit-500)/50.0;        //遥感推到最大，传过来1000 计算后得到10°即为目标值
    pitch_pid.measure=euler_angle.pitch;        //外环的测量值=>就是当前的俯仰角

    gyro_y_pid.measure=gyro_accel_data.gyro.gyro_y*2000/32768.0;     //内环的测量值=>当前的角速度
    //2.进行PID计算
    Com_PID_Calc_Chain(&pitch_pid,&gyro_y_pid);
    // debug_printf(":%.2f,%.2f\n",gyro_y_pid.err,gyro_y_pid.output);

    //横滚角
    //1.需要赋值目标值和测量值
    //外环的目标角度 =>如果是平稳飞行 值为0，如果需要遥控飞行，目标角度就是遥控器的值
    //数值转换 （0-1000  500为中间点）控制倾角范围在±10度
    roll_pid.desire=(remote_data.rol-500)/50.0;        //遥感推到最大，传过来1000 计算后得到10°即为目标值
    roll_pid.measure=euler_angle.roll;        //外环的测量值=>就是当前的横滚角
    gyro_x_pid.measure=gyro_accel_data.gyro.gyro_x*2000/32768.0;     //内环的测量值=>当前的角速度
    //2.进行PID计算
    Com_PID_Calc_Chain(&roll_pid,&gyro_x_pid);

    //偏航角
    //1.需要赋值目标值和测量值
    yaw_pid.desire=(remote_data.yaw-500)/50.0;
    yaw_pid.measure=euler_angle.yaw; //外环的测量值==>就是当前的偏航角
    gyro_z_pid.measure=(gyro_accel_data.gyro.gyro_z*2000.0/32768.0);    //内环的测量值，当前的角速度

    //进行PID计算
    Com_PID_Calc_Chain(&yaw_pid,&gyro_z_pid);
}

/**
 * 跟踪PID输出控制电机
 */
void App_flight_control_motor(void)
{
    //1.首先判断当前飞机的飞行状态
    switch (flight_state)
    {
    case IDLE:
        //一旦进入加锁状态，设置电机的速度为0
        left_top_motor.speed=0;
        left_bottom_motor.speed=0;
        right_top_motor.speed=0;
        right_bottom_motor.speed=0;
        break;
    case NORMAL:
        //俯仰角=>向前飞有角速度=>正误差--需要向后飞的反馈效果=>前两个电机转的快，后两个电机转的慢
        // left_top_motor.speed=remote_data.thr+gyro_y_pid.output-gyro_x_pid.output+Com_limit(gyro_z_pid.output,100,-100);
        // left_bottom_motor.speed=remote_data.thr-gyro_y_pid.output-gyro_x_pid.output-Com_limit(gyro_z_pid.output,100,-100);
        // right_top_motor.speed=remote_data.thr+gyro_y_pid.output+gyro_x_pid.output-Com_limit(gyro_z_pid.output,100,-100);
        // right_bottom_motor.speed=remote_data.thr-gyro_y_pid.output+gyro_x_pid.output+Com_limit(gyro_z_pid.output,100,-100);

        left_top_motor.speed=remote_data.thr+gyro_y_pid.output-gyro_x_pid.output+(gyro_z_pid.output);
        left_bottom_motor.speed=remote_data.thr-gyro_y_pid.output-gyro_x_pid.output-(gyro_z_pid.output);
        right_top_motor.speed=remote_data.thr+gyro_y_pid.output+gyro_x_pid.output-(gyro_z_pid.output);
        right_bottom_motor.speed=remote_data.thr-gyro_y_pid.output+gyro_x_pid.output+(gyro_z_pid.output);
        break;
    case FIX_HEIGHT:
        //只有定高状态才需要定高的PID计算=>定高也需要平稳飞行
        left_top_motor.speed=remote_data.thr+gyro_y_pid.output-gyro_x_pid.output+Com_limit(gyro_z_pid.output,100,-100)+height_pid.output;
        left_bottom_motor.speed=remote_data.thr-gyro_y_pid.output-gyro_x_pid.output-Com_limit(gyro_z_pid.output,100,-100)+height_pid.output;
        right_top_motor.speed=remote_data.thr+gyro_y_pid.output+gyro_x_pid.output-Com_limit(gyro_z_pid.output,100,-100)+height_pid.output;
        right_bottom_motor.speed=remote_data.thr-gyro_y_pid.output+gyro_x_pid.output+Com_limit(gyro_z_pid.output,100,-100)+height_pid.output;

    break;
    case FAIL:
        left_bottom_motor.speed-=3;
        left_top_motor.speed-=3;
        right_bottom_motor.speed-=3;
        right_top_motor.speed-=3;
        if (left_bottom_motor.speed<=0&&left_top_motor.speed<=0&&right_bottom_motor.speed<=0&& right_top_motor.speed<=0)
        {
            flight_state=IDLE;
            //故障处理完成，电机转速降低为0；
            xTaskNotifyGive(comm_task_handle);
        }
        
    break;
    default:
        break;
    }
    //限制电机速度上限值
    left_top_motor.speed=Com_limit(left_top_motor.speed,700,0);
    left_bottom_motor.speed=Com_limit(left_bottom_motor.speed,700,0);
    right_top_motor.speed=Com_limit(right_top_motor.speed,700,0);
    right_bottom_motor.speed=Com_limit(right_bottom_motor.speed,700,0);
    //安全限制-油门设置50时，强制将速度设置为0
    if(remote_data.thr < 50)
    {
        left_top_motor.speed = 0;
        left_bottom_motor.speed = 0;
        right_top_motor.speed = 0;
        right_bottom_motor.speed = 0;
    }
    //2.设置电机转速

    Int_motor_set_speed(&left_top_motor);
    Int_motor_set_speed(&left_bottom_motor);
    Int_motor_set_speed(&right_top_motor);
    Int_motor_set_speed(&right_bottom_motor);
    
}
/**
 * 进入定高功能之后的PID计算
 */
void App_flight_fix_height_pid_process(void)
{
    //24ms一次
    //1.填写目标值（按下定高按键时飞机所处的高度）和测量值（激光测距仪得到的高度）
    height_pid.desire=fix_height;
    height_pid.measure=Int_VL53L1X_GetDistance();

    //2.进行单环PID计算的到输出值
    Com_PID_Calc(&height_pid);
}
