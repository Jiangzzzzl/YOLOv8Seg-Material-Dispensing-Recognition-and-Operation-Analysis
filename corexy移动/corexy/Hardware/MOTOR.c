#include "MOTOR.h"
#include "USART.h"
#include "Delay.h"
#include <string.h>
#include <MOTOR_KEY.h>
#include <SERVO.h>

// 撒料口当前坐标（初始为左上角原点）
int32_t g_current_x = 0;
int32_t g_current_y = 0;
uint8_t timer_pulse_flag = 0;
float Angle = 0;

// 电机控制初始化
void Motor_Ctrl_Init(void) 
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);
	GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
}

// 解析Jetson发送的坐标（格式："X100Y-50"）
Coord_Result Coord_Parse_FromUsart(uint8_t *rx_buf, uint8_t rx_len) {
    Coord_Result result = {false, 0, 0};
    bool x_found = false, y_found = false;
    uint8_t i = 0;

    while (i < rx_len) {
        // 解析X坐标
        if (rx_buf[i] == 'X') {
            x_found = true;
            i++;
            while (i < rx_len && rx_buf[i] >= '0' && rx_buf[i] <= '9') {
                result.target_x = result.target_x * 10 + (rx_buf[i] - '0');
                i++;
            }
        }
        // 解析Y坐标（支持负数）
        else if (rx_buf[i] == 'Y') {
            y_found = true;
            i++;
            bool is_neg = (rx_buf[i] == '-');
            if (is_neg) i++;
            while (i < rx_len && rx_buf[i] >= '0' && rx_buf[i] <= '9') {
                result.target_y = result.target_y * 10 + (rx_buf[i] - '0');
                i++;
            }
            if (is_neg) result.target_y = -result.target_y;
        } else {
            i++;
        }
    }

    result.valid = x_found && y_found;
    return result;
}

// 设置电机方向
void Motor_SetDir(bool left_dir, bool right_dir) {
    if (left_dir) {
        GPIO_SetBits(MOTOR_GPIO, LEFT_MOTOR_DIR_PIN);
    } else {
        GPIO_ResetBits(MOTOR_GPIO, LEFT_MOTOR_DIR_PIN);
    }
    if (right_dir) {
        GPIO_SetBits(MOTOR_GPIO, RIGHT_MOTOR_DIR_PIN);
    } else {
        GPIO_ResetBits(MOTOR_GPIO, RIGHT_MOTOR_DIR_PIN);
    }
}

// 同步发送脉冲，控制两个电机转动指定步数
void Motor_SendSyncPulse(int32_t steps) {
    for (int32_t i = 0; i < steps; i++) {
//		if (g_key_state.stop) break; // 检查紧急停止标志
        // 生成左电机脉冲
        GPIO_SetBits(MOTOR_GPIO, LEFT_MOTOR_STEP_PIN);
        Delay_us(PULSE_HIGH_TIME_US);
        GPIO_ResetBits(MOTOR_GPIO, LEFT_MOTOR_STEP_PIN);
        Delay_us(PULSE_LOW_TIME_US);

        // 生成右电机脉冲（与左电机同步）
        GPIO_SetBits(MOTOR_GPIO, RIGHT_MOTOR_STEP_PIN);
        Delay_us(PULSE_HIGH_TIME_US);
        GPIO_ResetBits(MOTOR_GPIO, RIGHT_MOTOR_STEP_PIN);
        Delay_us(PULSE_LOW_TIME_US);
    }
}

// 脉冲模式：移动到目标坐标
void Motor_MoveTo_Pulse(int32_t target_x, int32_t target_y) {
    // 1. 计算X、Y轴增量
    int32_t delta_x = target_x - g_current_x;
    int32_t delta_y = target_y - g_current_y;

    // 2. 约束坐标范围（示例：X≥0，Y在[-400, 400]之间）
    if (target_x < 0) target_x = 0;
    if (target_y > 400) target_y = 400;
    if (target_y < -400) target_y = -400;
	
	// 步骤3：增量转换为驱动板单位
    int32_t x_drive = delta_x * X_UNIT;
    int32_t y_drive = delta_y * Y_UNIT;

    // 3. 发送X轴脉冲（方向由delta_x正负决定）
    if (delta_x > 0) //往右移动，都逆时针转
	{
		Motor_SetDir(0, 0); // 0 表示逆时针方向
        Motor_SendSyncPulse(x_drive);
//        Motor_SendPulse_X(delta_x, 1); // 正方向
    } else if (delta_x < 0) //往左移动，都顺时针转
	{
		Motor_SetDir(1, 1); // 1 表示顺时针方向
        Motor_SendSyncPulse(-x_drive);
//        Motor_SendPulse_X(-delta_x, 0); // 反方向
    }

	Delay_ms(500);
	
    // 4. 发送Y轴脉冲（方向由delta_y正负决定）
    if (delta_y > 0) //往后移动，左逆右顺
	{
		Motor_SetDir(0, 1); // 0 表示逆时针方向
        Motor_SendSyncPulse(y_drive);
//        Motor_SendPulse_Y(delta_y, 1); // 正方向
    } else if (delta_y < 0) //往前移动，左顺右逆
	{
		Motor_SetDir(1, 0); // 0 表示逆时针方向
        Motor_SendSyncPulse(-y_drive);
//        Motor_SendPulse_Y(-delta_y, 0); // 反方向
    }

    // 5. 更新当前坐标
    g_current_x = target_x;
    g_current_y = target_y;
	
	Delay_ms(300);
	
	Angle=120;
	Servo_SetAngle(Angle);
	
	Delay_ms(300);
	
	Angle=30;
	Servo_SetAngle(Angle);
	
	Delay_ms(300);
	
	USART_SendString(USART1,"ok\r\n");
}
