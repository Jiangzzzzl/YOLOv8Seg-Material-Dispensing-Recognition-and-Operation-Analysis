#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "RS485.h"
#include <stdio.h>
#include "LED.h"
#include "Timer.h"
#include "Serial.h"
#include "Step.h"
#include "Jiguangbi.h"
#include "OLED.h"
/*************************************************************************************

RS485_1:接上电机
RS485_2:接下电机





PID算法：
目标：激光笔照射点 = A4 纸中心坐标（由 K230 识别输出）；
反馈：当前激光笔照射点（可通过云台的位置传感器或编码器获取，即 “当前位置”）；
偏差：目标中心坐标 - 当前位置坐标（X 轴和 Y 轴各自独立计算）；
控制量：通过 PID 算法计算出步进电机需要转动到的 “目标位置”（你的需求是输出目标位置，与位置式 PID 的输出形式一致）。
因此，PID 算法能通过持续修正偏差，让激光笔逐步逼近并稳定在 A4 纸中心，尤其适合这种需要 “动态跟踪、减小稳态误差” 的场景。




**************************************************************************************/   

//全局变量，代表步进电机当前的值，避免需要去读寄存器获取(因为本身步进电机就是闭环的所以不用担心有误差)
int32_t current_out_x = 0,current_out_y = 0;




//公共变量，用于输入三十二位寄存器
u16 high,low;



// 将32位有符号整数转换为两个16位无符号整数
void convertToModbusRegisters(int32_t value, u16* highRegister, u16* lowRegister) 
{
    // 直接拆分为高16位和低16位
    *highRegister = (u16)(value >> 16);        // 高16位
    *lowRegister = (u16)(value & 0xFFFF);      // 低16位
}


// 根据测量数据计算的准确距离转换公式
float encoder_value_to_distance(int32_t encoder_value) {
    // 反转符号以修正方向
    float distance = (float)(-encoder_value) / 12800 + 10; // 添加负号
    return distance;
}





/***************************************************************************************************
 * 函数说明: 从230接收数据并转换坐标
 * 入口参数：无
 * 出口参数：无
 * 函数功能：从230接收数据并转换坐标，计算目标位置
***************************************************************************************************/ 
// 滤波后的坐标
static int filtered_x = 0, filtered_y = 0;
// 滤波系数，可根据实际情况调整（0.2~0.5较常用）
static float alpha = 0.5f;
// 动态阈值基数（建议值：屏幕尺寸的1/8~1/4）
static const int THRESHOLD_BASE = 100;
// 连续异常计数（用于动态调整阈值）
static int anomaly_count = 0;

/**
 * 改进的坐标滤波函数：带异常值过滤机制
 * @param img_x 原始X坐标
 * @param img_y 原始Y坐标
 * @param out_x 滤波后的X坐标（输出）
 * @param out_y 滤波后的Y坐标（输出）
 * @return 是否使用了滤波值（1=正常更新，0=异常跳过）
 */
int filter_coordinates(int img_x, int img_y, int* out_x, int* out_y) {
    // 初始状态直接赋值
    if (filtered_x == 0 && filtered_y == 0) {
        filtered_x = img_x;
        filtered_y = img_y;
        *out_x = filtered_x;
        *out_y = filtered_y;
        return 1;
    }

    // 1. 计算动态阈值（基础值+连续异常放大）
    int dynamic_threshold = THRESHOLD_BASE + (anomaly_count * 10);
    
    // 2. 检查坐标变化是否异常
    int dx = abs(img_x - filtered_x);
    int dy = abs(img_y - filtered_y);
    
    if (dx > dynamic_threshold || dy > dynamic_threshold) {
        // 异常处理：跳过当前帧
        anomaly_count = (anomaly_count < 10) ? anomaly_count + 1 : 10;
        *out_x = filtered_x;
        *out_y = filtered_y;
        return 0;
    }
    
    // 3. 正常数据：执行滤波
    filtered_x = (int)(alpha * img_x + (1 - alpha) * filtered_x);
    filtered_y = (int)(alpha * img_y + (1 - alpha) * filtered_y);
    
    // 4. 重置异常计数器
    anomaly_count = (anomaly_count > 0) ? anomaly_count - 1 : 0;
    
    *out_x = filtered_x;
    *out_y = filtered_y;
    return 1;
}




// PID参数 (可以根据实际情况调整)
//float kp_x = 0.18f;    // 比例系数
//float ki_x = 0.0f;    // 积分系数
//float kd_x = 0.0f;    // 微分系数

//float kp_y = 0.1f;    // 比例系数
//float ki_y = 0.0f;    // 积分系数
//float kd_y = 0.0f;    // 微分系数


float kp_x = 0.18f;    // 比例系数
float ki_x = 0.0f;    // 积分系数
float kd_x = 0.0f;    // 微分系数

float kp_y = 0.1f;    // 比例系数
float ki_y = 0.0f;    // 积分系数
float kd_y = 0.0f;    // 微分系数

// 目标坐标
//#define TARGET_X 223		老板K230参数
//#define TARGET_Y 182


//#define TARGET_X 185		//二代K230参数
//#define TARGET_Y 166

#define TARGET_X 801		//深度训练第一版K230参数
#define TARGET_Y 740

// 限制参数

#define MAX_STEP 50
// 最大步长
#define DEAD_ZONE 5    // 死区范围
#define INTEGRAL_LIMIT 100  // 积分限幅

// PID状态变量
int last_error_x = 0;
int integral_x = 0;
int last_error_y = 0;
int integral_y = 0;


// 简化版PID计算函数
int simple_pid(int target, int current, float kp, float ki, float kd, 
              int* last_error, int* integral) 
{

    // 计算误差
    int error = target - current;
    
    // 死区处理
    if (abs(error) <= DEAD_ZONE) {
        *integral = 0;  // 死区内重置积分，防止累积
        return 0;
    }
    
    // 计算各项
    int p = error * kp;
    *integral += error * ki;
    
    // 积分限幅
    if (*integral > INTEGRAL_LIMIT) *integral = INTEGRAL_LIMIT;
    else if (*integral < -INTEGRAL_LIMIT) *integral = -INTEGRAL_LIMIT;
    
    int d = (error - *last_error) * kd;
    *last_error = error;
    
    // 总输出
    int output = p + *integral + d;
    
    // 输出限幅
    if (output > MAX_STEP) output = MAX_STEP;
    else if (output < -MAX_STEP) output = -MAX_STEP;
    
    return output;
}



void Update_Target_From_K230(int img_x, int img_y) 
{
    // 1.坐标滤波
    int filtered_x, filtered_y;
	 img_y = 1080 - img_y;
    filter_coordinates(img_x, img_y, &filtered_x, &filtered_y);
    // 2.使用简化PID计算步长
    int step_x = simple_pid(TARGET_X, filtered_x, kp_x, ki_x, kd_x, &last_error_x, &integral_x);
    int step_y = simple_pid(TARGET_Y, filtered_y, kp_y, ki_y, kd_y, &last_error_y, &integral_y);
    
    
    // 4.发送指令

    
	if(step_x >= 0 && step_x <= 65000)
	{
		Step_distance_command(2, abs(step_x) , 0);
	}
	else if(step_x < 0 )
	{
		Step_distance_command(2, abs(step_x) , 1);
	}
	
	
	if(step_y >= 0)
	{
		Step_distance_command(1, abs(step_y) , 1);
		
	}
	else if(step_y < 0)
	{
		Step_distance_command(1, abs(step_y) , 0);
	}
	

}





void Question_2(void)
{
	//设置速度
	Step_set_speed(2,3000);
	
	// 下云台归零
	Step_Set_Zero(2);

	
	//上云台初始化到正确的位置
	Step_Set_Y_Zero();

	LED1_OFF();
	//开始采样，计算移动坐标
	
	// 定义变量
	int sample_count = 0; // 采样计数
	int32_t total_x = 0, total_y = 0; // 累加器
	int final_x = 0, final_y = 0; // 最终确定的坐标值
	const int SAMPLE_NUM = 5; // 采样次数，可根据需要调整

	// 多次次采样取平均，获取稳定的坐标值
	while (sample_count < SAMPLE_NUM)
	{
		// 等待接收数据
		if (Serial_RxFlag == 1)
		{
			// 验证数据包
			if (Serial_RxPacket [0] == 0x55)
			{
				
				// 正确解析X坐标（高8位和低8位组合）
				uint16_t x_coord = ((uint16_t)Serial_RxPacket[1] << 8) | Serial_RxPacket[2];
							
				// Y坐标直接取自第3个字节
				uint8_t y_coord = Serial_RxPacket[3];
				// 累加采样值
				total_x += x_coord;
				total_y += y_coord;
				sample_count++;

				LED1_Turn (); // 指示采样进度
			}
			Serial_RxFlag = 0; // 清除标志
		}
	}

	// 计算平均值作为最终坐标
	final_x = total_x / SAMPLE_NUM;
	final_y = total_y / SAMPLE_NUM;
	

	//计算最终所需输出值
	
    // 2.计算差值
    int32_t diff_x = final_x - TARGET_X;
    int32_t diff_y = final_y - TARGET_Y;
    
    // 3.计算步进值（注意方向应该相反）
    int32_t step_x = -diff_x * 25;  // 增加负号，向目标方向移动
    int32_t step_y = -diff_y * 25;
    
    
	if(step_x >= 0)
	{
		Step_distance_command(2, step_x , 1);
		
	}
	else if(step_x < 0)
	{
		Step_distance_command(2, step_x , 0);
	}
	
	
//	if(step_y >= 0)
//	{
//		Step_distance_command(1, step_y , 1);
//		
//	}
//	else if(step_x < 0)
//	{
//		Step_distance_command(1, step_y , 0);
//	}
	
    

	Delay_ms(150);

	// 打开激光笔，完成任务
	Jiguangbi_ON();
	
	return ;
}


void Question_3(uint8_t direction)
{
	//设置速度
	
	Step_set_speed(2,100000);
	
	// 下云台归零
	Step_Set_Zero(2);
	
	// 下云台初始化到正确的位置
	Step_Set_Y_Zero();
	
	
	//找X坐标的零位
	find_uv(direction);


	//Jiguangbi_ON();

	
	return ;
}



/***************************************************************************************************
 * 函数说明: 云台初始化
 * 入口参数：无
 * 出口参数：
 * 函数功能：初始化两个云台以及处理Modbus数据的TIM1定时中断
***************************************************************************************************/ 
void Step_Init(void)
{
    RS485_Init_1(115200);	
	RS485_Init_2(115200);	
    Timer1_Init();
}






/***************************************************************************************************
*  函数说明: 设置当前位置为0位
 * 入口参数：设置的云台编号
 * 出口参数：
 * 函数功能：初始化两个云台以及处理Modbus数据的TIM1定时中断
***************************************************************************************************/ 
void Step_Set_Zero(uint8_t step_num)
{
	if(step_num == 1)
	{
		//置相对位置为0	236(EC)
		convertToModbusRegisters(ZERO_NUM,&high,&low);
		RS485_RW_Opr_1(0x01, WRITE, 0XEC, high);
		RS485_RW_Opr_1(0x01, WRITE, 0XED, low);
		current_out_y = ZERO_NUM;
		
		//检查回读
//		while(ModBus_ReadCommand_1(0X01,0XEC,0X02,ZERO_NUM))
//		{
//			Delay_ms(200);
//			RS485_RW_Opr_1(0x01, WRITE, 0XEC, high);
//			RS485_RW_Opr_1(0x01, WRITE, 0XED, low);
//		}
//		
	}
	else
	{
		//置相对位置为0	236(EC)
		convertToModbusRegisters(ZERO_NUM,&high,&low);
		RS485_RW_Opr_2(0x01, WRITE, 0XEC, high);
		RS485_RW_Opr_2(0x01, WRITE, 0XED, low);
		current_out_x = ZERO_NUM;
		
				
//		//检查回读
//		while(ModBus_ReadCommand_2(0X01,0XEC,0X02,ZERO_NUM))
//		{
//			Delay_ms(200);
//			RS485_RW_Opr_2(0x01, WRITE, 0XEC, high);
//			RS485_RW_Opr_2(0x01, WRITE, 0XED, low);
//		}
	}

}


/***************************************************************************************************
*  函数说明: 寻找Y轴位置并且设置为0位
 * 入口参数：无
 * 出口参数：
 * 函数功能：寻找Y轴位置并且设置为0位
***************************************************************************************************/ 
void Step_Set_Y_Zero(void)
{
	Step_Set_Zero(1);
	Delay_ms(100);
	Step_set_speed(1,32000);
	//控制云台缓慢转动
	//Step_distance_command(1,30000,0);
	convertToModbusRegisters(30000,&high,&low);
   RS485_RW_Opr_1(0x01, WRITE, 0XEE, high);
   RS485_RW_Opr_1(0x01, WRITE, 0XEF, low);
	current_out_y = 30000;
	
	Step_Set_Zero(1);

}

/***************************************************************************************************
 * 函数说明: 定位uv纸
 * 入口参数：无
 * 出口参数：无
 * 函数功能：缓慢移动云台，高频检测串口数据，收到识别信号后立即停止
***************************************************************************************************/ 
uint8_t Find_uv_flag = 0;

void find_uv(uint8_t direction)
{
    uint8_t count = 0;  //不使用static，避免多次调用时状态残留
    int32_t move_code = 0;
    const int STEP_PER_MOVE = 60;  // 每次移动更小的步长（如10步），提高检测频率
    const int MAX_TOTAL_STEP = 40000;  // 最大总移动步数

    Step_Set_Zero(2);
    Delay_ms(100);
	 Find_uv_flag = 1;
    while(Find_uv_flag == 1)
    {
			if(direction == 1)
			{
			  // 执行小步移动（缩短单次移动时间，提高检测频率）
			  Step_distance_command(2, STEP_PER_MOVE, 1);
			  move_code += STEP_PER_MOVE;  // 累加总移动步数
			}
			else
			{
			  // 执行小步移动（缩短单次移动时间，提高检测频率）
			  Step_distance_command(2, STEP_PER_MOVE, 0);
			  move_code += STEP_PER_MOVE;  // 累加总移动步数
			}


    }
}




uint8_t Step_distance_get(float *distance)
{
	//读取编码值
	int32_t decode = ModBus_ReadData_2(0X01, 0XEE ,0X02);
	if(decode != 1)
	{
		*distance = encoder_value_to_distance(decode);
		return 1;
	}
	else
		return 0;
	
	
}



void Step_distance_command(uint8_t step_num , int32_t distance , uint8_t direction)
{
	int32_t out;
	if(direction == 1 && step_num == 1)
	{
		out = current_out_y + distance;
	}
	else if(direction == 0 && step_num == 1)
	{
		out = current_out_y - distance;
	}
	else if(direction == 1 && step_num == 2)
	{
		out = current_out_x + distance;
	}
	else if(direction == 0 && step_num == 2)
	{
		out = current_out_x - distance;
	}
	
	
	
	if(abs(distance) <= 2000)
		{
		if(step_num == 1)
		{

			convertToModbusRegisters(out,&high,&low);
			RS485_RW_Opr_1(0x01, WRITE, 0XEE, high);
			RS485_RW_Opr_1(0x01, WRITE, 0XEF, low);
			current_out_y = out;
		}

		else
		{
			convertToModbusRegisters(out,&high,&low);
			 RS485_RW_Opr_2(0x01, WRITE, 0XEE, high);
			 RS485_RW_Opr_2(0x01, WRITE, 0XEF, low);
			current_out_x = out;
		}
	}
   
}


void Step_set_speed(uint8_t step_num , int32_t speed_value)
{
	
	if(step_num == 1)
	{
		
		//最大速度	137(89) i32
		convertToModbusRegisters(speed_value,&high,&low);
		//设定目标速度
		RS485_RW_Opr_1(0x01, WRITE, 0X89, high);
		RS485_RW_Opr_1(0x01, WRITE, 0X8A, low);
		
		//检查回读
//		while(ModBus_ReadCommand_1(0X01,0X89,0X02,speed_value))
//		{
//			Delay_ms(200);
//			//设定目标速度
//			RS485_RW_Opr_1(0x01, WRITE, 0X89, high);
//			RS485_RW_Opr_1(0x01, WRITE, 0X8A, low);
//		}
		
	}
	else
	{
		//最大速度	137(89) i32
		convertToModbusRegisters(speed_value,&high,&low);
		//设定目标速度
		RS485_RW_Opr_2(0x01, WRITE, 0X89, high);
		RS485_RW_Opr_2(0x01, WRITE, 0X8A, low);
		
		//检查回读
//		while(ModBus_ReadCommand_2(0X01,0X89,0X02,speed_value))
//		{
//			Delay_ms(200);
//			//设定目标速度
//			RS485_RW_Opr_2(0x01, WRITE, 0X89, high);
//			RS485_RW_Opr_2(0x01, WRITE, 0X8A, low);
//		}
	}

}




