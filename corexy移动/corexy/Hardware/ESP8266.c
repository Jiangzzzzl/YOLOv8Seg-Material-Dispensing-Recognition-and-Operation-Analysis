#include "stm32f10x.h"                  // Device header
#include "ESP8266.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"
#include "string.h"
char* data;

char my_str[200];



_Bool ESP8266_SendCmd(char *cmd, char *res, u16 time)
{	
	Serial_SendString(cmd);
	while(time--)
	{	
		if(Serial_RxFlag == 1)							//如果收到数据
		{	
			Serial_RxFlag = 0;	
			if(strstr((const char *)Serial_RxPacket, res)!= NULL)		//如果检索到关键词
			{
				
				return 0;
			}
		}
		Delay_ms(1);
	}
	return 1;
}


void ESP8266_Init(void)
{
	Serial_Init();
	Serial_SendString("+++");
	Delay_ms(30);
	Serial_SendString("AT+RST\r\n");
	

	Delay_ms(500);
	while(ESP8266_SendCmd("ATE0\r\n", "OK", 200))        
		Delay_ms(50);
}


void ESP8266_connect_wifi(char* ssid,char* passward)
{
	
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))        
		Delay_ms(50);
	
	sprintf(my_str,"AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,passward);
	
	while(ESP8266_SendCmd(my_str, "OK", 300))         
		Delay_ms(100);
}

void ESP8266_connect_TCP(char* ip,char* port)
{
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK", 200))
		Delay_ms(50);
	
	sprintf(my_str,"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",ip,port);
	
	while(ESP8266_SendCmd(my_str, "CONNECT", 2000))  
	{
		Delay_ms(50);
	}
	Delay_ms(100);
	
	Serial_SendString("AT+CIPSEND\r\n");

}

void ESP8266_SendData(char *String)
{
	Serial_SendString(String);
}

