@echo off
title VPN Server

REM Check if running as Administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Starting VPN Server...
echo.

if not exist vpn-server.exe (
    echo vpn-server.exe not found. Please run build.bat first.
    pause
    exit /b 1
)

if not exist keys\server.key (
    echo Server key not found. Please run setup_keys.bat first.
    pause
    exit /b 1
)

echo VPN Server Configuration:
echo - Port: 1194
echo - Key: keys\server.key
echo - Local IP: 10.8.0.1
echo - Remote IP: 10.8.0.2
echo.
echo Press Ctrl+C to stop the server
echo.

vpn-server.exe -p 1194 -k keys\server.key -d tap0

pause