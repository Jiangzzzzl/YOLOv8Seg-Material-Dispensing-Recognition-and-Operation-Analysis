#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "TEST.h"
#include "USART.h"

// 发送单字节数据
void USART_SendByte(uint8_t byte) {
    USART_SendData(USART1, byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

// 发送电机控制指令（地址 + 功能码 + 3字节位置 + 校验）
void SendMotorCmd(uint8_t addr, int32_t pos) {
    USART_SendByte(addr);
    USART_SendByte(MOTOR_CMD_MOVE);
    USART_SendByte((pos) & 0xFF);         // 低字节
    USART_SendByte((pos >> 8) & 0xFF);    // 中字节
    USART_SendByte((pos >> 16) & 0xFF);   // 高字节
    USART_SendByte(MOTOR_CHECKSUM);       // 校验字节
}
