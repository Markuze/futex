#define _GNU_SOURCE
#include <sys/mman.h> 		/* MMap */
#include <fcntl.h>		/* open */
#include <unistd.h>		/* gettid */
#include <stdio.h>		/* snprintf */
#include <errno.h>
#include <string.h>
#include <sched.h>

#define PATH_LEN	256
#define MMAP_SIZE	(512 << 20) //512MB

struct per_cpu_addr {
	union {
		char ___pad[64];
		struct {
			unsigned long addr;
			unsigned long offset;
		};
	};
} __attribute__ ((aligned (64)));

extern int errorno;

static struct per_cpu_addr write_addr[64];
static __thread int cpuid = -1;

unsigned long get_per_cpu_addr(int cpu)
{
	int fd = 0;
	unsigned long addr = 0;
	char buffer[PATH_LEN];

	snprintf(buffer, PATH_LEN, "/mnt/ramfs/core_%d", cpu);

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
	if (addr <= 0)
		printf("Failed to MAP File %s [%d]!, with %d\n", buffer, fd, errno);
	return addr;
}

void write_buffer(const char* str, unsigned int size)
{
	unsigned long  offset = 0;
	/*
	 * This is a hopefull heuristic that a thread will remain on its own CPU.
	 * We avoid getcpu on each call but  Add_fetch is a must in any case.
	 * */
	if (cpuid < 0) {
		unsigned int cpu, node;
		getcpu(&cpu, &node);

		if (write_addr[cpu].addr == 0) {
			unsigned long addr = get_per_cpu_addr(cpu);
			if (addr > 0) {
				printf("Returning %llx on %d\n", addr, cpu);
				//open guarantees atomicity here no need to cmpxhg
				write_addr[cpu].addr = addr;
			}
		}
		__atomic_thread_fence(__ATOMIC_RELEASE);
		cpuid = cpu;
	}

	offset = __atomic_add_fetch(&write_addr[cpuid].offset, size, __ATOMIC_RELAXED);
	if (offset < MMAP_SIZE) {
		printf("[%d]> %llx [%lu, %u]: %s", cpuid, (write_addr[cpuid].addr + offset -size), offset, size, str);
		snprintf((char *)(write_addr[cpuid].addr + offset -size), size + 1,"%s", str);
	}
}
