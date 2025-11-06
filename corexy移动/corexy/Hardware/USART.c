#include "stm32f10x.h"
#include <stdbool.h>
#include "USART.h"
#include "delay.h"
#include "MOTOR.h"
#include "stm32f10x_usart.h"

// 接收数据全局变量定义
uint8_t g_usart_rx_buf[RX_BUF_SIZE] = {0};
uint8_t g_usart_rx_len = 0;
bool g_usart_rx_flag = false;

// 串口接收缓冲区
uint8_t g_rx_buf[RX_BUF_SIZE];
uint8_t g_rx_len = 0;


// 串口初始化
void USART_Comm_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   //开启GPIO时钟

    // 2. 配置TX引脚（复用推挽输出）
    GPIO_InitStruct.GPIO_Pin = USART_TX_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART_TX_PORT, &GPIO_InitStruct);

    // 3. 配置RX引脚（浮空输入）
    GPIO_InitStruct.GPIO_Pin = USART_RX_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART_RX_PORT, &GPIO_InitStruct);

    // 4. 配置串口参数
    USART_InitStruct.USART_BaudRate = USART_BAUDRATE;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART_DEV, &USART_InitStruct);

    // 5. 配置接收中断（RXNE）和空闲中断（IDLE，用于判断一帧结束）
    USART_ITConfig(USART_DEV, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART_DEV, USART_IT_IDLE, ENABLE);

    // 6. 配置中断优先级
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;  // 与USART_DEV对应
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 7. 使能串口
    USART_Cmd(USART_DEV, ENABLE);
}

// 发送1字节数据
void USART_Comm_SendByte(uint8_t data) 
{
    USART_SendData(USART_DEV, data);
    while (USART_GetFlagStatus(USART_DEV, USART_FLAG_TXE) == RESET);  // 等待发送完成
}

// 发送多字节缓冲区
void USART_Comm_SendBuf(uint8_t *buf, uint8_t len) 
{
    for (uint8_t i = 0; i < len; i++) 
	{
        USART_Comm_SendByte(buf[i]);
    }
}

// 清空接收缓冲区
void USART_Comm_ClearRxBuf(void) 
{
    for (uint8_t i = 0; i < RX_BUF_SIZE; i++) 
	{
        g_usart_rx_buf[i] = 0;
    }
    g_usart_rx_len = 0;
    g_usart_rx_flag = false;
}

// 串口中断服务函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(USART1);
        USART_SendData(USART1, ch); // 回显接收到的字符

        if (ch == '\r' || ch == '\n') {
            g_rx_buf[g_rx_len] = '\0';
            Coord_Result coord = Coord_Parse_FromUsart(g_rx_buf, g_rx_len);
            if (coord.valid) {
                Motor_MoveTo_Pulse(coord.target_x, coord.target_y); // 解析成功后控制电机移动
            }
            g_rx_len = 0;
        } else if (g_rx_len < RX_BUF_SIZE - 1) {
            g_rx_buf[g_rx_len++] = ch;
        }
    }
}

// 串口发送字符串（辅助调试）
void USART_SendString(USART_TypeDef* USARTx, char* str) {
    while (*str) {
        USART_SendData(USARTx, *str++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    }
}

// 串口发送整数
void USART_SendInt(USART_TypeDef* USARTx, int32_t num) {
    char str[16]; // 足够存储整数的字符串
    int i = 0;
    bool is_neg = false;

    if (num == 0) {
        USART_SendData(USARTx, '0');
        return;
    }

    if (num < 0) {
        is_neg = true;
        num = -num;
    }

    while (num > 0) {
        str[i++] = num % 10 + '0';
        num /= 10;
    }

    if (is_neg) {
        str[i++] = '-';
    }

    for (int j = i - 1; j >= 0; j--) {
        USART_SendData(USARTx, str[j]);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    }
}
