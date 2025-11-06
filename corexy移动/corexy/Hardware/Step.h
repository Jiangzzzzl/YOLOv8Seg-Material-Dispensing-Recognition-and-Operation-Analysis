#ifndef __STEP_H
#define __STEP_H			

#include "sys.h"	 								  
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "RS485.h"


#define ZERO_NUM 30000

extern int32_t Target_x;
extern int32_t Target_y;

extern uint8_t Find_uv_flag;


void Step_Set_Y_Zero(void);
void Step_Set_Zero(uint8_t step_num);
void Update_Target_From_K230(int img_x, int img_y);

// 输入一个十进制数，返回两个 u16 类型的十六进制数
void decimalToTwoHex(int decimal, u16* high, u16* low);

void convertToModbusRegisters(int32_t value, u16* highRegister, u16* lowRegister) ;

void Step_Init(void);
void find_uv(uint8_t direction);
void Question_3(uint8_t direction);
//void X_PID();


void Step_Return_zreo(void);
void Step_distance_command(uint8_t step_num , int32_t distance , uint8_t direction);

uint8_t Step_distance_get(float *distance);

void Step_velocity_control(uint8_t step_num, int32_t distance, uint16_t max_speed, uint16_t accel);

void Step_calibrate(void);
void Step_set_speed(uint8_t step_num , int32_t speed_value);

void Question_2(void);


#endif
