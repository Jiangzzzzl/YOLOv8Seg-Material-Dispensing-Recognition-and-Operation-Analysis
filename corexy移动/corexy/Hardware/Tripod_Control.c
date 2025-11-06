#include <math.h>
#include "stm32f10x.h"                  // Device header
#include "Tripod_Control.h"


// 云台参数定义
#define DIS_X 1000.0f  // X轴参考距离(mm)
#define Y_OFFSET 200   // Y轴偏移量
#define X_STEP_ANGLE 10.0f // X轴步距角(度/步)
#define Y_STEP_ANGLE 10.0f // Y轴步距角(度/步)

   

// 云台坐标控制结构体
typedef struct {
    float angle;      // 当前角度
    int32_t coord;    // 当前坐标
    uint8_t is_running;  // 运行状态
} Tripod_Struct;

Tripod_Struct Tripod_X, Tripod_Y; // 云台X/Y轴状态

// 坐标转换函数
void Tripod_X_Coord(int x_dat) {
    float temp = (float)x_dat - 684.0f;
    temp = atan2(temp, DIS_X) * 57.29577f; // 弧度转角度
    
    // 计算总步数
    int32_t total_steps = (int32_t)((temp - Tripod_X.angle) * X_STEP_ANGLE);
    
    // 使用S曲线速度规划
    if(abs(total_steps) > 10) {  // 小距离直接运动
        Step_velocity_control(1, total_steps, MAX_SPEED, ACCEL);
    } else {
        Step_distance_command(1, total_steps);
    }
    
    Tripod_X.angle = temp;
    Tripod_X.coord = x_dat;
    Tripod_X.is_running = 1;
}

void Tripod_Y_Coord(int y_dat) {
    float temp = (float)(Y_OFFSET - y_dat);
    float x_len = hypot((Tripod_X.coord - 684), DIS_X);
    temp = atan2(temp, x_len) * 57.29577f;
    
    // 计算总步数
    int32_t total_steps = (int32_t)((temp - Tripod_Y.angle) * Y_STEP_ANGLE);
    
    // 使用S曲线速度规划
    if(abs(total_steps) > 10) {  // 小距离直接运动
        Step_velocity_control(2, total_steps, MAX_SPEED, ACCEL);
    } else {
        Step_distance_command(2, total_steps);
    }
    
    Tripod_Y.angle = temp;
    Tripod_Y.coord = y_dat;
    Tripod_Y.is_running = 1;
}

// 等待运动完成函数
void Tripod_Wait(uint8_t axis) {
    if(axis == 1) {
        while(Tripod_X.is_running); // 实际应用中需要添加超时检测
    } else {
        while(Tripod_Y.is_running); // 实际应用中需要添加超时检测
    }
}

// 优化后的圆形轨迹函数
void Tripod_Draw_Circle(uint16_t X_coord, uint16_t Y_coord, uint16_t radius) {
    // 使用更精确的三角函数计算
    for(int angle = 0; angle < 360; angle += 5) {  // 5度间隔
        float rad = angle * 3.1415926f / 180.0f;
        int x = X_coord + radius * cos(rad);
        int y = Y_coord + radius * sin(rad);
        
        Tripod_X_Coord(x);
        Tripod_Y_Coord(y);
        Tripod_Wait(1);
        Tripod_Wait(2);
    }
}

// 初始化函数
void Tripod_Init(void) 
{
    Tripod_X.angle = 0.0f;
    Tripod_X.coord = 684;
    Tripod_X.is_running = 0;
    
    Tripod_Y.angle = 0.0f;
    Tripod_Y.coord = Y_OFFSET;
    Tripod_Y.is_running = 0;
    
    // 这里可以添加Modbus初始化代码
}

// 示例Modbus控制函数 (需要根据实际Modbus库实现)

