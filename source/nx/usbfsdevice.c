//#ifdef __SX__
// Copyright (c) 2018 Team Xecuter
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/param.h>
#include <unistd.h>
#include <switch.h>
#include <stdio.h>
#include "usbfs.h"
#include "usbfsdevice.h"


static u64 g_mountstatus = USBFS_UNMOUNTED;

static int       usbfs_dev_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int       usbfs_dev_close(struct _reent *r, void *fd);
static ssize_t   usbfs_dev_read(struct _reent *r, void *fd, char *ptr, size_t len);
static ssize_t   usbfs_dev_write(struct _reent *r, void *fd, const char *ptr, size_t len);
static off_t     usbfs_dev_seek(struct _reent *r, void *fd, off_t pos, int dir);
static int       usbfs_dev_fstat(struct _reent *r, void *fd, struct stat *st);
static int       usbfs_dev_stat(struct _reent *r, const char *path, struct stat *st);
static int       usbfs_dev_unlink(struct _reent *r, const char *path);
static int       usbfs_dev_chdir(struct _reent *r, const char *path);
static int       usbfs_dev_mkdir(struct _reent *r, const char *path, int mode);
static DIR_ITER* usbfs_dev_diropen(struct _reent *r, DIR_ITER *dirState, const char *path);
static int       usbfs_dev_dirreset(struct _reent *r, DIR_ITER *dirState);
static int       usbfs_dev_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
static int       usbfs_dev_dirclose(struct _reent *r, DIR_ITER *dirState);
static int       usbfs_dev_ftruncate(struct _reent *r, void *fd, off_t len);
static int       usbfs_dev_fsync(struct _reent *r, void *fd);
static int       usbfs_dev_rmdir(struct _reent *r, const char *path);
static int       usbfs_dev_statvfs(struct _reent *r, const char *path, struct statvfs *buf);

typedef struct {
    uint64_t fileid;
    int flags;
} usbfs_dev_file;

typedef struct
{
    uint64_t dirid;
} usbfs_dev_dir;

static devoptab_t usbfs_dev_devoptab =
{
    .name         = "usbhdd",
    .structSize   = sizeof(usbfs_dev_file),
    .open_r       = usbfs_dev_open,
    .close_r      = usbfs_dev_close,
    .read_r       = usbfs_dev_read,
    .write_r      = usbfs_dev_write,
    .seek_r       = usbfs_dev_seek,
    .fstat_r      = usbfs_dev_fstat,
    .stat_r       = usbfs_dev_stat,
    .unlink_r     = usbfs_dev_unlink,
    .chdir_r      = usbfs_dev_chdir,
    .mkdir_r      = usbfs_dev_mkdir,
    .dirStateSize = sizeof(usbfs_dev_dir),
    .diropen_r    = usbfs_dev_diropen,
    .dirreset_r   = usbfs_dev_dirreset,
    .dirnext_r    = usbfs_dev_dirnext,
    .dirclose_r   = usbfs_dev_dirclose,
    .statvfs_r    = usbfs_dev_statvfs,
    .ftruncate_r  = usbfs_dev_ftruncate,
    .fsync_r      = usbfs_dev_fsync,
    .deviceData   = 0,
    .rmdir_r      = usbfs_dev_rmdir,
};

void usbFsDeviceRegister() {
    AddDevice(&usbfs_dev_devoptab);
}

int usbFsDeviceUpdate() {
    u64 status;
    Result rc = usbFsGetMountStatus(&status);

    if (R_FAILED(rc)) {
        return 0;
    }

    if (g_mountstatus == status) {
        return 0;
    }

    g_mountstatus = status;
    return 1;
}

int usbFsDeviceGetMountStatus() {
    return g_mountstatus;
}

int usbfs_dev_open(struct _reent *r, void *fd, const char *path, int flags, int mode) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    Result rc;

    char* pathAtColon = strchr(path, ':');
    if (pathAtColon) {
        path = pathAtColon + 1;
    }

    memset(file, 0, sizeof(usbfs_dev_file));
    file->flags = flags;
    rc = usbFsOpenFile(&file->fileid, path, flags);
    if (R_FAILED(rc)) {
        r->_errno = ENOENT;
        return -1;
    }

    return 0;
}

int usbfs_dev_close(struct _reent *r, void *fd) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    Result rc = usbFsCloseFile(file->fileid);
    file->fileid = 0xFFFFFFFF;

    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

ssize_t usbfs_dev_read(struct _reent *r, void *fd, char *ptr, size_t len) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    u64 retsize;

    Result rc = usbFsReadFile(file->fileid, ptr, len, &retsize);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return retsize;
}


ssize_t usbfs_dev_write(struct _reent *r, void *fd, const char *ptr, size_t len) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    u64 retsize;

    Result rc = usbFsWriteFile(file->fileid, ptr, len, &retsize);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    if (file->flags & O_SYNC) {
        usbFsSyncFile(file->fileid);
    }

    return retsize;
}

off_t usbfs_dev_seek(struct _reent *r, void *fd, off_t pos, int whence) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    u64 retpos;
    Result rc = usbFsSeekFile(file->fileid, pos, whence, &retpos);

    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return retpos;
}

int usbfs_dev_unlink(struct _reent *r, const char *path) {
    char* pathAtColon = strchr(path, ':');
    if (pathAtColon) {
        path = pathAtColon + 1;
    }

    Result rc = usbFsDeleteFile(path);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

int usbfs_dev_ftruncate(struct _reent *r, void *fd, off_t len) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    Result rc;

    if (len < 0) {
        r->_errno = EINVAL;
        return -1;
    }

    rc = usbFsTruncateFile(file->fileid, len);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

int usbfs_dev_fsync(struct _reent *r, void *fd) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    Result rc;

    rc = usbFsSyncFile(file->fileid);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

int usbfs_dev_fstat(struct _reent *r, void *fd, struct stat *st) {
    usbfs_dev_file* file = (usbfs_dev_file*)fd;
    u64 size;
    u64 mode;
    Result rc;

    rc = usbFsStatFile(file->fileid, &size, &mode);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_nlink = 1;
    st->st_size = (off_t)size;
    st->st_mode = mode;
    return 0;
}

int usbfs_dev_stat(struct _reent *r, const char *path, struct stat *st) {
    u64 size;
    u64 mode;
    Result rc;

    rc = usbFsStatPath(path, &size, &mode);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    memset(st, 0, sizeof(struct stat));
    st->st_nlink = 1;
    st->st_size = (off_t)size;
    st->st_mode = mode;
    return 0;
}

int usbfs_dev_chdir(struct _reent *r, const char *path) {
    r->_errno = EINVAL;
    return -1;
}

int usbfs_dev_mkdir(struct _reent *r, const char *path, int mode) {
    char* pathAtColon = strchr(path, ':');
    if (pathAtColon) {
        path = pathAtColon + 1;
    }

    Result rc = usbFsCreateDir(path);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

DIR_ITER* usbfs_dev_diropen(struct _reent *r, DIR_ITER *dirState, const char *path) {
    usbfs_dev_dir *d = (usbfs_dev_dir*)(dirState->dirStruct);
    Result rc;

    char* pathAtColon = strchr(path, ':');
    if (pathAtColon) {
        path = pathAtColon + 1;
    }

    rc = usbFsOpenDir(&d->dirid, path);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return NULL;
    }

    return dirState;
}

int usbfs_dev_dirreset(struct _reent *r, DIR_ITER *dirState) {
    r->_errno = EINVAL;
    return -1;
}

int usbfs_dev_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat) {
    usbfs_dev_dir *d = (usbfs_dev_dir*)(dirState->dirStruct);
    Result rc;
    u64 type, size;

    memset(filename, 0, NAME_MAX);


    rc = usbFsReadDir(d->dirid, &type, &size, filename, NAME_MAX);
    if (rc == 0x68A) {
        // there are no more entries; ENOENT signals end-of-directory
        r->_errno = ENOENT;
        return -1;
    }

    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    filestat->st_ino = 0;
    filestat->st_mode = type;
    filestat->st_size = size;

    return 0;
}

int usbfs_dev_dirclose(struct _reent *r, DIR_ITER *dirState) {
    usbfs_dev_dir *d = (usbfs_dev_dir*)(dirState->dirStruct);
    Result rc;

    rc = usbFsCloseDir(d->dirid);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}


static int usbfs_dev_rmdir(struct _reent *r, const char *path) {
    char* pathAtColon = strchr(path, ':');
    if (pathAtColon) {
        path = pathAtColon + 1;
    }

    Result rc = usbFsDeleteDir(path);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    return 0;
}

int usbfs_dev_statvfs(struct _reent *r, const char *path, struct statvfs *buf) {
    Result rc;
    u64 freespace, totalspace;

    rc = usbFsStatFilesystem(&totalspace, &freespace);
    if (R_FAILED(rc)) {
        r->_errno = EINVAL;
        return -1;
    }

    buf->f_bsize   = 1;
    buf->f_frsize  = 1;
    buf->f_blocks  = totalspace;
    buf->f_bfree   = freespace;
    buf->f_bavail  = freespace;
    buf->f_files   = 0;
    buf->f_ffree   = 0;
    buf->f_favail  = 0;
    buf->f_fsid    = 0;
    buf->f_flag    = ST_NOSUID;
    buf->f_namemax = 0;
    return 0;
}

//#endif