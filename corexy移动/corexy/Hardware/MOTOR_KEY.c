#include "MOTOR_KEY.h"
#include "Delay.h"
#include "MOTOR.h"

// 全局按键状态变量，初始化为0
Key_State_TypeDef g_key_state = {0};

// 按键初始化
void Key_Init(void) {
    RCC_APB2PeriphClockCmd(KEY_RCC | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = KEY_X_PLUS_PIN | KEY_X_MINUS_PIN | KEY_Y_PLUS_PIN | KEY_Y_MINUS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(KEY_GPIO, &GPIO_InitStruct);

    // 将GPIOB的Pin0映射到EXTI_Line0
    GPIO_EXTILineConfig(KEY_EXTI_PORT, KEY_EXTI_PIN);

    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = KEY_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = KEY_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void Motor_Stop(void) {
    GPIO_ResetBits(MOTOR_GPIO, LEFT_MOTOR_STEP_PIN | RIGHT_MOTOR_STEP_PIN);
}

// 按键中断服务函数
void EXTI0_IRQHandler(void) {
    // 检查是否由我们配置的中断线触发
    if (EXTI_GetITStatus(KEY_EXTI_LINE) != RESET) {
        // 延时消抖
        Delay_ms(20);

        // 通过读取GPIO电平来判断哪个按键被按下
        if (GPIO_ReadInputDataBit(KEY_GPIO, KEY_X_PLUS_PIN) == RESET) {
            g_key_state.x_plus = 1;
        }
        if (GPIO_ReadInputDataBit(KEY_GPIO, KEY_X_MINUS_PIN) == RESET) {
            g_key_state.x_minus = 1;
        }
        if (GPIO_ReadInputDataBit(KEY_GPIO, KEY_Y_PLUS_PIN) == RESET) {
            g_key_state.y_plus = 1;
        }
        if (GPIO_ReadInputDataBit(KEY_GPIO, KEY_Y_MINUS_PIN) == RESET) {
            g_key_state.y_minus = 1;
        }
        
        // 清除中断标志位，否则会一直触发中断
        EXTI_ClearITPendingBit(KEY_EXTI_LINE);
    }
}
