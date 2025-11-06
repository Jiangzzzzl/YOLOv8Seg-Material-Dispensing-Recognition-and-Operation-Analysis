#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"                  // Device header


void Timer1_Init(void);
void Timer4_Init(void);


void TIM2_Init(void);
void TIM2_Cmd(FunctionalState NewState);
void TIM2_SetCompare1(uint16_t Value);

#endif

