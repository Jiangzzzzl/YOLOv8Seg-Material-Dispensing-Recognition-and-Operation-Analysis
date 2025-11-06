#ifndef __TEST_H
#define __TEST_H

// 驱动板协议参数（与.c文件保持一致）
#define MOTOR_ADDR_LEFT   0x01
#define MOTOR_ADDR_RIGHT  0x02
#define MOTOR_CMD_MOVE    0x01
#define MOTOR_CHECKSUM    0x6B

// 测试参数（可在头文件中声明，供其他模块修改）
#define TEST_UNIT_COUNT  1000  

// 函数声明
void SendMotorCmd(uint8_t addr, int32_t pos);
void Delay_ms(uint32_t ms);

#endif
