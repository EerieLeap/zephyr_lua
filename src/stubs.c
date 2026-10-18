#include <errno.h>
#include <stdio.h>
#include <sys/times.h>
#include <zephyr/fs/fs.h>

/* libc entry points Lua pulls in that the Zephyr targets do not provide, as weak stubs that
 * report ENOTSUP. Even with the io and os libraries left out, ltablib.c seeds the sort pivot
 * with clock() (l_randomizePivot), and picolibc's clock() calls times(); a -1 result only
 * feeds the seed. The newlib stubs cover liolib/loslib if they are ever enabled there.
 *
 * The base library's loadfile and dofile call luaL_loadfilex, which opens the file with
 * picolibc's fopen/freopen; those are built over the POSIX file calls, which Zephyr defines only
 * with CONFIG_POSIX_DEVICE_IO (open, close, read, write) and CONFIG_POSIX_FD_MGMT (lseek). The
 * weak definitions below keep the link whole where those options are off, so loadfile fails with
 * "Operation not supported"; where they are on, Zephyr's strong definitions win.
 */

#ifdef CONFIG_PICOLIBC
    #include <fcntl.h>
    #include <unistd.h>

clock_t times(struct tms* tms) __attribute__((weak));
clock_t times(struct tms* tms) {
    errno = ENOTSUP;
    return (clock_t)-1;
}

int open(const char* pathname, int flags, ...) __attribute__((weak));
int open(const char* pathname, int flags, ...) {
    (void)pathname;
    (void)flags;
    errno = ENOTSUP;
    return -1;
}

int close(int fd) __attribute__((weak));
int close(int fd) {
    (void)fd;
    errno = ENOTSUP;
    return -1;
}

ssize_t read(int fd, void* buf, size_t nbyte) __attribute__((weak));
ssize_t read(int fd, void* buf, size_t nbyte) {
    (void)fd;
    (void)buf;
    (void)nbyte;
    errno = ENOTSUP;
    return -1;
}

ssize_t write(int fd, const void* buf, size_t nbyte) __attribute__((weak));
ssize_t write(int fd, const void* buf, size_t nbyte) {
    (void)fd;
    (void)buf;
    (void)nbyte;
    errno = ENOTSUP;
    return -1;
}

off_t lseek(int fd, off_t offset, int whence) __attribute__((weak));
off_t lseek(int fd, off_t offset, int whence) {
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ENOTSUP;
    return (off_t)-1;
}
#endif

#ifdef CONFIG_NEWLIB_LIBC
int _link(const char* oldpath, const char* newpath) __attribute__((weak));
int _link(const char* oldpath, const char* newpath) {
    errno = ENOTSUP;
    return -1;
}

int _unlink(const char* pathname) __attribute__((weak));
int _unlink(const char* pathname) {
    errno = ENOTSUP;
    return -1;
}

int _open(const char* pathname) __attribute__((weak));
int _open(const char* pathname) {
    errno = ENOTSUP;
    return -1;
}

int _times(const char* pathname) __attribute__((weak));
int _times(const char* pathname) {
    errno = ENOTSUP;
    return -1;
}
#endif
