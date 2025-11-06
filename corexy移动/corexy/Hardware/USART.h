#ifndef __USART_H
#define __USART_H
#include <stdbool.h>

// 串口参数配置（可根据硬件调整）
#define USART_DEV           USART1        // 所用串口设备
#define USART_BAUDRATE      115200        // 波特率（与Jetson一致）
#define USART_TX_PIN        GPIO_Pin_9    // TX引脚（PA9）
#define USART_RX_PIN        GPIO_Pin_10   // RX引脚（PA10）
#define USART_TX_PORT       GPIOA         // TX引脚端口
#define USART_RX_PORT       GPIOA         // RX引脚端口
#define RX_BUF_SIZE         32            // 接收缓冲区大小

// 接收数据全局变量（对外提供只读访问）
extern uint8_t g_usart_rx_buf[RX_BUF_SIZE];  // 接收缓冲区
extern uint8_t g_usart_rx_len;               // 接收数据长度
extern bool g_usart_rx_flag;                 // 接收完成标志（true=一帧数据接收完）

// 函数声明
void USART_Comm_Init(void);                  // 串口初始化
void USART_Comm_SendByte(uint8_t data);      // 发送1字节数据
void USART_Comm_SendBuf(uint8_t *buf, uint8_t len);  // 发送多字节缓冲区
void USART_Comm_ClearRxBuf(void);            // 清空接收缓冲区
void USART_SendString(USART_TypeDef* USARTx, char* str);
void USART_SendInt(USART_TypeDef* USARTx, int32_t num);

#endif
