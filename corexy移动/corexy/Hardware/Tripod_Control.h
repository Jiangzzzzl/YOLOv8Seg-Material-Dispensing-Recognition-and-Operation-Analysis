#ifndef __TRIPOD_CONTROL_H__
#define __TRIPOD_CONTROL_H__


#include "Step.h" 
#include <math.h>

// 运动控制参数
#define MAX_SPEED 1000  // 最大速度(步/秒)
#define ACCEL 500      // 加速度(步/秒²)


// 坐标转换函数
void Tripod_X_Coord(int x_dat);

void Tripod_Y_Coord(int y_dat);
void Tripod_Wait(uint8_t axis);
void Tripod_Draw_Circle(uint16_t X_coord, uint16_t Y_coord, uint16_t radius);

// 初始化函数
void Tripod_Init(void);




#endif
