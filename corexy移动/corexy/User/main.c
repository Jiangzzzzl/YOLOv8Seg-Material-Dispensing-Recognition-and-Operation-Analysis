#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "MOTOR.h"
#include "USART.h"
#include "SERVO.h"
#include "PWM.h"

int main(void) {
    USART_Comm_Init();
    Motor_Ctrl_Init();
	Servo_Init();

    while (1) {
        
    }
}
