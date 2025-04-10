// Copyright (c) 2014 Rusty Wagner
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

#ifndef __LIBC__SYSCALL_H__
#define __LIBC__SYSCALL_H__

#define SYS_Linux			4000
#define SYS_syscall			(SYS_Linux +	0)
#define SYS_exit			(SYS_Linux +	1)
#define SYS_fork			(SYS_Linux +	2)
#define SYS_read			(SYS_Linux +	3)
#define SYS_write			(SYS_Linux +	4)
#define SYS_open			(SYS_Linux +	5)
#define SYS_close			(SYS_Linux +	6)
#define SYS_waitpid			(SYS_Linux +	7)
#define SYS_creat			(SYS_Linux +	8)
#define SYS_link			(SYS_Linux +	9)
#define SYS_unlink			(SYS_Linux +  10)
#define SYS_execve			(SYS_Linux +  11)
#define SYS_chdir			(SYS_Linux +  12)
#define SYS_time			(SYS_Linux +  13)
#define SYS_mknod			(SYS_Linux +  14)
#define SYS_chmod			(SYS_Linux +  15)
#define SYS_lchown			(SYS_Linux +  16)
#define SYS_break			(SYS_Linux +  17)
#define SYS_unused18			(SYS_Linux +  18)
#define SYS_lseek			(SYS_Linux +  19)
#define SYS_getpid			(SYS_Linux +  20)
#define SYS_mount			(SYS_Linux +  21)
#define SYS_umount			(SYS_Linux +  22)
#define SYS_setuid			(SYS_Linux +  23)
#define SYS_getuid			(SYS_Linux +  24)
#define SYS_stime			(SYS_Linux +  25)
#define SYS_ptrace			(SYS_Linux +  26)
#define SYS_alarm			(SYS_Linux +  27)
#define SYS_unused28			(SYS_Linux +  28)
#define SYS_pause			(SYS_Linux +  29)
#define SYS_utime			(SYS_Linux +  30)
#define SYS_stty			(SYS_Linux +  31)
#define SYS_gtty			(SYS_Linux +  32)
#define SYS_access			(SYS_Linux +  33)
#define SYS_nice			(SYS_Linux +  34)
#define SYS_ftime			(SYS_Linux +  35)
#define SYS_sync			(SYS_Linux +  36)
#define SYS_kill			(SYS_Linux +  37)
#define SYS_rename			(SYS_Linux +  38)
#define SYS_mkdir			(SYS_Linux +  39)
#define SYS_rmdir			(SYS_Linux +  40)
#define SYS_dup			(SYS_Linux +  41)
#define SYS_pipe			(SYS_Linux +  42)
#define SYS_times			(SYS_Linux +  43)
#define SYS_prof			(SYS_Linux +  44)
#define SYS_brk			(SYS_Linux +  45)
#define SYS_setgid			(SYS_Linux +  46)
#define SYS_getgid			(SYS_Linux +  47)
#define SYS_signal			(SYS_Linux +  48)
#define SYS_geteuid			(SYS_Linux +  49)
#define SYS_getegid			(SYS_Linux +  50)
#define SYS_acct			(SYS_Linux +  51)
#define SYS_umount2			(SYS_Linux +  52)
#define SYS_lock			(SYS_Linux +  53)
#define SYS_ioctl			(SYS_Linux +  54)
#define SYS_fcntl			(SYS_Linux +  55)
#define SYS_mpx			(SYS_Linux +  56)
#define SYS_setpgid			(SYS_Linux +  57)
#define SYS_ulimit			(SYS_Linux +  58)
#define SYS_unused59			(SYS_Linux +  59)
#define SYS_umask			(SYS_Linux +  60)
#define SYS_chroot			(SYS_Linux +  61)
#define SYS_ustat			(SYS_Linux +  62)
#define SYS_dup2			(SYS_Linux +  63)
#define SYS_getppid			(SYS_Linux +  64)
#define SYS_getpgrp			(SYS_Linux +  65)
#define SYS_setsid			(SYS_Linux +  66)
#define SYS_sigaction			(SYS_Linux +  67)
#define SYS_sgetmask			(SYS_Linux +  68)
#define SYS_ssetmask			(SYS_Linux +  69)
#define SYS_setreuid			(SYS_Linux +  70)
#define SYS_setregid			(SYS_Linux +  71)
#define SYS_sigsuspend			(SYS_Linux +  72)
#define SYS_sigpending			(SYS_Linux +  73)
#define SYS_sethostname		(SYS_Linux +  74)
#define SYS_setrlimit			(SYS_Linux +  75)
#define SYS_getrlimit			(SYS_Linux +  76)
#define SYS_getrusage			(SYS_Linux +  77)
#define SYS_gettimeofday		(SYS_Linux +  78)
#define SYS_settimeofday		(SYS_Linux +  79)
#define SYS_getgroups			(SYS_Linux +  80)
#define SYS_setgroups			(SYS_Linux +  81)
#define SYS_reserved82			(SYS_Linux +  82)
#define SYS_symlink			(SYS_Linux +  83)
#define SYS_unused84			(SYS_Linux +  84)
#define SYS_readlink			(SYS_Linux +  85)
#define SYS_uselib			(SYS_Linux +  86)
#define SYS_swapon			(SYS_Linux +  87)
#define SYS_reboot			(SYS_Linux +  88)
#define SYS_readdir			(SYS_Linux +  89)
#define SYS_mmap			(SYS_Linux +  90)
#define SYS_munmap			(SYS_Linux +  91)
#define SYS_truncate			(SYS_Linux +  92)
#define SYS_ftruncate			(SYS_Linux +  93)
#define SYS_fchmod			(SYS_Linux +  94)
#define SYS_fchown			(SYS_Linux +  95)
#define SYS_getpriority		(SYS_Linux +  96)
#define SYS_setpriority		(SYS_Linux +  97)
#define SYS_profil			(SYS_Linux +  98)
#define SYS_statfs			(SYS_Linux +  99)
#define SYS_fstatfs			(SYS_Linux + 100)
#define SYS_ioperm			(SYS_Linux + 101)
#define SYS_socketcall			(SYS_Linux + 102)
#define SYS_syslog			(SYS_Linux + 103)
#define SYS_setitimer			(SYS_Linux + 104)
#define SYS_getitimer			(SYS_Linux + 105)
#define SYS_stat			(SYS_Linux + 106)
#define SYS_lstat			(SYS_Linux + 107)
#define SYS_fstat			(SYS_Linux + 108)
#define SYS_unused109			(SYS_Linux + 109)
#define SYS_iopl			(SYS_Linux + 110)
#define SYS_vhangup			(SYS_Linux + 111)
#define SYS_idle			(SYS_Linux + 112)
#define SYS_vm86			(SYS_Linux + 113)
#define SYS_wait4			(SYS_Linux + 114)
#define SYS_swapoff			(SYS_Linux + 115)
#define SYS_sysinfo			(SYS_Linux + 116)
#define SYS_ipc			(SYS_Linux + 117)
#define SYS_fsync			(SYS_Linux + 118)
#define SYS_sigreturn			(SYS_Linux + 119)
#define SYS_clone			(SYS_Linux + 120)
#define SYS_setdomainname		(SYS_Linux + 121)
#define SYS_uname			(SYS_Linux + 122)
#define SYS_modify_ldt			(SYS_Linux + 123)
#define SYS_adjtimex			(SYS_Linux + 124)
#define SYS_mprotect			(SYS_Linux + 125)
#define SYS_sigprocmask		(SYS_Linux + 126)
#define SYS_create_module		(SYS_Linux + 127)
#define SYS_init_module		(SYS_Linux + 128)
#define SYS_delete_module		(SYS_Linux + 129)
#define SYS_get_kernel_syms		(SYS_Linux + 130)
#define SYS_quotactl			(SYS_Linux + 131)
#define SYS_getpgid			(SYS_Linux + 132)
#define SYS_fchdir			(SYS_Linux + 133)
#define SYS_bdflush			(SYS_Linux + 134)
#define SYS_sysfs			(SYS_Linux + 135)
#define SYS_personality		(SYS_Linux + 136)
#define SYS_afs_syscall		(SYS_Linux + 137) /* Syscall for Andrew File System */
#define SYS_setfsuid			(SYS_Linux + 138)
#define SYS_setfsgid			(SYS_Linux + 139)
#define SYS__llseek			(SYS_Linux + 140)
#define SYS_getdents			(SYS_Linux + 141)
#define SYS__newselect			(SYS_Linux + 142)
#define SYS_flock			(SYS_Linux + 143)
#define SYS_msync			(SYS_Linux + 144)
#define SYS_readv			(SYS_Linux + 145)
#define SYS_writev			(SYS_Linux + 146)
#define SYS_cacheflush			(SYS_Linux + 147)
#define SYS_cachectl			(SYS_Linux + 148)
#define SYS_sysmips			(SYS_Linux + 149)
#define SYS_unused150			(SYS_Linux + 150)
#define SYS_getsid			(SYS_Linux + 151)
#define SYS_fdatasync			(SYS_Linux + 152)
#define SYS__sysctl			(SYS_Linux + 153)
#define SYS_mlock			(SYS_Linux + 154)
#define SYS_munlock			(SYS_Linux + 155)
#define SYS_mlockall			(SYS_Linux + 156)
#define SYS_munlockall			(SYS_Linux + 157)
#define SYS_sched_setparam		(SYS_Linux + 158)
#define SYS_sched_getparam		(SYS_Linux + 159)
#define SYS_sched_setscheduler		(SYS_Linux + 160)
#define SYS_sched_getscheduler		(SYS_Linux + 161)
#define SYS_sched_yield		(SYS_Linux + 162)
#define SYS_sched_get_priority_max	(SYS_Linux + 163)
#define SYS_sched_get_priority_min	(SYS_Linux + 164)
#define SYS_sched_rr_get_interval	(SYS_Linux + 165)
#define SYS_nanosleep			(SYS_Linux + 166)
#define SYS_mremap			(SYS_Linux + 167)
#define SYS_accept			(SYS_Linux + 168)
#define SYS_bind			(SYS_Linux + 169)
#define SYS_connect			(SYS_Linux + 170)
#define SYS_getpeername		(SYS_Linux + 171)
#define SYS_getsockname		(SYS_Linux + 172)
#define SYS_getsockopt			(SYS_Linux + 173)
#define SYS_listen			(SYS_Linux + 174)
#define SYS_recv			(SYS_Linux + 175)
#define SYS_recvfrom			(SYS_Linux + 176)
#define SYS_recvmsg			(SYS_Linux + 177)
#define SYS_send			(SYS_Linux + 178)
#define SYS_sendmsg			(SYS_Linux + 179)
#define SYS_sendto			(SYS_Linux + 180)
#define SYS_setsockopt			(SYS_Linux + 181)
#define SYS_shutdown			(SYS_Linux + 182)
#define SYS_socket			(SYS_Linux + 183)
#define SYS_socketpair			(SYS_Linux + 184)
#define SYS_setresuid			(SYS_Linux + 185)
#define SYS_getresuid			(SYS_Linux + 186)
#define SYS_query_module		(SYS_Linux + 187)
#define SYS_poll			(SYS_Linux + 188)
#define SYS_nfsservctl			(SYS_Linux + 189)
#define SYS_setresgid			(SYS_Linux + 190)
#define SYS_getresgid			(SYS_Linux + 191)
#define SYS_prctl			(SYS_Linux + 192)
#define SYS_rt_sigreturn		(SYS_Linux + 193)
#define SYS_rt_sigaction		(SYS_Linux + 194)
#define SYS_rt_sigprocmask		(SYS_Linux + 195)
#define SYS_rt_sigpending		(SYS_Linux + 196)
#define SYS_rt_sigtimedwait		(SYS_Linux + 197)
#define SYS_rt_sigqueueinfo		(SYS_Linux + 198)
#define SYS_rt_sigsuspend		(SYS_Linux + 199)
#define SYS_pread64			(SYS_Linux + 200)
#define SYS_pwrite64			(SYS_Linux + 201)
#define SYS_chown			(SYS_Linux + 202)
#define SYS_getcwd			(SYS_Linux + 203)
#define SYS_capget			(SYS_Linux + 204)
#define SYS_capset			(SYS_Linux + 205)
#define SYS_sigaltstack		(SYS_Linux + 206)
#define SYS_sendfile			(SYS_Linux + 207)
#define SYS_getpmsg			(SYS_Linux + 208)
#define SYS_putpmsg			(SYS_Linux + 209)
#define SYS_mmap2			(SYS_Linux + 210)
#define SYS_truncate64			(SYS_Linux + 211)
#define SYS_ftruncate64		(SYS_Linux + 212)
#define SYS_stat64			(SYS_Linux + 213)
#define SYS_lstat64			(SYS_Linux + 214)
#define SYS_fstat64			(SYS_Linux + 215)
#define SYS_pivot_root			(SYS_Linux + 216)
#define SYS_mincore			(SYS_Linux + 217)
#define SYS_madvise			(SYS_Linux + 218)
#define SYS_getdents64			(SYS_Linux + 219)
#define SYS_fcntl64			(SYS_Linux + 220)
#define SYS_reserved221		(SYS_Linux + 221)
#define SYS_gettid			(SYS_Linux + 222)
#define SYS_readahead			(SYS_Linux + 223)
#define SYS_setxattr			(SYS_Linux + 224)
#define SYS_lsetxattr			(SYS_Linux + 225)
#define SYS_fsetxattr			(SYS_Linux + 226)
#define SYS_getxattr			(SYS_Linux + 227)
#define SYS_lgetxattr			(SYS_Linux + 228)
#define SYS_fgetxattr			(SYS_Linux + 229)
#define SYS_listxattr			(SYS_Linux + 230)
#define SYS_llistxattr			(SYS_Linux + 231)
#define SYS_flistxattr			(SYS_Linux + 232)
#define SYS_removexattr		(SYS_Linux + 233)
#define SYS_lremovexattr		(SYS_Linux + 234)
#define SYS_fremovexattr		(SYS_Linux + 235)
#define SYS_tkill			(SYS_Linux + 236)
#define SYS_sendfile64			(SYS_Linux + 237)
#define SYS_futex			(SYS_Linux + 238)
#define SYS_sched_setaffinity		(SYS_Linux + 239)
#define SYS_sched_getaffinity		(SYS_Linux + 240)
#define SYS_io_setup			(SYS_Linux + 241)
#define SYS_io_destroy			(SYS_Linux + 242)
#define SYS_io_getevents		(SYS_Linux + 243)
#define SYS_io_submit			(SYS_Linux + 244)
#define SYS_io_cancel			(SYS_Linux + 245)
#define SYS_exit_group			(SYS_Linux + 246)
#define SYS_lookup_dcookie		(SYS_Linux + 247)
#define SYS_epoll_create		(SYS_Linux + 248)
#define SYS_epoll_ctl			(SYS_Linux + 249)
#define SYS_epoll_wait			(SYS_Linux + 250)
#define SYS_remap_file_pages		(SYS_Linux + 251)
#define SYS_set_tid_address		(SYS_Linux + 252)
#define SYS_restart_syscall		(SYS_Linux + 253)
#define SYS_fadvise64			(SYS_Linux + 254)
#define SYS_statfs64			(SYS_Linux + 255)
#define SYS_fstatfs64			(SYS_Linux + 256)
#define SYS_timer_create		(SYS_Linux + 257)
#define SYS_timer_settime		(SYS_Linux + 258)
#define SYS_timer_gettime		(SYS_Linux + 259)
#define SYS_timer_getoverrun		(SYS_Linux + 260)
#define SYS_timer_delete		(SYS_Linux + 261)
#define SYS_clock_settime		(SYS_Linux + 262)
#define SYS_clock_gettime		(SYS_Linux + 263)
#define SYS_clock_getres		(SYS_Linux + 264)
#define SYS_clock_nanosleep		(SYS_Linux + 265)
#define SYS_tgkill			(SYS_Linux + 266)
#define SYS_utimes			(SYS_Linux + 267)
#define SYS_mbind			(SYS_Linux + 268)
#define SYS_get_mempolicy		(SYS_Linux + 269)
#define SYS_set_mempolicy		(SYS_Linux + 270)
#define SYS_mq_open			(SYS_Linux + 271)
#define SYS_mq_unlink			(SYS_Linux + 272)
#define SYS_mq_timedsend		(SYS_Linux + 273)
#define SYS_mq_timedreceive		(SYS_Linux + 274)
#define SYS_mq_notify			(SYS_Linux + 275)
#define SYS_mq_getsetattr		(SYS_Linux + 276)
#define SYS_vserver			(SYS_Linux + 277)
#define SYS_waitid			(SYS_Linux + 278)
/* #define SYS_sys_setaltroot		(SYS_Linux + 279) */
#define SYS_add_key			(SYS_Linux + 280)
#define SYS_request_key		(SYS_Linux + 281)
#define SYS_keyctl			(SYS_Linux + 282)
#define SYS_set_thread_area		(SYS_Linux + 283)
#define SYS_inotify_init		(SYS_Linux + 284)
#define SYS_inotify_add_watch		(SYS_Linux + 285)
#define SYS_inotify_rm_watch		(SYS_Linux + 286)
#define SYS_migrate_pages		(SYS_Linux + 287)
#define SYS_openat			(SYS_Linux + 288)
#define SYS_mkdirat			(SYS_Linux + 289)
#define SYS_mknodat			(SYS_Linux + 290)
#define SYS_fchownat			(SYS_Linux + 291)
#define SYS_futimesat			(SYS_Linux + 292)
#define SYS_fstatat64			(SYS_Linux + 293)
#define SYS_unlinkat			(SYS_Linux + 294)
#define SYS_renameat			(SYS_Linux + 295)
#define SYS_linkat			(SYS_Linux + 296)
#define SYS_symlinkat			(SYS_Linux + 297)
#define SYS_readlinkat			(SYS_Linux + 298)
#define SYS_fchmodat			(SYS_Linux + 299)
#define SYS_faccessat			(SYS_Linux + 300)
#define SYS_pselect6			(SYS_Linux + 301)
#define SYS_ppoll			(SYS_Linux + 302)
#define SYS_unshare			(SYS_Linux + 303)
#define SYS_splice			(SYS_Linux + 304)
#define SYS_sync_file_range		(SYS_Linux + 305)
#define SYS_tee			(SYS_Linux + 306)
#define SYS_vmsplice			(SYS_Linux + 307)
#define SYS_move_pages			(SYS_Linux + 308)
#define SYS_set_robust_list		(SYS_Linux + 309)
#define SYS_get_robust_list		(SYS_Linux + 310)
#define SYS_kexec_load			(SYS_Linux + 311)
#define SYS_getcpu			(SYS_Linux + 312)
#define SYS_epoll_pwait		(SYS_Linux + 313)
#define SYS_ioprio_set			(SYS_Linux + 314)
#define SYS_ioprio_get			(SYS_Linux + 315)
#define SYS_utimensat			(SYS_Linux + 316)
#define SYS_signalfd			(SYS_Linux + 317)
#define SYS_timerfd			(SYS_Linux + 318)
#define SYS_eventfd			(SYS_Linux + 319)
#define SYS_fallocate			(SYS_Linux + 320)
#define SYS_timerfd_create		(SYS_Linux + 321)
#define SYS_timerfd_gettime		(SYS_Linux + 322)
#define SYS_timerfd_settime		(SYS_Linux + 323)
#define SYS_signalfd4			(SYS_Linux + 324)
#define SYS_eventfd2			(SYS_Linux + 325)
#define SYS_epoll_create1		(SYS_Linux + 326)
#define SYS_dup3			(SYS_Linux + 327)
#define SYS_pipe2			(SYS_Linux + 328)
#define SYS_inotify_init1		(SYS_Linux + 329)
#define SYS_preadv			(SYS_Linux + 330)
#define SYS_pwritev			(SYS_Linux + 331)
#define SYS_rt_tgsigqueueinfo		(SYS_Linux + 332)
#define SYS_perf_event_open		(SYS_Linux + 333)
#define SYS_accept4			(SYS_Linux + 334)
#define SYS_recvmmsg			(SYS_Linux + 335)
#define SYS_fanotify_init		(SYS_Linux + 336)
#define SYS_fanotify_mark		(SYS_Linux + 337)
#define SYS_prlimit64			(SYS_Linux + 338)
#define SYS_name_to_handle_at		(SYS_Linux + 339)
#define SYS_open_by_handle_at		(SYS_Linux + 340)
#define SYS_clock_adjtime		(SYS_Linux + 341)
#define SYS_syncfs			(SYS_Linux + 342)
#define SYS_sendmmsg			(SYS_Linux + 343)
#define SYS_setns			(SYS_Linux + 344)
#define SYS_process_vm_readv		(SYS_Linux + 345)
#define SYS_process_vm_writev		(SYS_Linux + 346)
#define SYS_kcmp			(SYS_Linux + 347)
#define SYS_finit_module		(SYS_Linux + 348)

#endif

