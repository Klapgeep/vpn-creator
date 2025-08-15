#ifdef _WIN32
#include "vpn.h"
#include <windows.h>
#include <winioctl.h>
#include <iphlpapi.h>

#define TAP_CONTROL_CODE(request,method) CTL_CODE(FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_MAC               TAP_CONTROL_CODE(1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION           TAP_CONTROL_CODE(2, METHOD_BUFFERED)
#define TAP_IOCTL_GET_MTU               TAP_CONTROL_CODE(3, METHOD_BUFFERED)
#define TAP_IOCTL_GET_INFO              TAP_CONTROL_CODE(4, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_POINT_TO_POINT TAP_CONTROL_CODE(5, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS      TAP_CONTROL_CODE(6, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_MASQ      TAP_CONTROL_CODE(7, METHOD_BUFFERED)
#define TAP_IOCTL_GET_LOG_LINE          TAP_CONTROL_CODE(8, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_SET_OPT   TAP_CONTROL_CODE(9, METHOD_BUFFERED)

#define USERMODEDEVICEDIR "\\\\.\\Global\\"
#define TAPSUFFIX         ".tap"

char *get_tap_device_guid(void) {
    HKEY adapter_key;
    LONG status;
    DWORD len;
    char *device_guid = NULL;
    
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                         "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 
                         0, KEY_READ, &adapter_key);
    
    if (status != ERROR_SUCCESS) {
        return NULL;
    }
    
    for (int i = 0; ; ++i) {
        char enum_name[256];
        char unit_string[256];
        HKEY unit_key;
        char component_id_string[] = "ComponentId";
        char component_id[256];
        char net_cfg_instance_id_string[] = "NetCfgInstanceId";
        char net_cfg_instance_id[256];
        DWORD data_type;
        
        len = sizeof(enum_name);
        status = RegEnumKeyEx(adapter_key, i, enum_name, &len, NULL, NULL, NULL, NULL);
        if (status == ERROR_NO_MORE_ITEMS) break;
        if (status != ERROR_SUCCESS) continue;
        
        snprintf(unit_string, sizeof(unit_string), 
                "%s\\%s", 
                "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}",
                enum_name);
                
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, unit_string, 0, KEY_READ, &unit_key);
        if (status != ERROR_SUCCESS) continue;
        
        len = sizeof(component_id);
        status = RegQueryValueEx(unit_key, component_id_string, NULL, &data_type, 
                                (LPBYTE)component_id, &len);
        
        if (status == ERROR_SUCCESS && data_type == REG_SZ) {
            if (strstr(component_id, "tap")) {
                len = sizeof(net_cfg_instance_id);
                status = RegQueryValueEx(unit_key, net_cfg_instance_id_string, NULL, 
                                       &data_type, (LPBYTE)net_cfg_instance_id, &len);
                
                if (status == ERROR_SUCCESS && data_type == REG_SZ) {
                    device_guid = malloc(strlen(net_cfg_instance_id) + 1);
                    strcpy(device_guid, net_cfg_instance_id);
                    RegCloseKey(unit_key);
                    break;
                }
            }
        }
        RegCloseKey(unit_key);
    }
    
    RegCloseKey(adapter_key);
    return device_guid;
}

int create_tap_interface_windows(void) {
    char *device_guid;
    char device_path[256];
    HANDLE tap_fd;
    ULONG status;
    DWORD len;
    
    // Get TAP device GUID
    device_guid = get_tap_device_guid();
    if (!device_guid) {
        fprintf(stderr, "No TAP device found. Please install TAP-Win32 adapter.\n");
        return -1;
    }
    
    // Create device path
    snprintf(device_path, sizeof(device_path), "%s%s%s", 
             USERMODEDEVICEDIR, device_guid, TAPSUFFIX);
    
    printf("Opening TAP device: %s\n", device_path);
    
    // Open TAP device
    tap_fd = CreateFile(device_path, GENERIC_READ | GENERIC_WRITE, 0, 0, 
                       OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, 0);
    
    if (tap_fd == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open TAP device: %d\n", GetLastError());
        free(device_guid);
        return -1;
    }
    
    // Set media status to connected
    status = TRUE;
    if (!DeviceIoControl(tap_fd, TAP_IOCTL_SET_MEDIA_STATUS, &status, sizeof(status), 
                        &status, sizeof(status), &len, NULL)) {
        fprintf(stderr, "Failed to set media status: %d\n", GetLastError());
        CloseHandle(tap_fd);
        free(device_guid);
        return -1;
    }
    
    printf("TAP interface opened successfully\n");
    free(device_guid);
    return (int)(intptr_t)tap_fd;
}

int configure_tap_interface(HANDLE tap_fd, const char *local_ip, const char *remote_ip, const char *netmask) {
    DWORD len;
    ULONG ep[3];
    
    // Convert IP addresses
    ep[0] = inet_addr(local_ip);
    ep[1] = inet_addr(remote_ip);  
    ep[2] = inet_addr(netmask);
    
    // Configure point-to-point
    if (!DeviceIoControl(tap_fd, TAP_IOCTL_CONFIG_POINT_TO_POINT, ep, sizeof(ep), 
                        ep, sizeof(ep), &len, NULL)) {
        fprintf(stderr, "Failed to configure TAP interface: %d\n", GetLastError());
        return -1;
    }
    
    printf("TAP interface configured: %s -> %s (%s)\n", local_ip, remote_ip, netmask);
    return 0;
}

#endif // _WIN32