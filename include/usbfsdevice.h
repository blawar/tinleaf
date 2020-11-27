#ifndef __USBFSDEVICE_H__
#define __USBFSDEVICE_H__

#define USBFS_UNMOUNTED         0
#define USBFS_MOUNTED           1
#define USBFS_UNSUPPORTED_FS    2

#if defined (__cplusplus)
extern "C" {
#endif
    
// Register "usbhdd:" device
void usbFsDeviceRegister(void);

// Keep calling update periodically to check USB drive ready
// Returns 1 if status changed
int usbFsDeviceUpdate(void);

// If status changed, check mount status
// Returns 0 == USBFS_UNMOUNTED, 1 == USBFS_MOUNTED, 2 == USBFS_UNSUPPORTED_FS
int usbFsDeviceGetMountStatus(void);

#if defined (__cplusplus)
}
#endif

#endif
