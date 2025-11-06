#include "stm32f10x.h"

/**
 * 初始化TIM1的PWM功能
 * 引脚：PA8（TIM1_CH1）
 * 频率：50Hz（适合舵机、电机控制）
 */
void PWM_Init(void)
{
    // 1. 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);      // TIM1时钟（APB2）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);     // GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);      // 复用功能时钟

    // 2. 配置GPIO（PA8为TIM1_CH1复用推挽输出）
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;          // 预分频器：72MHz/72=1MHz
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;          // 自动重装载值：1MHz/20000=50Hz
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;        // 重复计数器（高级定时器特有）
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 4. 配置PWM输出通道（通道1）
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);                    // 初始化默认参数
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;          // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能主通道输出
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;   // 输出极性：高电平有效
    TIM_OCInitStructure.TIM_Pulse = 1000;                      // 初始比较值（占空比5%）
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);                   // 配置通道1

    // 5. 高级定时器必须使能主输出（关键步骤）
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 6. 启动定时器
    TIM_Cmd(TIM1, ENABLE);
}

/**
 * 设置TIM1通道1的PWM占空比
 * @param compare 比较值（范围：0~19999）
 */
void PWM_SetCompare1(uint16_t compare)
{
    // 限制比较值范围，防止溢出
    if (compare > 19999) compare = 19999;
    TIM_SetCompare1(TIM1, compare);
}
