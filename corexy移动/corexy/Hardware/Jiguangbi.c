#include "stm32f10x.h"                  // Device header

#define 	JIGUANGBI_Clock				RCC_APB2Periph_GPIOA
#define 	JIGUANGBI_Pin					GPIO_Pin_0
#define 	JIGUANGBI_Port				GPIOA


void Jiguangbi_Init(void)
{
	RCC_APB2PeriphClockCmd(JIGUANGBI_Clock, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = JIGUANGBI_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(JIGUANGBI_Port, &GPIO_InitStructure);
	
	GPIO_SetBits(JIGUANGBI_Port, JIGUANGBI_Pin);
	
	
}

void Jiguangbi_ON(void)
{
	GPIO_ResetBits(JIGUANGBI_Port, JIGUANGBI_Pin);
}


void Jiguangbi_OFF(void)
{
	GPIO_SetBits(JIGUANGBI_Port, JIGUANGBI_Pin);
}

void Jiguangbi_Turn(void)
{
	if (GPIO_ReadOutputDataBit(JIGUANGBI_Port, JIGUANGBI_Pin) == 1)
	{
		GPIO_ResetBits(JIGUANGBI_Port, JIGUANGBI_Pin);
	}
	else
	{
		GPIO_SetBits(JIGUANGBI_Port, JIGUANGBI_Pin);
	}
}

