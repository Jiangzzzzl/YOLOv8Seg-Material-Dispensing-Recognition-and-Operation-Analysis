#include "stm32f10x.h"
#include "Timer.h"
#include "DMA1.h"

#define WS2812B_LED_QUANTITY	30	//这个宏定义了WS2812B灯带上LED的数量，这里假设有32个LED。

extern uint32_t WS2812B_Buf[WS2812B_LED_QUANTITY];    // 0xGGRRBB

/*
这个数组用来存储每个LED的颜色信息，
每个元素是一个32位的整数，
代表一个LED的颜色，格式为0xGGRRBB，其中GG代表绿色、RR代表红色、BB代表蓝色。
*/
uint32_t WS2812B_Buf[WS2812B_LED_QUANTITY];	

/*
 这个数组用来存储控制WS2812B灯带的数据位，
 因为每个LED需要24个位，所以数组大小是24*WS2812B_LED_QUANTITY+1，
 多出的一个元素可能是用来表示数据传输结束。
*/
uint16_t WS2812B_Bit[24*WS2812B_LED_QUANTITY+1];

uint8_t WS2812B_Flag;//这个标志用来指示数据传输是否完成。


void WS2812B_IRQHandler(void);


/*
这个函数用来初始化WS2812B灯带，
它调用了来设置DMA1的中断处理函数，来初始化DMA1，
传递了WS2812B_Bit数组的地址，
还调用了来初始化定时器2 （TIM2）。
*/
void WS2812B_Init(void)
{
	DMA1_SetIRQHandler(WS2812B_IRQHandler);
	DMA1_Init((uint32_t)(&WS2812B_Bit));
	TIM2_Init();
}

/*
这个函数用来清除WS2812B_Buf数组中的数据，
将所有LED的颜色都设置为黑色（0x000000）。
*/
void WS2812B_ClearBuf(void)
{
	uint8_t i;
	for(i=0;i<WS2812B_LED_QUANTITY;i++)
	{
		WS2812B_Buf[i]=0x000000;
	}
}


/*
 这个函数用来将所有LED的颜色设置为给定的颜色。
它接受一个32位的颜色值作为参数，
然后将WS2812B_Buf数组中的所有元素都设置为这个颜色。
*/
void WS2812B_SetBuf(uint32_t Color)
{
	uint8_t i;
	for(i=0;i<WS2812B_LED_QUANTITY;i++)
	{
		WS2812B_Buf[i]=Color;
	}
}


/*
这个函数用来更新WS2812B灯带的颜色数据。
它首先根据WS2812B_Buf数组中的数据，
生成对应的控制数据并存储在WS2812B_Bit数组中。
然后启动DMA传输，
将WS2812B_Bit数组中的数据传输到WS2812B灯带。
最后启动定时器2来控制数据传输的时序，并等待传输完成。
*/
void WS2812B_UpdateBuf(void)
{
	uint8_t i,j;
	for(j=0;j<WS2812B_LED_QUANTITY;j++)
	{
		for(i=0;i<24;i++)
		{
			if(WS2812B_Buf[j]&(0x800000>>i)){WS2812B_Bit[j*24+i+1]=60;}
			else{WS2812B_Bit[j*24+i+1]=30;}
		}
	}
	DMA1_Start(24*WS2812B_LED_QUANTITY+1);
	TIM2_Cmd(ENABLE);
	while(WS2812B_Flag==0);
	WS2812B_Flag=0;
}


/*
这个函数是DMA1的中断处理函数。
当DMA传输完成后，会触发DMA1的中断，
进入这个函数。在这个函数中，
首先将定时器2的比较值设置为0，
停止数据传输;然后禁用定时器2;
最后设置WS2812B_Flag标志表示数据传输完成。
*/
void WS2812B_IRQHandler(void)
{
	TIM2_SetCompare1(0);
	TIM2_Cmd(DISABLE);
	WS2812B_Flag=1;
}



/*
该函数接收一个表示颜色的十六进制数，格式为 0xRRGGBB，并返回转换后的十六进制数，格式为 0xGGRRBB
*/

unsigned int convertHexColor(unsigned int hexColor) {
    // 提取红色部分（最高8位）
    unsigned int red = (hexColor & 0xFF0000) >> 16;
    // 提取绿色部分（中间8位）
    unsigned int green = (hexColor & 0x00FF00) >> 8;
    // 提取蓝色部分（最低8位）
    unsigned int blue = hexColor & 0x0000FF;
    
    // 重新组合为 0xGGRRBB 格式
    unsigned int convertedHexColor = (green << 16) | (red << 8) | blue;
    
    return convertedHexColor;
}

