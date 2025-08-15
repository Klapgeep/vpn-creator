# VPN Creator

A lightweight VPN implementation in C for privacy and security purposes.

## Features

- Custom VPN server and client
- TUN/TAP interface support
- Strong encryption (AES-256)
- Cross-platform compatibility
- Simple configuration

## Building

```bash
make
```

## Usage

### Server
```bash
./vpn-server -p 1194 -k server.key
```

### Client
```bash
./vpn-client -h server_ip -p 1194 -k client.key
```

## Security Notice

This VPN is designed for legitimate privacy and security purposes only. Use responsibly and in accordance with local laws.

## License

MIT License