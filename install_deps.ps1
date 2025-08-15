# VPN Creator - Windows Dependencies Installer
# Run as Administrator

Write-Host "Installing VPN Creator Dependencies..." -ForegroundColor Green

# Check if running as Administrator
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "Please run this script as Administrator!" -ForegroundColor Red
    exit 1
}

# Install Chocolatey if not present
if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Host "Installing Chocolatey..." -ForegroundColor Yellow
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
}

# Install MinGW-w64
Write-Host "Installing MinGW-w64..." -ForegroundColor Yellow
choco install mingw -y

# Install OpenSSL
Write-Host "Installing OpenSSL..." -ForegroundColor Yellow
choco install openssl -y

# Install TAP-Win32 adapter
Write-Host "Installing TAP-Win32 adapter..." -ForegroundColor Yellow
$tapUrl = "https://build.openvpn.net/downloads/releases/tap-windows-9.24.7-I601-Win10.exe"
$tapInstaller = "$env:TEMP\tap-windows-installer.exe"

try {
    Invoke-WebRequest -Uri $tapUrl -OutFile $tapInstaller
    Start-Process -FilePath $tapInstaller -Args "/S" -Wait
    Remove-Item $tapInstaller
    Write-Host "TAP adapter installed successfully" -ForegroundColor Green
} catch {
    Write-Host "Failed to install TAP adapter. Please install manually from: https://openvpn.net/community-downloads/" -ForegroundColor Yellow
}

# Create batch files for easy compilation
Write-Host "Creating build scripts..." -ForegroundColor Yellow

$buildScript = @"
@echo off
echo Building VPN Creator...

set OPENSSL_DIR=C:\Program Files\OpenSSL-Win64
set MINGW_DIR=C:\ProgramData\chocolatey\lib\mingw\tools\install\mingw64

set PATH=%MINGW_DIR%\bin;%PATH%
set CPATH=%OPENSSL_DIR%\include;%CPATH%
set LIBRARY_PATH=%OPENSSL_DIR%\lib;%LIBRARY_PATH%

gcc -o vpn-server.exe src/vpn_server.c src/crypto.c src/network.c src/tun.c src/windows_tap.c -Iinclude -I"%OPENSSL_DIR%\include" -L"%OPENSSL_DIR%\lib" -lssl -lcrypto -lws2_32 -liphlpapi -ladvapi32 -static-libgcc

gcc -o vpn-client.exe src/vpn_client.c src/crypto.c src/network.c src/tun.c src/windows_tap.c -Iinclude -I"%OPENSSL_DIR%\include" -L"%OPENSSL_DIR%\lib" -lssl -lcrypto -lws2_32 -liphlpapi -ladvapi32 -static-libgcc

if exist vpn-server.exe (
    echo Build successful!
    echo.
    echo Usage:
    echo   Server: vpn-server.exe -p 1194 -k server.key
    echo   Client: vpn-client.exe -s SERVER_IP -p 1194 -k client.key
) else (
    echo Build failed. Check error messages above.
)

pause
"@

Set-Content -Path "build.bat" -Value $buildScript

Write-Host "Dependencies installed! Run build.bat to compile the VPN." -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Run build.bat to compile"
Write-Host "2. Run setup_keys.bat to generate keys"
Write-Host "3. Start server with: vpn-server.exe -p 1194 -k server.key"
Write-Host "4. Connect client with: vpn-client.exe -s SERVER_IP -p 1194 -k client.key"