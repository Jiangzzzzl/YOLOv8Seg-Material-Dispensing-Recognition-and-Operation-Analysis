/*************************************************************************************
文件名称：RS485.c
版    本：V1.0
日    期：2020-5-11
编    著：Eric Xie
说    明：串口相关设置
修改日志：

modbus协义基本格式说明：
	1.主机对从机写数据操作，从机接收到报文后对报文进行解析，然后执行相应的处理，同时需要向主机应答
	  为什么要应答？因为主机需要确认从机是否收到报文，然后可向从机发送其它的数据，执行其它的操作，
	  有些操作是基于上一次操作而产生的，所以需要应答，以保证系统的健壮性和稳定性。当然也可不理会
	  
	例：主机向从机发送一条报文格式如下(16进制)
		01		  06	 00 01	   00 17	 98 04
	  从机地址	功能号	数据地址	数据	CRC校验码	
	解：主机向地址为01的从机 寄存器地址为0001的地方 写入数据0017
	    从机可把这条报文原样返回作为应答，主机收到后表示写数据成功，可执行下一条操作
	
	2.主机对从机进行读数据操作，从机接收到报文后进行解析，然后根据报文的内容，把需要返回给主机的数
	  据返回给主机，返回的报文即同等于应答。
	
	例：主机向从机发送一条报文格式如下(16进制)
		 01		  03	 00 01		   00 01		 d5 ca
	  从机地址	功能号	数据地址	读取数据个数	CRC校验码
	解：假设从机寄存器组0001的地方存放的数据为aa，那么返回给主机的数据为 01 03 02 00 aa 38 3b
		主机收到后对这条报文解析或把读到的数据保存在指定的变量中即可。
		
备注说明：以上是基本通讯格式，这些数据可通过其它方式逻辑实现更多的功能，具体请自行研究

**************************************************************************************/   
#include "rs485.h"	 
#include "Delay.h"
#include "crc16.h"
#include "Serial.h"
#include "OLED.h"


uint8_t ModBus_RxFlag_1;					//定义接收数据包标志位
uint8_t ModBus_RxFlag_2;					//定义接收数据包标志位

int32_t bytes_to_int32(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) 
{
    // 按照大端字节序组合四个字节
    int32_t result = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    return result;
}

int16_t bytes_to_int16(uint8_t byte1, uint8_t byte2) 
{
    // 按照大端字节序组合两个字节
    int16_t result = (byte1 << 8) | byte2;
    return result;
}



/* RS4851----------UART2---------------------------- */

/***************************************************************************************************
 * 函数说明: 485串口初始化
 * 入口参数：u32 bound
 * 出口参数：
 * 函数功能：初始化串口2 用于485通讯 
 *			 波特率要和从机的波特率一致
 *			 具体设置可以rs485.h是查看设置 对应硬件
***************************************************************************************************/ 
void RS485_Init_1(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RS485_GPIO_RCC_1 | RS485_MODE_RCC_1, ENABLE);	//打开串口GPIO时钟 RS485控制引脚GPIO时钟 
	RCC_APB1PeriphClockCmd(RS485_USART_RCC_1,ENABLE);				//使能对应使用串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);			//复用功能时钟，如果串口引脚为复用功能即需要有这一条

	GPIO_InitStructure.GPIO_Pin   = RS485_MODE_Pin_1;				//RS485模式控制引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//翻转速度50MHz
	GPIO_Init(RS485_MODE_PROT_1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = RS485_TX_Pin_1;				//TXD引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;			//复用推挽
	GPIO_Init(RS485_PROT_1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = RS485_RX_Pin_1;				//RXD引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; 		//浮空输入
	GPIO_Init(RS485_PROT_1, &GPIO_InitStructure);  
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 			//使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 	//先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 			//从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); 							//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式

	USART_Init(RS485_USART_NUM_1, &USART_InitStructure); ; 		//初始化串口
	USART_ITConfig(RS485_USART_NUM_1, USART_IT_RXNE, ENABLE);		//开启中断
	USART_Cmd(RS485_USART_NUM_1, ENABLE);                    		//使能串口 
}

/****************************************************************************************************
 * 函数名称： void Send_Data(u8 *buf,u8 len)
 * 入口参数：u8 *buf u8 len
 * 返回  值：无
 * 功能说明：串口发送数据
 * 			 buf:发送区首地址
 *			 len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 ***************************************************************************************************/
void Send_Data_1(u8 *buf,u8 len)
{
	u8 t;
	Delay_ms(1);
	for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(RS485_USART_NUM_1, USART_FLAG_TC) == RESET);	  
		USART_SendData(RS485_USART_NUM_1,buf[t]);
	}	 

}



/****************************************************************************************************
 * 函数名称：void RS485_RW_Opr(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate)
 * 入口参数：u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate
 * 			 ucAddr：从机地址
 *			 ucCmd ：功能码 03->读	06->写
 *			 ucReg ：寄存器地址
 *			 uiDate：写操作即是发送的数据 读操作即是读取数据个数
 * 返回  值：无
 * 功能说明：485读写操作函数
 ***************************************************************************************************/   
void RS485_RW_Opr_1(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate)
{
	unsigned int crc;
	unsigned char crcl;
	unsigned char crch;	
	unsigned char ucBuf[8];
	
	ucBuf[0] = ucAddr;				/* 从机地址 */
	ucBuf[1] = ucCmd;				/* 命令 06 写 03 读 */
	ucBuf[2] = ucReg >> 8;			/* 寄存器地址高位 */
	ucBuf[3] = ucReg;				/* 寄存器地址低位 */
	
	
	ucBuf[4] = uiDate >> 8;			/* 数据高8位 */
	ucBuf[5] = uiDate;				/* 数据低8位 */
	crc      = GetCRC16(ucBuf,6);   /* 计算CRC校验值 */
	crch     = crc >> 8;    		/* crc高位 */
	crcl     = crc &  0xFF;			/* crc低位 */
	ucBuf[6] = crch;				/* 校验高8位 */
	ucBuf[7] = crcl;				/* 校验低8位 */
	
	Send_Data_1(ucBuf,8);				/* 发送数据 */
}





uint8_t Serial2_RxPacket[9];  // Modbus数据缓冲区
uint8_t Serial2_RxFlag;       // 数据接收完成标志
uint8_t Rx_Modbus_data_1[5];  // 额外缓冲区

void USART2_IRQHandler(void)
{

	static uint8_t Rx2State = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRx2Packet = 0;	//定义表示当前接收数据位置的静态变量
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		uint8_t Rx2Data = USART_ReceiveData(USART2);				//读取数据寄存器，存放在接收的数据变量

		/*使用状态机的思路，依次处理数据包的不同部分*/
		if(Rx2State == 0 &&  Serial2_RxFlag == 0)
		{
			if(Rx2Data == 1)
			{
				Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
				pRx2Packet ++;				//数据包的位置自增
				Rx2State = 1;	//状态1：接收第二个字节
			}
		}
		else if(Rx2State == 1)
		{
			Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
			pRx2Packet ++;				//数据包的位置自增
			Rx2State = 2;
		}
		
		else if(Rx2State == 2)
		{
			if(Rx2Data == 4)	//若为四字节型接收
			{
				Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
				pRx2Packet ++;				//数据包的位置自增
				Rx2State = 3;		//四字节型接收状态位
			}
			else if(Rx2Data == 2)	//若为二字节型接收
			{
				Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
				pRx2Packet ++;				//数据包的位置自增
				Rx2State = 4;		//二字节型接收状态位
			}

		}
		
		else if(Rx2State == 3)
		{
			if(pRx2Packet <= 8)//6
			{
				Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
				pRx2Packet ++;				//数据包的位置自增
			}
			else
			{
				pRx2Packet = 0;
				Rx2State = 0;
				Serial2_RxFlag = 1;
			}

		}
		else if(Rx2State == 4)
		{
			if(pRx2Packet <= 6)
			{
				Serial2_RxPacket[pRx2Packet] = Rx2Data;	//将数据存入数据包数组的指定位置
				pRx2Packet ++;				//数据包的位置自增
			}
			else
			{
				pRx2Packet = 0;
				Rx2State = 0;
				Serial2_RxFlag = 1;
			}

		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);		//清除标志位
	}											 
} 




/**
  * 函    数：ModBus协议读取命令(一位寄存器/两位寄存器皆可)
  * 参    数：	ucAddr ModBus设备地址
				ucReg  ModBus要读取的寄存器地址
				uiDate 读操作即是读取数据个数
				data	要检查的数据
  * 返 回 值：	1 	命令读取失败，超时或数据不对
				0	读取成功,寄存器值与data相等
**/
_Bool ModBus_ReadCommand_1(u8 ucAddr,u16 ucReg,u16 uiDate,int32_t data)
{
	RS485_RW_Opr_1(ucAddr, READ, ucReg,uiDate);
	uint8_t time = 200;
	while(time--)
	{	
		if(ModBus_RxFlag_1 == 4)							//如果收到数据
		{	
			int32_t result = bytes_to_int32(Rx_Modbus_data_1[0],Rx_Modbus_data_1[1],Rx_Modbus_data_1[2],Rx_Modbus_data_1[3]);
			//printf("result:%d  data:%d\r\n",result,data);
			if(result == data)		//如果检索到关键词
			{
				return 0;
			}
			
		}
		else if(ModBus_RxFlag_1 == 2)							//如果收到数据
		{	
			int32_t result = bytes_to_int16(Rx_Modbus_data_1[0],Rx_Modbus_data_1[1]);
			//printf("result:%d  data:%d\r\n",result,data);
			if(result == data)		//如果检索到关键词
			{
				return 0;
			}
		}
		Delay_ms(1);
	}
	return 1;
}

/**
  * 函    数：ModBus协议读取寄存器数据(一位寄存器/两位寄存器皆可)
  * 参    数：	ucAddr ModBus设备地址
				ucReg  ModBus要读取的寄存器地址
				uiDate 读操作即是读取数据个数
  * 返 回 值：	1 读取失败,超时
				!1	读取成功,返回值为读取值
**/
int32_t ModBus_ReadData_1(u8 ucAddr,u16 ucReg,u16 uiDate)
{
	RS485_RW_Opr_1(ucAddr, READ, ucReg,uiDate);
	uint8_t time = 200;
	while(time--)
	{	
		if(ModBus_RxFlag_1 == 4)							//如果收到数据
		{	
			int32_t result = bytes_to_int32(Rx_Modbus_data_1[0],Rx_Modbus_data_1[1],Rx_Modbus_data_1[2],Rx_Modbus_data_1[3]);
			return result;
		}
		else if(ModBus_RxFlag_1 == 2)							//如果收到数据
		{	
			int32_t result = bytes_to_int16(Rx_Modbus_data_1[0],Rx_Modbus_data_1[1]);
			return result;
		}
		Delay_ms(1);
	}
	return 1;
}












/* RS4852----------UART3---------------------------- */
/***************************************************************************************************
 * 函数说明: 485串口初始化
 * 入口参数：u32 bound
 * 出口参数：
 * 函数功能：初始化串口2 用于485通讯 
 *			 波特率要和从机的波特率一致
 *			 具体设置可以rs485.h是查看设置 对应硬件
***************************************************************************************************/ 
void RS485_Init_2(u32 bound)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RS485_GPIO_RCC_2 | RS485_MODE_RCC_2, ENABLE);	//打开串口GPIO时钟 RS485控制引脚GPIO时钟 
	RCC_APB1PeriphClockCmd(RS485_USART_RCC_2,ENABLE);				//使能对应使用串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);			//复用功能时钟，如果串口引脚为复用功能即需要有这一条

	GPIO_InitStructure.GPIO_Pin   = RS485_MODE_Pin_2;				//RS485模式控制引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//翻转速度50MHz
	GPIO_Init(RS485_MODE_PROT_2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = RS485_TX_Pin_2;				//TXD引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;			//复用推挽
	GPIO_Init(RS485_PROT_2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = RS485_RX_Pin_2;				//RXD引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; 		//浮空输入
	GPIO_Init(RS485_PROT_2, &GPIO_InitStructure);  
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; 			//使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; 	//先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; 			//从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 			//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); 							//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式

	USART_Init(RS485_USART_NUM_2, &USART_InitStructure); ; 		//初始化串口
	USART_ITConfig(RS485_USART_NUM_2, USART_IT_RXNE, ENABLE);		//开启中断
	USART_Cmd(RS485_USART_NUM_2, ENABLE);                    		//使能串口 
}



/****************************************************************************************************
 * 函数名称： void Send_Data(u8 *buf,u8 len)
 * 入口参数：u8 *buf u8 len
 * 返回  值：无
 * 功能说明：串口发送数据
 * 			 buf:发送区首地址
 *			 len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 ***************************************************************************************************/
void Send_Data_2(u8 *buf,u8 len)
{
	u8 t;
	Delay_ms(1);
	for(t=0;t<len;t++)		//循环发送数据
	{		   
		while(USART_GetFlagStatus(RS485_USART_NUM_2, USART_FLAG_TC) == RESET);	  
		USART_SendData(RS485_USART_NUM_2,buf[t]);
	}	 

}

/****************************************************************************************************
 * 函数名称：void RS485_RW_Opr(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate)
 * 入口参数：u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate
 * 			 ucAddr：从机地址
 *			 ucCmd ：功能码 03->读	06->写
 *			 ucReg ：寄存器地址
 *			 uiDate：写操作即是发送的数据 读操作即是读取数据个数
 * 返回  值：无
 * 功能说明：485读写操作函数
 ***************************************************************************************************/   
void RS485_RW_Opr_2(u8 ucAddr,u8 ucCmd,u16 ucReg,u16 uiDate)
{
	unsigned int crc;
	unsigned char crcl;
	unsigned char crch;	
	unsigned char ucBuf[8];
	
	ucBuf[0] = ucAddr;				/* 从机地址 */
	ucBuf[1] = ucCmd;				/* 命令 06 写 03 读 */
	ucBuf[2] = ucReg >> 8;			/* 寄存器地址高位 */
	ucBuf[3] = ucReg;				/* 寄存器地址低位 */
	
	
	ucBuf[4] = uiDate >> 8;			/* 数据高8位 */
	ucBuf[5] = uiDate;				/* 数据低8位 */
	crc      = GetCRC16(ucBuf,6);   /* 计算CRC校验值 */
	crch     = crc >> 8;    		/* crc高位 */
	crcl     = crc &  0xFF;			/* crc低位 */
	ucBuf[6] = crch;				/* 校验高8位 */
	ucBuf[7] = crcl;				/* 校验低8位 */
	
	Send_Data_2(ucBuf,8);				/* 发送数据 */
}



uint8_t Serial3_RxPacket[9];//7
uint8_t Serial3_RxFlag;					//定义接收数据包标志位
uint8_t Rx_Modbus_data_2[5];

void USART3_IRQHandler(void)
{

	static uint8_t Rx3State = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRx3Packet = 0;	//定义表示当前接收数据位置的静态变量
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		uint8_t Rx3Data = USART_ReceiveData(USART3);				//读取数据寄存器，存放在接收的数据变量

		/*使用状态机的思路，依次处理数据包的不同部分*/
		if(Rx3State == 0 &&  Serial3_RxFlag == 0)
		{
			if(Rx3Data == 1)
			{
				Serial3_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
				pRx3Packet ++;				//数据包的位置自增
				Rx3State = 1;	//状态1：接收第二个字节
			}
		}
		else if(Rx3State == 1)
		{
			Serial3_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
			pRx3Packet ++;				//数据包的位置自增
			Rx3State = 2;
		}
		
		else if(Rx3State == 2)
		{
			if(Rx3Data == 4)	//若为四字节型接收
			{
				Serial3_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
				pRx3Packet ++;				//数据包的位置自增
				Rx3State = 3;		//四字节型接收状态位
			}
			else if(Rx3Data == 2)	//若为二字节型接收
			{
				Serial2_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
				pRx3Packet ++;				//数据包的位置自增
				Rx3State = 4;		//二字节型接收状态位
			}

		}
		
		else if(Rx3State == 3)
		{
			if(pRx3Packet <= 8)//6
			{
				Serial3_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
				pRx3Packet ++;				//数据包的位置自增
			}
			else
			{
				pRx3Packet = 0;
				Rx3State = 0;
				Serial3_RxFlag = 1;
			}

		}
		else if(Rx3State == 4)
		{
			if(pRx3Packet <= 6)
			{
				Serial3_RxPacket[pRx3Packet] = Rx3Data;	//将数据存入数据包数组的指定位置
				pRx3Packet ++;				//数据包的位置自增
			}
			else
			{
				pRx3Packet = 0;
				Rx3State = 0;
				Serial3_RxFlag = 1;
			}

		}
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);		//清除标志位
	}											 
} 



/**
  * 函    数：ModBus协议读取命令(一位寄存器/两位寄存器皆可)
  * 参    数：	ucAddr ModBus设备地址
				ucReg  ModBus要读取的寄存器地址
				uiDate 读操作即是读取数据个数
				data	要检查的数据
  * 返 回 值：	1 	命令读取失败，超时或数据不对
				0	读取成功,寄存器值与data相等
**/
_Bool ModBus_ReadCommand_2(u8 ucAddr,u16 ucReg,u16 uiDate,int32_t data)
{
	RS485_RW_Opr_2(ucAddr, READ, ucReg,uiDate);
	uint8_t time = 200;
	while(time--)
	{	
		if(ModBus_RxFlag_2 == 4)							//如果收到数据
		{	
			int32_t result = bytes_to_int32(Rx_Modbus_data_2[0],Rx_Modbus_data_2[1],Rx_Modbus_data_2[2],Rx_Modbus_data_2[3]);
			if(result == data)		//如果检索到关键词
			{
				return 0;
			}
			
		}
		else if(ModBus_RxFlag_2 == 2)							//如果收到数据
		{	
			int32_t result = bytes_to_int16(Rx_Modbus_data_2[0],Rx_Modbus_data_2[1]);
			if(result == data)		//如果检索到关键词
			{
				return 0;
			}
		}
		Delay_ms(1);
	}
	return 1;
}

/**
  * 函    数：ModBus协议读取寄存器数据(一位寄存器/两位寄存器皆可)
  * 参    数：	ucAddr ModBus设备地址
				ucReg  ModBus要读取的寄存器地址
				uiDate 读操作即是读取数据个数
  * 返 回 值：	1 读取失败,超时
				!1	读取成功,返回值为读取值
**/
int32_t ModBus_ReadData_2(u8 ucAddr,u16 ucReg,u16 uiDate)
{
	RS485_RW_Opr_2(ucAddr, READ, ucReg,uiDate);
	uint8_t time = 50;
	while(time--)
	{	
		if(ModBus_RxFlag_2 == 4)							//如果收到数据
		{	
			int32_t result = bytes_to_int32(Rx_Modbus_data_2[0],Rx_Modbus_data_2[1],Rx_Modbus_data_2[2],Rx_Modbus_data_2[3]);
			return result;
		}
		else if(ModBus_RxFlag_2 == 2)							//如果收到数据
		{	
			int32_t result = bytes_to_int16(Rx_Modbus_data_2[0],Rx_Modbus_data_2[1]);
			return result;
		}
		Delay_ms(50);
		RS485_RW_Opr_2(ucAddr, READ, ucReg,uiDate);
	}
	return 1;
}






void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
    {
        if(Serial2_RxFlag == 1)  // 判断是否接收到报文
        {
            /*
            格式解析：
            1,3,2,0,1
            1: 从机地址
            3: 命令号
            2: 数据字节数
            0,1: 数据
            */
            // OLED显示接收到的数据（调试用）
//						OLED_Clear();
//            OLED_ShowNum(1,1,Serial2_RxPacket[0],2);  // 从机地址
//            OLED_ShowNum(1,4,Serial2_RxPacket[1],2);  // 命令号
//            OLED_ShowNum(1,7,Serial2_RxPacket[2],2);  // 数据长度
//            OLED_ShowNum(1,10,Serial2_RxPacket[3],2); // 数据1
//            OLED_ShowNum(2,1,Serial2_RxPacket[4],2);  // 数据2
//            OLED_ShowNum(2,4,Serial2_RxPacket[5],2);  // 数据3（如有）
//            OLED_ShowNum(2,7,Serial2_RxPacket[6],2);  // 数据4（如有）
//            OLED_ShowNum(2,10,a++,2);                 // 计数器（调试用）

            if(Serial2_RxPacket[2] == 2)    // 2字节数据形式
            {
                // 直接存储数据，不进行CRC校验
                Rx_Modbus_data_1[0] = Serial2_RxPacket[3];
                Rx_Modbus_data_1[1] = Serial2_RxPacket[4];
                ModBus_RxFlag_1 = 2;  // 设置为2，表示2字节数据接收成功
            }
            else if(Serial2_RxPacket[2] == 4)    // 4字节数据形式
            {
                // 直接存储数据，不进行CRC校验
                Rx_Modbus_data_1[0] = Serial2_RxPacket[3];
                Rx_Modbus_data_1[1] = Serial2_RxPacket[4];
                Rx_Modbus_data_1[2] = Serial2_RxPacket[5];
                Rx_Modbus_data_1[3] = Serial2_RxPacket[6];
                ModBus_RxFlag_1 = 4;  // 设置为4，表示4字节数据接收成功
            }

            // 清除接收标志
            Serial2_RxFlag = 0;
        }
        
        if(Serial3_RxFlag == 1)  // 判断是否接收到报文
        {
//					OLED_Clear();
//            // OLED显示接收到的数据（调试用）
//            OLED_ShowNum(1,1,Serial3_RxPacket[0],2);
//            OLED_ShowNum(1,4,Serial3_RxPacket[1],2);
//            OLED_ShowNum(1,7,Serial3_RxPacket[2],2);
//            OLED_ShowNum(1,10,Serial3_RxPacket[3],2);
//            OLED_ShowNum(2,1,Serial3_RxPacket[4],2);
//            OLED_ShowNum(2,4,Serial3_RxPacket[5],2);
//            OLED_ShowNum(2,7,Serial3_RxPacket[6],2);
//            OLED_ShowNum(2,10,a++,2);

            if(Serial3_RxPacket[2] == 2)    // 2字节数据形式
            {
                // 直接存储数据，不进行CRC校验
                Rx_Modbus_data_2[0] = Serial3_RxPacket[3];
                Rx_Modbus_data_2[1] = Serial3_RxPacket[4];
                ModBus_RxFlag_2 = 2;  // 设置为2，表示2字节数据接收成功
            }
            else if(Serial3_RxPacket[2] == 4)    // 4字节数据形式
            {
                // 直接存储数据，不进行CRC校验
                Rx_Modbus_data_2[0] = Serial3_RxPacket[3];
                Rx_Modbus_data_2[1] = Serial3_RxPacket[4];
                Rx_Modbus_data_2[2] = Serial3_RxPacket[5];
                Rx_Modbus_data_2[3] = Serial3_RxPacket[6];
                ModBus_RxFlag_2 = 4;  // 设置为4，表示4字节数据接收成功
            }

            // 清除接收标志
            Serial3_RxFlag = 0;
        }
        
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  // 清除定时器中断标志
    }
}













