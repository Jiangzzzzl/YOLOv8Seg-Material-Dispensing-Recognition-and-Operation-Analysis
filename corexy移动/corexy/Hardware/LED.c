#include "stm32f10x.h"                  // Device header

#define 	LED_Clock				RCC_APB2Periph_GPIOC
#define 	LED_Pin					GPIO_Pin_13
#define 	LED_Sign				GPIO_Pin_13
#define 	LED_GPIO				GPIOC


void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(LED_Clock, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = LED_Pin | LED_Sign;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_GPIO, &GPIO_InitStructure);
	
	GPIO_SetBits(LED_GPIO, LED_Pin);
	
	
}

void LED1_ON(void)
{
	GPIO_ResetBits(LED_GPIO, LED_Pin);
}


void LED_Sign_ON(void)
{
	GPIO_ResetBits(LED_GPIO, LED_Sign);
}

void LED_Sign_OFF(void)
{
	GPIO_SetBits(LED_GPIO, LED_Sign);
}

void LED1_OFF(void)
{
	GPIO_SetBits(LED_GPIO, LED_Pin);
}

void LED1_Turn(void)
{
	if (GPIO_ReadOutputDataBit(LED_GPIO, LED_Pin) == 1)
	{
		GPIO_ResetBits(LED_GPIO, LED_Pin);
	}
	else
	{
		GPIO_SetBits(LED_GPIO, LED_Pin);
	}
}

