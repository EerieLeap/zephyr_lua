#include <stdio.h>
#include <errno.h>
#include <sys/times.h>
#include <zephyr/fs/fs.h>

/* libc entry points Lua pulls in that the Zephyr targets do not provide, as weak stubs that
 * report ENOTSUP. Even with the io and os libraries left out, ltablib.c seeds the sort pivot
 * with clock() (l_randomizePivot), and picolibc's clock() calls times(); a -1 result only
 * feeds the seed. The newlib stubs cover liolib/loslib if they are ever enabled there.
 */


#ifdef CONFIG_PICOLIBC
clock_t times(struct tms* tms) __attribute__((weak));
clock_t times(struct tms* tms)
{
	errno = ENOTSUP;
	return (clock_t) -1;

}
#endif

#ifdef CONFIG_NEWLIB_LIBC
int _link(const char *oldpath, const char *newpath) __attribute__((weak));
int _link(const char *oldpath, const char *newpath)
{
	errno = ENOTSUP;
	return -1;
}

int _unlink(const char *pathname) __attribute__((weak));
int _unlink(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}

int _open(const char *pathname) __attribute__((weak));
int _open(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}

int _times(const char *pathname) __attribute__((weak));
int _times(const char *pathname)
{
	errno = ENOTSUP;
	return -1;
}
#endif
