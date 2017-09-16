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
#include <stdbool.h>

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

uint64_t get_time_us(void)
{
	struct timespec ts;
	uint64_t time;

	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	time = ts.tv_sec * 1000000;
	time += ts.tv_nsec / 1000;

	return time;
}

bool init_rt(void)
{
	if (mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
		perror("mlockall");
		return false;
	}

	return true;
}

void die(const char *msg)
{
	write(STDERR_FILENO, msg, strlen(msg));
	exit(EXIT_FAILURE);
}

int drop_priviliges(void)
{
	int id = 1000;
	if (getuid() == 0) {
		/* process is running as root, drop privileges */
		if (setgid(id) != 0) {
			perror("setgid");
			return -1;

		}
		if (setuid(id) != 0) {
			perror("setuid");
			return -1;
		}
	}

	if (chdir("/") != 0) {
		perror("chdir");
		return -1;
	}

	// check if we successfully dropped the root privileges
	if (setuid(0) == 0 || seteuid(0) == 0) {
		printf("could not drop root privileges!\n");
		return -1;
	}


	return 0;
}

int main()
{
	bool ok;
	int ret;
	struct sched_attr attr;

	ret = sched_getattr(0, &attr, sizeof(attr), 0);
	if (ret < 0) {
		perror("sched_getattr");
		die("get attr\n");;
	}

	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = 3000000;
	attr.sched_period = 10000000;
	attr.sched_deadline = attr.sched_period;

	ok = init_rt();
	if (!ok) {
		die("Failed to initialize RT subsystem, bye\n");
	}

	if (sched_setattr(0, &attr, 0))
		perror("sched_setattr()");

	//drop_root_privileges();
	drop_priviliges();

	printf("uid: %d\n", getuid());

	return 0;
}
