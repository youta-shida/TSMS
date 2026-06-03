@echo off
setlocal
chcp 65001 > nul

where gcc > nul 2> nul
if %errorlevel%==0 (
    echo [TSMS] 使用 MinGW GCC 编译...
    gcc -std=c11 -Wall -Wextra -O2 -o tsms.exe src\main.c
    if %errorlevel% neq 0 exit /b %errorlevel%
    echo [TSMS] 编译成功：tsms.exe
    exit /b 0
)

where cl > nul 2> nul
if %errorlevel%==0 (
    echo [TSMS] 使用 Visual Studio cl 编译...
    cl /W4 /O2 /Fe:tsms.exe src\main.c
    if %errorlevel% neq 0 exit /b %errorlevel%
    echo [TSMS] 编译成功：tsms.exe
    exit /b 0
)

echo [TSMS] 未找到 gcc 或 cl。
echo 请安装 MinGW-w64，或打开 Visual Studio Developer Command Prompt 后重新运行本脚本。
exit /b 1
