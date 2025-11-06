#ifndef MOTOR_KEY_H
#define MOTOR_KEY_H

#include "stm32f10x.h"

// 将4个按键连接到GPIOB的Pin0, Pin1, Pin2, Pin3
#define KEY_GPIO       GPIOB
#define KEY_RCC        RCC_APB2Periph_GPIOB
// 所有按键共享一条中断线 (EXTI_Line0)
#define KEY_EXTI_PORT  GPIO_PortSourceGPIOB
#define KEY_EXTI_PIN   GPIO_PinSource0
#define KEY_EXTI_LINE  EXTI_Line0
#define KEY_IRQn       EXTI0_IRQn

// 定义4个按键的具体引脚
#define KEY_X_PLUS_PIN   GPIO_Pin_0 // X=0
#define KEY_X_MINUS_PIN  GPIO_Pin_1 // X=40
#define KEY_Y_PLUS_PIN   GPIO_Pin_2 // Y=0
#define KEY_Y_MINUS_PIN  GPIO_Pin_3 // Y=40

// 使用结构体来存储按键状态
typedef struct {
    uint8_t stop;      // 紧急停止标志
    uint8_t x_plus;    // X=0 按键按下标志
    uint8_t x_minus;   // X=40 按键按下标志
    uint8_t y_plus;    // Y=0 按键按下标志
    uint8_t y_minus;   // Y=40 按键按下标志
} Key_State_TypeDef;

extern Key_State_TypeDef g_key_state; // 全局按键状态

// 函数声明
void Key_Init(void);
//void Motor_SendPulse_X(int32_t steps, uint8_t direction); // 修改为独立控制X
//void Motor_SendPulse_Y(int32_t steps, uint8_t direction); // 修改为独立控制Y
void Motor_Stop(void);

#endif

