@echo off
echo Building VPN Creator...

REM Compile server
gcc -o vpn-server.exe src/vpn_server.c src/crypto.c src/network.c src/tun.c -Iinclude -lssl -lcrypto -lws2_32 -static-libgcc

REM Compile client  
gcc -o vpn-client.exe src/vpn_client.c src/crypto.c src/network.c src/tun.c -Iinclude -lssl -lcrypto -lws2_32 -static-libgcc

echo Build complete!
echo.
echo Usage:
echo   Server: vpn-server.exe -p 1194 -k server.key
echo   Client: vpn-client.exe -s SERVER_IP -p 1194 -k client.key