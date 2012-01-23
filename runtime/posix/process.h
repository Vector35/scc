#ifndef __LIBC__PROCESS_H__
#define __LIBC__PROCESS_H__

typedef int pid_t;
typedef int uid_t;

void exit(int result);

pid_t fork(void);
int execve(const char* filename, const char** argv, const char** envp);
pid_t wait(int* status);
pid_t waitpid(pid_t pid, int* status, int options);

unsigned int alarm(unsigned int seconds);

#endif

