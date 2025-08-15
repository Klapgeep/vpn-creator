#include "vpn.h"

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#include <iphlpapi.h>

// Windows TAP interface functions (defined in windows_tap.c)
extern int create_tap_interface_windows(void);
extern int configure_tap_interface(HANDLE tap_fd, const char *local_ip, const char *remote_ip, const char *netmask);

int create_tun_interface(const char *dev_name) {
    printf("Creating Windows TAP interface...\n");
    return create_tap_interface_windows();
}

#else
// Linux/Unix TUN interface
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
#endif