#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"
#include <stdbool.h>

// 引脚定义
#define LEFT_MOTOR_STEP_PIN    GPIO_Pin_6
#define LEFT_MOTOR_DIR_PIN     GPIO_Pin_7//x
#define RIGHT_MOTOR_STEP_PIN    GPIO_Pin_1
#define RIGHT_MOTOR_DIR_PIN     GPIO_Pin_2//y
#define MOTOR_GPIO    GPIOA

// 脉冲参数（可根据需求调整）
#define PULSE_HIGH_TIME_US  20   // 脉冲高电平持续时间(μs)
#define PULSE_LOW_TIME_US   50   // 脉冲低电平持续时间(μs)，总周期100μs → 10kHz频率
#define X_UNIT              984.6  // 1mm 对应驱动板的单位数
#define Y_UNIT              985.6  // 1mm 对应驱动板的单位数
#define PWM_PERIOD     99    // 定时器周期（决定频率：72MHz/(99+1) = 720kHz）
#define PWM_PULSE      50    // 占空比50%（50/100）

// 电机方向宏定义
#define MOTOR_CW 1   // 顺时针
#define MOTOR_CCW 0  // 逆时针

// 定时器相关宏
#define MOTOR_TIM TIM2
#define MOTOR_TIM_CLK RCC_APB1Periph_TIM2

// 坐标解析结果结构体
typedef struct {
    bool valid;
    int32_t target_x;
    int32_t target_y;
} Coord_Result;

// 函数声明
void Motor_Ctrl_Init(void);
void Motor_SendPulse_X(int32_t steps, int8_t direction); // X轴发脉冲
void Motor_SendPulse_Y(int32_t steps, int8_t direction); // Y轴发脉冲
void Motor_MoveTo_Pulse(int32_t target_x, int32_t target_y); // 脉冲模式移动到目标
Coord_Result Coord_Parse_FromUsart(uint8_t *rx_buf, uint8_t rx_len);
void Motor_SetDir(bool left_dir, bool right_dir);
void Motor_StartPulse(void);
void Motor_StopPulse(void);
void Timer_Init(void);

#endif
