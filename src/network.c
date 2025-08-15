#include "vpn.h"
#include <sys/select.h>
#include <errno.h>

int setup_server_socket(int port) {
    int sock_fd;
    struct sockaddr_in server_addr;
    int opt = 1;
    
    // Create socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    
    // Set socket options
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sock_fd);
        return -1;
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Bind socket
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

int connect_to_server(const char *server_ip, int port) {
    int sock_fd;
    struct sockaddr_in server_addr;
    
    // Create socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock_fd);
        return -1;
    }
    
    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

void vpn_server_loop(vpn_context_t *ctx) {
    fd_set read_fds;
    int max_fd;
    ssize_t n;
    vpn_packet_t packet;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t decrypted_data[MAX_PACKET_SIZE];
    
    max_fd = (ctx->tun_fd > ctx->sock_fd) ? ctx->tun_fd : ctx->sock_fd;
    
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(ctx->tun_fd, &read_fds);
        FD_SET(ctx->sock_fd, &read_fds);
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        // Data from TUN interface (to be sent to client)
        if (FD_ISSET(ctx->tun_fd, &read_fds)) {
            n = read(ctx->tun_fd, packet.data, MAX_PACKET_SIZE);
            if (n > 0) {
                packet.length = htonl(n);
                
                // Encrypt packet
                int encrypted_len = encrypt_packet(packet.data, n, ctx->key, 
                                                 packet.data, packet.iv);
                if (encrypted_len > 0) {
                    packet.length = htonl(encrypted_len);
                    sendto(ctx->sock_fd, &packet, sizeof(packet.length) + 
                          encrypted_len + AES_IV_SIZE, 0, 
                          (struct sockaddr *)&client_addr, client_len);
                }
            }
        }
        
        // Data from client socket (to be sent to TUN)
        if (FD_ISSET(ctx->sock_fd, &read_fds)) {
            n = recvfrom(ctx->sock_fd, &packet, sizeof(packet), 0,
                        (struct sockaddr *)&client_addr, &client_len);
            if (n > 0) {
                int data_len = ntohl(packet.length);
                
                // Decrypt packet
                int decrypted_len = decrypt_packet(packet.data, data_len,
                                                 ctx->key, packet.iv, decrypted_data);
                if (decrypted_len > 0) {
                    write(ctx->tun_fd, decrypted_data, decrypted_len);
                }
            }
        }
    }
}

void vpn_client_loop(vpn_context_t *ctx) {
    fd_set read_fds;
    int max_fd;
    ssize_t n;
    vpn_packet_t packet;
    uint8_t decrypted_data[MAX_PACKET_SIZE];
    
    max_fd = (ctx->tun_fd > ctx->sock_fd) ? ctx->tun_fd : ctx->sock_fd;
    
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(ctx->tun_fd, &read_fds);
        FD_SET(ctx->sock_fd, &read_fds);
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        
        // Data from TUN interface (to be sent to server)
        if (FD_ISSET(ctx->tun_fd, &read_fds)) {
            n = read(ctx->tun_fd, packet.data, MAX_PACKET_SIZE);
            if (n > 0) {
                packet.length = htonl(n);
                
                // Encrypt packet
                int encrypted_len = encrypt_packet(packet.data, n, ctx->key,
                                                 packet.data, packet.iv);
                if (encrypted_len > 0) {
                    packet.length = htonl(encrypted_len);
                    send(ctx->sock_fd, &packet, sizeof(packet.length) + 
                        encrypted_len + AES_IV_SIZE, 0);
                }
            }
        }
        
        // Data from server socket (to be sent to TUN)
        if (FD_ISSET(ctx->sock_fd, &read_fds)) {
            n = recv(ctx->sock_fd, &packet, sizeof(packet), 0);
            if (n > 0) {
                int data_len = ntohl(packet.length);
                
                // Decrypt packet
                int decrypted_len = decrypt_packet(packet.data, data_len,
                                                 ctx->key, packet.iv, decrypted_data);
                if (decrypted_len > 0) {
                    write(ctx->tun_fd, decrypted_data, decrypted_len);
                }
            }
        }
    }
}

void cleanup_vpn(vpn_context_t *ctx) {
    if (ctx->tun_fd >= 0) {
        close(ctx->tun_fd);
    }
    if (ctx->sock_fd >= 0) {
        close(ctx->sock_fd);
    }
    if (ctx->ssl) {
        SSL_free(ctx->ssl);
    }
    if (ctx->ssl_ctx) {
        SSL_CTX_free(ctx->ssl_ctx);
    }
}