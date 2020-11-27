#ifndef __USBFS_H__
#define __USBFS_H__

#include <switch.h>

enum UsbFsServiceCmd {
    UsbFs_Cmd_GetMountStatus = 0,
    UsbFs_Cmd_OpenFile = 1,
    UsbFs_Cmd_CloseFile = 2,
    UsbFs_Cmd_ReadFile = 3,
    UsbFs_Cmd_IsReady = 4,
    UsbFs_Cmd_OpenDir = 5,
    UsbFs_Cmd_CloseDir = 6,
    UsbFs_Cmd_ReadDir = 7,
    UsbFs_Cmd_CreateDir = 8,
    UsbFs_Cmd_SeekFile = 9,
    UsbFs_Cmd_ReadRaw = 10,
    UsbFs_Cmd_WriteFile = 11,
    UsbFs_Cmd_SyncFile = 12,
    UsbFs_Cmd_DeleteDir = 13,
    UsbFs_Cmd_DeleteFile = 14,
    UsbFs_Cmd_TruncateFile = 15,
    UsbFs_Cmd_StatFile = 16,
    UsbFs_Cmd_StatPath = 17,
    UsbFs_Cmd_StatFilesystem = 18,
};

#ifdef __cplusplus
extern "C" {
#endif

Result usbFsIsInitialized(void);
Result usbFsInitialize(void);
void usbFsExit(void);
Result usbFsIsReady(void);
Result usbFsGetMountStatus(u64* status);
Result usbFsOpenFile(u64* fileid, const char* filepath, u64 mode);
Result usbFsCloseFile(u64 fileid);
Result usbFsReadFile(u64 fileid, void* buffer, size_t size, size_t* retsize);
Result usbFsWriteFile(u64 fileid, const void* buffer, size_t size, size_t* retsize);
Result usbFsSeekFile(u64 fileid, u64 pos, u64 whence, u64 *retpos);
Result usbFsSyncFile(u64 fileid);
Result usbFsTruncateFile(u64 fileid, u64 size);
Result usbFsDeleteFile(const char* filepath);
Result usbFsStatFile(u64 fileid, u64* size, u64* mode);
Result usbFsStatPath(const char* path, u64* size, u64* mode);
Result usbFsStatFilesystem(u64* totalsize, u64* freesize);
Result usbFsOpenDir(u64* dirid, const char* dirpath);
Result usbFsReadDir(u64 dirid, u64* type, u64* size, char* name, size_t namemax);
Result usbFsCloseDir(u64 dirid);
Result usbFsCreateDir(const char* dirpath);
Result usbFsDeleteDir(const char* dirpath);
Result usbFsReadRaw(u64 sector, u64 sectorcount, void* buffer);
Result sxIsAuth();

#ifdef __cplusplus
}
#endif

#endif
