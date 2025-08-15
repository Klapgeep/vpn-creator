#include "vpn.h"
#include <getopt.h>
#include <signal.h>

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
    printf("\nShutting down VPN server...\n");
}

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -p, --port PORT     Listen port (default: 1194)\n");
    printf("  -k, --key FILE      Key file path\n");
    printf("  -d, --device DEV    TUN device name (default: tun0)\n");
    printf("  -h, --help          Show this help\n");
}

int main(int argc, char *argv[]) {
    int port = 1194;
    char *key_file = NULL;
    char *tun_device = "tun0";
    int opt;
    
    struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"key", required_argument, 0, 'k'},
        {"device", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "p:k:d:h", long_options, NULL)) != -1) {
        switch (opt) {
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
    
    if (!key_file) {
        fprintf(stderr, "Error: Key file is required\n");
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
    printf("VPN Server starting...\n");
    printf("Port: %d\n", port);
    printf("TUN Device: %s\n", tun_device);
    
    // Create TUN interface
    ctx.tun_fd = create_tun_interface(tun_device);
    if (ctx.tun_fd < 0) {
        fprintf(stderr, "Failed to create TUN interface\n");
        return 1;
    }
    printf("TUN interface created: %s\n", tun_device);
    
    // Set up server socket
    ctx.sock_fd = setup_server_socket(port);
    if (ctx.sock_fd < 0) {
        fprintf(stderr, "Failed to create server socket\n");
        close(ctx.tun_fd);
        return 1;
    }
    printf("Server socket listening on port %d\n", port);
    
    // Start server loop
    printf("VPN Server ready. Waiting for connections...\n");
    vpn_server_loop(&ctx);
    
    // Cleanup
    cleanup_vpn(&ctx);
    EVP_cleanup();
    
    return 0;
}