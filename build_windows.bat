@echo off
setlocal
chcp 936 > nul

where gcc > nul 2> nul
if %errorlevel%==0 (
    echo [TSMS] ʹ MinGW GCC ...
    gcc -std=c11 -Wall -Wextra -O2 -finput-charset=GBK -fexec-charset=GBK -o tsms.exe src\main.c
    if %errorlevel% neq 0 exit /b %errorlevel%
    echo [TSMS] ɹtsms.exe
    exit /b 0
)

where cl > nul 2> nul
if %errorlevel%==0 (
    echo [TSMS] ʹ Visual Studio cl ...
    cl /source-charset:gbk /execution-charset:gbk /W4 /O2 /Fe:tsms.exe src\main.c
    if %errorlevel% neq 0 exit /b %errorlevel%
    echo [TSMS] ɹtsms.exe
    exit /b 0
)

echo [TSMS] δҵ gcc  cl
echo 밲װ MinGW-w64 Visual Studio Developer Command Prompt бű
exit /b 1
