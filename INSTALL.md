# VPN Creator - Installation Guide

## Quick Setup (Windows)

### 1. Install Dependencies (Run as Administrator)
```powershell
PowerShell -ExecutionPolicy Bypass -File install_deps.ps1
```

### 2. Build the VPN
```cmd
build.bat
```

### 3. Setup Keys and Configuration
```cmd
setup_keys.bat
```

### 4. Start VPN Server (Run as Administrator)
```cmd
start_server.bat
```

### 5. Connect VPN Client (Run as Administrator)
```cmd
start_client.bat
```

## Manual Installation

### Prerequisites
- Windows 10/11
- Administrator privileges
- MinGW-w64 compiler
- OpenSSL development libraries
- TAP-Win32 network adapter

### Step-by-Step

1. **Install MinGW-w64**
   - Download from: https://www.mingw-w64.org/downloads/
   - Add to PATH: `C:\mingw64\bin`

2. **Install OpenSSL**
   - Download from: https://slproweb.com/products/Win32OpenSSL.html
   - Install to: `C:\Program Files\OpenSSL-Win64`

3. **Install TAP-Win32 Adapter**
   - Download OpenVPN: https://openvpn.net/community-downloads/
   - During installation, ensure TAP-Win32 adapter is installed
   - Or download standalone: https://build.openvpn.net/downloads/releases/

4. **Compile VPN**
   ```cmd
   gcc -o vpn-server.exe src/*.c -Iinclude -I"C:\Program Files\OpenSSL-Win64\include" -L"C:\Program Files\OpenSSL-Win64\lib" -lssl -lcrypto -lws2_32 -liphlpapi -ladvapi32
   gcc -o vpn-client.exe src/*.c -Iinclude -I"C:\Program Files\OpenSSL-Win64\include" -L"C:\Program Files\OpenSSL-Win64\lib" -lssl -lcrypto -lws2_32 -liphlpapi -ladvapi32
   ```

## Configuration

### Server Setup
1. Run `setup_keys.bat` to generate keys
2. Edit `keys\server.conf` if needed
3. Run `start_server.bat` as Administrator

### Client Setup
1. Copy `keys\client.key` from server
2. Edit `keys\client.conf` with server IP
3. Run `start_client.bat` as Administrator

## Network Configuration

After connecting, configure IP routes:

### Server Machine
```cmd
route add 10.8.0.0 mask 255.255.255.0 10.8.0.2
```

### Client Machine
```cmd
route add 0.0.0.0 mask 0.0.0.0 10.8.0.1
```

## Firewall Configuration

Allow UDP port 1194 through Windows Firewall:
```cmd
netsh advfirewall firewall add rule name="VPN Server" dir=in action=allow protocol=UDP localport=1194
```

## Troubleshooting

### TAP Adapter Issues
- Ensure TAP-Win32 adapter is installed
- Check Device Manager for TAP adapters
- Reinstall OpenVPN TAP driver if needed

### Compilation Errors
- Verify MinGW-w64 is in PATH
- Check OpenSSL library paths
- Run as Administrator

### Connection Issues
- Verify firewall rules
- Check server IP and port
- Ensure both machines have TAP adapters

## Security Notes

- Change default keys in production
- Use proper key exchange protocols
- Enable logging for debugging
- Regular security updates

## Support

For issues:
1. Check Windows Event Viewer
2. Verify TAP adapter status
3. Test with `ping 10.8.0.1` after connection
4. Submit issues to GitHub repository