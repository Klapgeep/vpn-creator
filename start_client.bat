@echo off
title VPN Client

REM Check if running as Administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Starting VPN Client...
echo.

if not exist vpn-client.exe (
    echo vpn-client.exe not found. Please run build.bat first.
    pause
    exit /b 1
)

if not exist keys\client.key (
    echo Client key not found. Please run setup_keys.bat first.
    pause
    exit /b 1
)

set /p SERVER_IP=Enter VPN server IP address: 

if "%SERVER_IP%"=="" (
    echo No server IP provided. Exiting.
    pause
    exit /b 1
)

echo.
echo VPN Client Configuration:
echo - Server: %SERVER_IP%:1194
echo - Key: keys\client.key
echo - Local IP: 10.8.0.2
echo - Remote IP: 10.8.0.1
echo.
echo Connecting to VPN server...
echo Press Ctrl+C to disconnect
echo.

vpn-client.exe -s %SERVER_IP% -p 1194 -k keys\client.key -d tap1

pause