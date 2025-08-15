@echo off
echo Setting up VPN keys...

REM Create keys directory
if not exist "keys" mkdir keys

REM Generate server key (32 bytes for AES-256)
echo Creating server key...
openssl rand -hex 32 > keys\server.key

REM Generate client key (should match server in production)
echo Creating client key...
copy keys\server.key keys\client.key

REM Create server config
echo Creating server config...
(
echo # VPN Server Configuration
echo port=1194
echo key_file=keys\server.key
echo local_ip=10.8.0.1
echo remote_ip=10.8.0.2
echo netmask=255.255.255.0
echo device=tap0
) > keys\server.conf

REM Create client config  
echo Creating client config...
(
echo # VPN Client Configuration
echo server_ip=SERVER_IP_HERE
echo port=1194
echo key_file=keys\client.key
echo local_ip=10.8.0.2
echo remote_ip=10.8.0.1
echo netmask=255.255.255.0
echo device=tap1
) > keys\client.conf

echo.
echo Keys and configuration files created in 'keys' directory
echo.
echo IMPORTANT:
echo 1. Edit keys\client.conf and replace SERVER_IP_HERE with actual server IP
echo 2. Copy keys\client.key and keys\client.conf to client machine
echo 3. Keep keys\server.key secure on server machine only
echo.
echo Ready to start VPN!

pause