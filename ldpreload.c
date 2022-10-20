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
#include <stdlib.h>
//#include <unistd.h>
#include <execinfo.h>
#include <linux/futex.h>      /* Definition of FUTEX_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */

#include "get_addr.h"

extern int errorno;

static long (*_syscall)(long number, ...) = NULL;


long syscall(long sysnum, u_int32_t *uaddr, int futex_op, u_int32_t val,
		const struct timespec *timeout,   /* or: u_int32_t val2 */
		u_int32_t *uaddr2, u_int32_t val3) 
{
	void *buffer[1024];
	char scratch[256];
	unsigned long offset = 0;
	char **strings = NULL;
	int nptrs = backtrace(buffer, 1024);

	if (_syscall == NULL)
		_syscall = (long (*)(long number, ...)) dlsym(RTLD_NEXT, "syscall");

	strings = backtrace_symbols(buffer, nptrs);

	for (int j = 0; j < nptrs; j++) {
		offset = snprintf(scratch, 256,"\t[%d]%s\n", j, strings[j]);
		write_buffer(scratch, offset);
	}


        free(strings);

	printf("\nsyscall :) [%ld] %s\n", sysnum, (sysnum == SYS_futex) ? "Futex": "Other");
	return _syscall(sysnum, uaddr, futex_op, val, timeout, uaddr2, val3);
}
