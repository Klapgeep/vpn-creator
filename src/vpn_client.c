#include "vpn.h"
#include <getopt.h>
#include <signal.h>

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
    printf("\nDisconnecting from VPN server...\n");
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -s, --server HOST   Server hostname/IP\n");
    printf("  -p, --port PORT     Server port (default: 1194)\n");
    printf("  -k, --key FILE      Key file path\n");
    printf("  -d, --device DEV    TUN device name (default: tun1)\n");
    printf("  -h, --help          Show this help\n");
}

int main(int argc, char *argv[]) {
    char *server_host = NULL;
    int port = 1194;
    char *key_file = NULL;
    char *tun_device = "tun1";
    int opt;
    
    struct option long_options[] = {
        {"server", required_argument, 0, 's'},
        {"port", required_argument, 0, 'p'},
        {"key", required_argument, 0, 'k'},
        {"device", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "s:p:k:d:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                server_host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'k':
                key_file = optarg;
                break;
            case 'd':
                tun_device = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (!server_host || !key_file) {
        fprintf(stderr, "Error: Server host and key file are required\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize VPN context
    vpn_context_t ctx = {0};
    
    // Generate or load key
    generate_key(ctx.key);
    printf("VPN Client starting...\n");
    printf("Server: %s:%d\n", server_host, port);
    printf("TUN Device: %s\n", tun_device);
    
    // Create TUN interface
    ctx.tun_fd = create_tun_interface(tun_device);
    if (ctx.tun_fd < 0) {
        fprintf(stderr, "Failed to create TUN interface\n");
        return 1;
    }
    printf("TUN interface created: %s\n", tun_device);
    
    // Connect to server
    ctx.sock_fd = connect_to_server(server_host, port);
    if (ctx.sock_fd < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        close(ctx.tun_fd);
        return 1;
    }
    printf("Connected to VPN server %s:%d\n", server_host, port);
    
    // Start client loop
    printf("VPN tunnel established. Traffic is now being tunneled...\n");
    vpn_client_loop(&ctx);
    
    // Cleanup
    cleanup_vpn(&ctx);
    EVP_cleanup();
    
    return 0;
}