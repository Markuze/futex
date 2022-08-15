/*
 * File: soft_atimes.c
 * Author: D.J. Capelis
 *
 * Compile:
 * gcc -fPIC -c -o soft_atimes.o soft_atimes.c
 * gcc -shared -o soft_atimes.so soft_atimes.o -ldl
 *
 * Use:
 * LD_PRELOAD="./soft_atimes.so" command
 *
 * Copyright 2007 Regents of the University of California
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#define _FCNTL_H
#include <sys/types.h>
#include <bits/fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <execinfo.h>
#include <linux/futex.h>      /* Definition of FUTEX_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */
//#include <unistd.h>

extern int errorno;

int __thread (*_open)(const char * pathname, int flags, ...) = NULL;
int __thread (*_open64)(const char * pathname, int flags, ...) = NULL;

long (*_syscall)(long number, ...) = NULL;

/* TODO: TLS */ void *buffer[1024];
long syscall(long sysnum, u_int32_t *uaddr, int futex_op, u_int32_t val,
		const struct timespec *timeout,   /* or: u_int32_t val2 */
		u_int32_t *uaddr2, u_int32_t val3) 
{
	char **strings = NULL;
	int nptrs = backtrace(buffer, 1024);

	if (_syscall == NULL)
		_syscall = (long (*)(long number, ...)) dlsym(RTLD_NEXT, "syscall");

	strings = backtrace_symbols(buffer, nptrs);
	for (int j = 0; j < nptrs; j++)
		printf("%s\n", strings[j]);

        free(strings);

	printf("syscall :) [%ld] %s\n", sysnum, (sysnum == SYS_futex) ? "Futex": "Other");
	return _syscall(sysnum, uaddr, futex_op, val, timeout, uaddr2, val3);
}

int open(const char * pathname, int flags, mode_t mode)
{
    if (NULL == _open) {
        _open = (int (*)(const char * pathname, int flags, ...)) dlsym(RTLD_NEXT, "open");
    }
    if(flags & O_CREAT)
        return _open(pathname, flags | O_NOATIME, mode);
    else
        return _open(pathname, flags | O_NOATIME, 0);
}

int open64(const char * pathname, int flags, mode_t mode)
{
    if (NULL == _open64) {
        _open64 = (int (*)(const char * pathname, int flags, ...)) dlsym(RTLD_NEXT, "open64");
    }
    if(flags & O_CREAT)
        return _open64(pathname, flags | O_NOATIME, mode);
    else
        return _open64(pathname, flags | O_NOATIME, 0);
}
