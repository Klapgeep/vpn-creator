#include "vpn.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

int create_tun_interface(const char *dev_name) {
    struct ifreq ifr;
    int fd, err;
    
    // Open TUN device
    if ((fd = open(TUN_DEVICE, O_RDWR)) < 0) {
        perror("open");
        return -1;
    }
    
    // Configure TUN interface
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    
    if (dev_name) {
        strncpy(ifr.ifr_name, dev_name, IFNAMSIZ - 1);
    }
    
    // Create the interface
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        perror("ioctl");
        close(fd);
        return -1;
    }
    
    printf("TUN interface %s created\n", ifr.ifr_name);
    return fd;
}

// Windows-specific TUN interface creation (placeholder)
#ifdef _WIN32
int create_tun_interface_windows(const char *dev_name) {
    // Windows TAP-Win32 adapter implementation would go here
    // This requires the TAP-Win32 driver to be installed
    printf("Windows TUN interface creation not yet implemented\n");
    printf("Please install TAP-Win32 adapter for Windows support\n");
    return -1;
}
#endif