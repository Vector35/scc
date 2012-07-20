// Copyright (c) 2012 Rusty Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef __LIBC__FREEBSD_PROCESS_H__
#define __LIBC__FREEBSD_PROCESS_H__

#include "runtime/posix/time.h"

#define SIGHUP     1
#define SIGINT     2
#define SIGQUIT    3
#define SIGILL     4
#define SIGTRAP    5
#define SIGABRT    6
#define SIGIOT     6
#define SIGEMT     7
#define SIGFPE     8
#define SIGKILL    9
#define SIGBUS     10
#define SIGSEGV    11
#define SIGSYS     12
#define SIGPIPE    13
#define SIGALRM    14
#define SIGTERM    15
#define SIGURG     16
#define SIGSTOP    17
#define SIGTSTP    18
#define SIGCONT    19
#define SIGCHLD    20
#define SIGTTIN    21
#define SIGTTOU    22
#define SIGIO      23
#define SIGXCPU    24
#define SIGXFSZ    25
#define SIGVTALRM  26
#define SIGPROF    27
#define SIGWINCH   28
#define SIGINFO    29
#define SIGUSR1    30
#define SIGUSR2    31
#define SIGTHR     32
#define SIGLWP     32

#define SIG_DFL     ((sig_t)0)
#define SIG_IGN     ((sig_t)1)
#define SIG_HOLD    ((sig_t)3)

#define SA_ONSTACK    1
#define SA_RESTART    2
#define SA_RESETHAND  4
#define SA_NOCLDSTOP  8
#define SA_NODEFER    0x10
#define SA_NOCLDWAIT  0x20
#define SA_SIGINFO    0x40

#define	CTL_UNSPEC    0  // unused
#define	CTL_KERN      1  // "high kernel": proc, limits
#define	CTL_VM        2  // virtual memory
#define	CTL_VFS       3  // filesystem, mount type is next
#define	CTL_NET       4  // network, see socket.h
#define	CTL_DEBUG     5  // debugging parameters
#define	CTL_HW        6  // generic cpu/io
#define	CTL_MACHDEP   7  // machine dependent
#define	CTL_USER      8  // user-level
#define	CTL_P1003_1B  9  // POSIX 1003.1B
#define	CTL_MAXID     10 // number of valid top-level ids

#define	KERN_OSTYPE            1  // string: system version
#define	KERN_OSRELEASE         2  // string: system release
#define	KERN_OSREV             3  // int: system revision
#define	KERN_VERSION	 	   4  // string: compile time info
#define	KERN_MAXVNODES         5  // int: max vnodes
#define	KERN_MAXPROC           6  // int: max processes
#define	KERN_MAXFILES          7  // int: max open files
#define	KERN_ARGMAX            8  // int: max arguments to exec
#define	KERN_SECURELVL         9  // int: system security level
#define	KERN_HOSTNAME         10  // string: hostname
#define	KERN_HOSTID           11  // int: host identifier
#define	KERN_CLOCKRATE        12  // struct: struct clockrate
#define	KERN_VNODE            13  // struct: vnode structures
#define	KERN_PROC             14  // struct: process entries
#define	KERN_FILE             15  // struct: file entries
#define	KERN_PROF             16  // node: kernel profiling info
#define	KERN_POSIX1           17  // int: POSIX.1 version
#define	KERN_NGROUPS          18  // int: # of supplemental group ids
#define	KERN_JOB_CONTROL      19  // int: is job control available
#define	KERN_SAVED_IDS        20  // int: saved set-user/group-ID
#define	KERN_BOOTTIME         21  // struct: time kernel was booted
#define KERN_NISDOMAINNAME    22  // string: YP domain name
#define KERN_UPDATEINTERVAL   23  // int: update process sleep time
#define KERN_OSRELDATE        24  // int: kernel release date
#define KERN_NTP_PLL          25  // node: NTP PLL control
#define	KERN_BOOTFILE         26  // string: name of booted kernel
#define	KERN_MAXFILESPERPROC  27  // int: max open files per proc
#define	KERN_MAXPROCPERUID    28  // int: max processes per uid
#define KERN_DUMPDEV          29  // struct cdev *: device to dump on
#define	KERN_IPC              30  // node: anything related to IPC
#define	KERN_DUMMY            31  // unused
#define	KERN_PS_STRINGS       32  // int: address of PS_STRINGS
#define	KERN_USRSTACK         33  // int: address of USRSTACK
#define	KERN_LOGSIGEXIT       34  // int: do we log sigexit procs?
#define	KERN_IOV_MAX          35  // int: value of UIO_MAXIOV
#define	KERN_HOSTUUID         36  // string: host UUID identifier
#define	KERN_ARND             37  // int: from arc4rand()
#define	KERN_MAXID            38  // number of valid kern ids

#define KERN_PROC_ALL          0  // everything
#define	KERN_PROC_PID          1  // by process id
#define	KERN_PROC_PGRP         2  // by process group id
#define	KERN_PROC_SESSION      3  // by session of pid
#define	KERN_PROC_TTY          4  // by controlling tty
#define	KERN_PROC_UID          5  // by effective uid
#define	KERN_PROC_RUID         6  // by real uid
#define	KERN_PROC_ARGS         7  // get/set arguments/proctitle
#define	KERN_PROC_PROC         8  // only return procs
#define	KERN_PROC_SV_NAME      9  // get syscall vector name
#define	KERN_PROC_RGID        10  // by real group id
#define	KERN_PROC_GID         11  // by effective group id
#define	KERN_PROC_PATHNAME    12  // path to executable
#define	KERN_PROC_OVMMAP      13  // Old VM map entries for process
#define	KERN_PROC_OFILEDESC   14  // Old file descriptors for process
#define	KERN_PROC_KSTACK      15  // Kernel stacks for process
#define	KERN_PROC_INC_THREAD  16
#define	KERN_PROC_VMMAP       32  // VM map entries for process
#define	KERN_PROC_FILEDESC    33  // File descriptors for process
#define	KERN_PROC_GROUPS      34  // process groups

typedef void (*sig_t)(int);

typedef struct __sigset_t
{
	uint32_t __bits[4];
} sigset_t;

typedef struct
{
	int si_signo;
	int si_errno;
	int si_code;
	pid_t si_pid;
	uid_t si_uid;
	int si_status;
	void* si_addr;
} siginfo_t;

struct sigaction
{
	union
	{
		sig_t sa_handler;
		void (*sa_sigaction)(int, siginfo_t*, void*);
	};
	int sa_flags;
	sigset_t sa_mask;
};

struct priority
{
	uint8_t pri_class;
	uint8_t pri_level;
	uint8_t pri_native;
	uint8_t pri_user;
};

struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
	ssize_t ru_maxrss;
	ssize_t ru_ixrss;
	ssize_t ru_idrss;
	ssize_t ru_isrss;
	ssize_t ru_minflt;
	ssize_t ru_majflt;
	ssize_t ru_nswap;
	ssize_t ru_inblock;
	ssize_t ru_oublock;
	ssize_t ru_msgsnd;
	ssize_t ru_msgrcv;
	ssize_t ru_nsignals;
	ssize_t ru_nvcsw;
	ssize_t ru_nivcsw;
};

struct kinfo_proc
{
	int ki_structsize;
	int ki_layout;
	void* ki_args;
	void* ki_paddr;
	void* ki_addr;
	void* ki_tracep;
	void* ki_textvp;
	void* ki_fd;
	void* ki_vmspace;
	void* ki_wchan;
	pid_t ki_pid;
	pid_t ki_ppid;
	pid_t ki_pgid;
	pid_t ki_tpgid;
	pid_t ki_sid;
	pid_t ki_tsid;
	short ki_jobc;
	short ki_spare_short1;
	uint32_t ki_tdev;
	sigset_t ki_siglist;
	sigset_t ki_sigmask;
	sigset_t ki_sigignore;
	sigset_t ki_sigcatch;
	uid_t ki_uid;
	uid_t ki_ruid;
	uid_t ki_svuid;
	gid_t ki_rgid;
	gid_t ki_svgid;
	short ki_ngroups;
	short ki_spare_short2;
	gid_t ki_groups[16];
	size_t ki_size;
	ssize_t ki_rssize;
	ssize_t ki_swrss;
	ssize_t ki_tsize;
	ssize_t ki_dsize;
	ssize_t ki_ssize;
	uint16_t ki_xstat;
	uint16_t ki_acflag;
	uint32_t ki_pctcpu;
	uint32_t ki_estcpu;
	uint32_t ki_slptime;
	uint32_t ki_swtime;
	int ki_spareint1;
	uint64_t ki_runtime;
	struct timeval ki_start;
	struct timeval ki_childtime;
	ssize_t ki_flag;
	ssize_t ki_kiflag;
	int ki_traceflag;
	char ki_stat;
	int8_t ki_nice;
	char ki_lock;
	char ki_rqindex;
	uint8_t ki_oncpu;
	uint8_t ki_lastcpu;
	char ki_tdname[17];
	char ki_wmesg[9];
	char ki_login[18];
	char ki_lockname[9];
	char ki_comm[20];
	char ki_emul[17];
	char ki_loginclass[18];
	char ki_sparestrings[50];
	int ki_spareints[9];
	uint32_t ki_cr_flags;
	int ki_jid;
	int ki_numthreads;
	pid_t ki_tid;
	struct priority ki_pri;
	struct rusage ki_rusage;
	struct rusage ki_rusage_ch;
	void* ki_pcb;
	void* ki_kstack;
	void* ki_udata;
	void* ki_tdaddr;
	void* ki_spareptrs[6];
	size_t ki_sparelongs[12];
	ssize_t ki_sflag;
	ssize_t ki_tdflags;
};

int sysctl(const int* name, size_t namelen, void* oldp, size_t* oldlenp, const void* newp, size_t newlen);

sig_t signal(int sig, sig_t func);
int sigaction(int sig, const struct sigaction* act, struct sigaction* old);

#endif

