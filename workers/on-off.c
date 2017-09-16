#define _GNU_SOURCE
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>
#include <inttypes.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <linux/unistd.h>
#include <linux/magic.h>

#ifdef __i386__
#ifndef __NR_sched_setattr
#define __NR_sched_setattr		351
#endif
#ifndef __NR_sched_getattr
#define __NR_sched_getattr		352
#endif
#ifndef __NR_getcpu
#define __NR_getcpu			309
#endif
#else /* x86_64 */
#ifndef __NR_sched_setattr
#define __NR_sched_setattr		314
#endif
#ifndef __NR_sched_getattr
#define __NR_sched_getattr		315
#endif
#ifndef __NR_getcpu
#define __NR_getcpu			309
#endif
#endif /* i386 or x86_64 */
#ifndef SCHED_DEADLINE
#define SCHED_DEADLINE		6
#endif

#define _STR(x) #x
#define STR(x) _STR(x)
#ifndef MAXPATH
#define MAXPATH 1024
#endif

#define gettid() syscall(__NR_gettid)
#define sched_setattr(pid, attr, flags) syscall(__NR_sched_setattr, pid, attr, flags)
#define sched_getattr(pid, attr, size, flags) syscall(__NR_sched_getattr, pid, attr, size, flags)
#define getcpu(cpup, nodep, unused) syscall(__NR_getcpu, cpup, nodep, unused)

struct sched_attr {
	uint32_t size;

	uint32_t sched_policy;
	uint64_t sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority;

	/* SCHED_DEADLINE */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;
};

static uint64_t get_time_us(void)
{
	struct timespec ts;
	uint64_t time;

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	time = ts.tv_sec * 1000000;
	time += ts.tv_nsec / 1000;

	return time;
}

int main ()
{
	struct sched_attr attr;
	attr.size = sizeof(struct sched_attr);
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = 30000000;
	attr.sched_period = 100000000;
	attr.sched_deadline = attr.sched_period;

	if (sched_setattr(gettid(), &attr, 0))
		perror("sched_setattr()");

	return 0;
}
