#ifndef __ESP8266_H__
#define __ESP8266_H__
void ESP8266_Init(void);
void ESP8266_connect_wifi(char* ssid,char* passward);
void ESP8266_connect_TCP(char* ip,char* port);
void formatString(char *input, char *output, int fixedLength);
void ESP8266_SendData(char *String);
#endif

