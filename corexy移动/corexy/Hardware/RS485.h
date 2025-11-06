#ifndef __RS485_H
#define __RS485_H			

#include "sys.h"	 								  



/* RS4851----------UART2---------------------------- */

/* 串口相关设置，如需更换串口更改下面这几项即可 */
#define	RS485_PROT_1		GPIOA					//串口端口
#define RS485_GPIO_RCC_1	RCC_APB2Periph_GPIOA	//串口端口GPIO时钟
#define RS485_TX_Pin_1	GPIO_Pin_2				//串口TXD引脚
#define RS485_RX_Pin_1	GPIO_Pin_3				//串口RXD引脚
#define RS485_USART_RCC_1	RCC_APB1Periph_USART2	//串口功能时钟
#define RS485_USART_NUM_1	USART2					//串口通道

/* 485模式控制引脚设置 如需更换更改下面这几项即可 */
#define	RS485_MODE_PROT_1	GPIOA					//RS485模式控制端口
#define RS485_MODE_Pin_1	GPIO_Pin_1				//RS485模式控制引脚
#define RS485_MODE_RCC_1	RCC_APB2Periph_GPIOA	//RS485模式控制端口GPIO时钟
#define RS485_TX_EN_1		PAout(1)				//485模式控制.0,接收;1,发送



/* RS4852----------UART3---------------------------- */



/* 串口相关设置，如需更换串口更改下面这几项即可 */
#define	RS485_PROT_2		GPIOB					//串口端口
#define RS485_GPIO_RCC_2	RCC_APB2Periph_GPIOB	//串口端口GPIO时钟
#define RS485_TX_Pin_2	GPIO_Pin_10				//串口TXD引脚
#define RS485_RX_Pin_2	GPIO_Pin_11				//串口RXD引脚
#define RS485_USART_RCC_2	RCC_APB1Periph_USART3	//串口功能时钟
#define RS485_USART_NUM_2	USART3					//串口通道

/* 485模式控制引脚设置 如需更换更改下面这几项即可 */
#define	RS485_MODE_PROT_2	GPIOA					//RS485模式控制端口
#define RS485_MODE_Pin_2	GPIO_Pin_1				//RS485模式控制引脚
#define RS485_MODE_RCC_2	RCC_APB2Periph_GPIOA	//RS485模式控制端口GPIO时钟
#define RS485_TX_EN_2		PAout(1)				//485模式控制.0,接收;1,发送



/* 如果想串口中断接收，请不要注释以下宏定义 */
#define EN_USART2_RX 	1						//0,不接收;1,接收.
#define EN_USART3_RX 	1						//0,不接收;1,接收.

#define READ	0x03
#define WRITE	0X06




extern uint8_t Serial2_RxPacket[];
extern uint8_t Serial2_RxFlag;
extern uint8_t Rx_Modbus_data[];

extern uint8_t ModBus_RxFlag_1;

void RS485_Init_1(u32 bound);
void RS485_RW_Opr_1(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate);
_Bool ModBus_ReadCommand_1(u8 ucAddr,u16 ucReg,u16 uiDate,int32_t data);
int32_t ModBus_ReadData_1(u8 ucAddr,u16 ucReg,u16 uiDate);


void RS485_Init_2(u32 bound);
void RS485_RW_Opr_2(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate);
_Bool ModBus_ReadCommand_2(u8 ucAddr,u16 ucReg,u16 uiDate,int32_t data);
int32_t ModBus_ReadData_2(u8 ucAddr,u16 ucReg,u16 uiDate);


int32_t bytes_to_int32(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
int16_t bytes_to_int16(uint8_t byte1, uint8_t byte2);


#endif	   
















