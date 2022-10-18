#define _GNU_SOURCE
#include <sys/mman.h> 		/* MMap */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* gettid */
#include <stdio.h>		/* snprintf */


#define PATH_LEN	256
#define MMAP_SIZE	(512 << 20) //512MB

static __thread int tid = 0;
static __thread unsigned long addr = 0;
static __thread char buffer[PATH_LEN];

unsigned long get_addr(void) 
{
	int fd = 0;

	if (tid)
		return addr;

	tid = gettid();
	snprintf(buffer, PATH_LEN, "/mnt/ramfs/%d", tid);

	fd = open(buffer, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG);

	if (fd < 0) {
		printf("Failed to create File %s!\n", buffer);
	}
	
	/* Consider HTLB MAP_HUGETLB Documentation/admin-guide/mm/hugetlbpage.rst */
	addr = (unsigned long)mmap(0, MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED|MAP_POPULATE, fd, 0);
	if (addr < 0)
		printf("Failed to MAP File %s [%d]!\n", buffer, fd);
}
