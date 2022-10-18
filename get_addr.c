#define _GNU_SOURCE
#include <sys/mman.h> 		/* MMap */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* gettid */
#include <stdio.h>		/* snprintf */
#include <errno.h>
#include <string.h>

#define PATH_LEN	256
#define MMAP_SIZE	(512 << 20) //512MB

extern int errorno;

static __thread int tid = 0;
static __thread unsigned long addr = 0;
static __thread char buffer[PATH_LEN];

/* use this instead -- chnage __thread to array... __cahce_aligned(?) */
#if 0
       #define _GNU_SOURCE             /* See feature_test_macros(7) */
       #include <sched.h>

       int getcpu(unsigned int *cpu, unsigned int *node);
#endif

unsigned long get_addr(void) 
{
	int fd = 0;

	if (tid)
		return addr;

	tid = gettid();
	snprintf(buffer, PATH_LEN, "/mnt/ramfs/%d", tid);

	fd = open(buffer, O_RDWR|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		printf("Failed to create File %s, with %s %s!\n", buffer, errno);
		return 0;
	}

	if (fallocate(fd, 0, 0, MMAP_SIZE) < 0) {
		printf("Failed to fallocate File %s, with %s !\n", buffer, strerror(errno));
		return 0;
	}

	/* Consider HTLB MAP_HUGETLB Documentation/admin-guide/mm/hugetlbpage.rst */
	addr = (unsigned long)mmap(0, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED|MAP_POPULATE, fd, 0);
	if (addr < 0)
		printf("Failed to MAP File %s [%d]!, with %d\n", buffer, fd, errno);
	return addr;
}
