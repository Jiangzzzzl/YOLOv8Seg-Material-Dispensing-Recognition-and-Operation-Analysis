@echo off
echo 测试TCP连接到 192.168.3.16:8086
echo.
echo 请确保Android设备已启动TCP服务器
echo.
echo 按任意键开始测试...
pause >nul

telnet 192.168.3.16 8086

echo.
echo 测试完成
pause