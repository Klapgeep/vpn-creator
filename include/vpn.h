#ifndef VPN_H
#define VPN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_PACKET_SIZE 2048
#define AES_KEY_SIZE 32
#define AES_IV_SIZE 16
#define TUN_DEVICE "/dev/net/tun"

// VPN packet structure
typedef struct {
    uint32_t length;
    uint8_t data[MAX_PACKET_SIZE];
    uint8_t iv[AES_IV_SIZE];
} vpn_packet_t;

// VPN context
typedef struct {
    int tun_fd;
    int sock_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    uint8_t key[AES_KEY_SIZE];
    SSL_CTX *ssl_ctx;
    SSL *ssl;
} vpn_context_t;

// Function prototypes
int create_tun_interface(const char *dev_name);
int setup_server_socket(int port);
int connect_to_server(const char *server_ip, int port);
int encrypt_packet(const uint8_t *plaintext, int plaintext_len, 
                  const uint8_t *key, uint8_t *ciphertext, uint8_t *iv);
int decrypt_packet(const uint8_t *ciphertext, int ciphertext_len,
                  const uint8_t *key, const uint8_t *iv, uint8_t *plaintext);
void generate_key(uint8_t *key);
void vpn_server_loop(vpn_context_t *ctx);
void vpn_client_loop(vpn_context_t *ctx);
void cleanup_vpn(vpn_context_t *ctx);

#endif // VPN_H