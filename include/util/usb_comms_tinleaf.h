/**
 * @file usb_comms.h
 * @brief USB comms.
 * @author yellows8
 * @author plutoo
 * @copyright libnx Authors
 */
#ifdef __cplusplus
extern "C" {
#endif

#pragma once
#include "switch/types.h"

typedef struct {
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
} tinleaf_UsbCommsInterfaceInfo;

/// Initializes usbComms with the default number of interfaces (1)
Result tinleaf_usbCommsInitialize(void);

/// Initializes usbComms with a specific number of interfaces.
Result tinleaf_usbCommsInitializeEx(u32 num_interfaces, const tinleaf_UsbCommsInterfaceInfo *infos);

/// Exits usbComms.
void tinleaf_usbCommsExit(void);

/// Sets whether to throw a fatal error in usbComms{Read/Write}* on failure, or just return the transferred size. By default (false) the latter is used.
void tinleaf_usbCommsSetErrorHandling(bool flag);

/// Read data with the default interface.
size_t tinleaf_usbCommsRead(void* buffer, size_t size, u64 timeout);

/// Write data with the default interface.
size_t tinleaf_usbCommsWrite(const void* buffer, size_t size, u64 timeout);

/// Same as usbCommsRead except with the specified interface.
size_t tinleaf_usbCommsReadEx(void* buffer, size_t size, u32 interface, u64 timeout);

/// Same as usbCommsWrite except with the specified interface.
size_t tinleaf_usbCommsWriteEx(const void* buffer, size_t size, u32 interface, u64 timeout);

#ifdef __cplusplus
}
#endif