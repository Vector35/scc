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

#ifndef __LIBC__PROCESS_H__
#define __LIBC__PROCESS_H__

#define PROCESS_TERMINATE                 1
#define PROCESS_CREATE_THREAD             2
#define PROCESS_VM_OPERATION              8
#define PROCESS_VM_READ                   0x10
#define PROCESS_VM_WRITE                  0x20
#define PROCESS_DUP_HANDLE                0x40
#define PROCESS_CREATE_PROCESS            0x80
#define PROCESS_SET_QUOTA                 0x100
#define PROCESS_SET_INFORMATION           0x200
#define PROCESS_QUERY_INFORMATION         0x400
#define PROCESS_SUSPEND_RESUME            0x800
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#define DELETE                            0x10000
#define READ_CONTROL                      0x20000
#define WRITE_DAC                         0x40000
#define WRITE_OWNER                       0x80000
#define SYNCHRONIZE                       0x100000

#define DEBUG_PROCESS                    1
#define DEBUG_ONLY_THIS_PROCESS          2
#define CREATE_SUSPENDED                 4
#define DETACHED_PROCESS                 8
#define CREATE_NEW_CONSOLE               0x10
#define CREATE_NEW_PROCESS_GROUP         0x200
#define CREATE_UNICODE_ENVIRONMENT       0x400
#define INHERIT_PARENT_AFFINITY          0x10000
#define CREATE_PROTECTED_PROCESS         0x40000
#define EXTENDED_STARTUPINFO_PRESENT     0x80000
#define CREATE_BREAKAWAY_FROM_JOB        0x1000000
#define CREATE_PRESERVE_CODE_AUTHZ_LEVEL 0x2000000
#define CREATE_DEFAULT_ERROR_MODE        0x4000000
#define CREATE_NO_WINDOW                 0x8000000

#define STARTF_USESHOWWINDOW    1
#define STARTF_USESIZE          2
#define STARTF_USEPOSITION      4
#define STARTF_USECOUNTCHARS    8
#define STARTF_USEFILLATTRIBUTE 0x10
#define STARTF_RUNFULLSCREEN    0x20
#define STARTF_FORCEONFEEDBACK  0x40
#define STARTF_FORCEOFFFEEDBACK 0x80
#define STARTF_USESTDHANDLES    0x100
#define STARTF_USEHOTKEY        0x200
#define STARTF_TITLEISLINKNAME  0x800
#define STARTF_TITLEISAPPID     0x1000
#define STARTF_PREVENTPINNING   0x2000

#define SW_HIDE            0
#define SW_SHOWNORMAL      1
#define SW_SHOWMINIMIZED   2
#define SW_MAXIMIZE        3
#define SW_SHOWMAXIMIZED   3
#define SW_SHOWNOACTIVATE  4
#define SW_SHOW            5
#define SW_MINIMIZE        6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA          8
#define SW_RESTORE         9
#define SW_SHOWDEFAULT     10
#define SW_FORCEMINIMIZE   11

#define MB_OK                   0
#define MB_OKCANCEL             1
#define MB_ABORTRETRYIGNORE     2
#define MB_YESNOCANCEL          3
#define MB_YESNO                4
#define MB_RETRYCANCEL          5
#define MB_CANCELTRYCONTINUE    6
#define MB_ICONSTOP             0x10
#define MB_ICONERROR            0x10
#define MB_ICONHAND             0x10
#define MB_ICONQUESTION         0x20
#define MB_ICONEXCLAMATION      0x30
#define MB_ICONWARNING          0x30
#define MB_ICONINFORMATION      0x40
#define MB_ICONASTERISK         0x40
#define MB_DEFBUTTON1           0
#define MB_DEFBUTTON2           0x100
#define MB_DEFBUTTON3           0x200
#define MB_DEFBUTTON4           0x300
#define MB_APPMODAL             0
#define MB_SYSTEMMODAL          0x1000
#define MB_TASKMODAL            0x2000
#define MB_HELP                 0x4000
#define MB_SETFOREGROUND        0x10000
#define MB_DEFAULT_DESKTOP_ONLY 0x20000
#define MB_TOPMOST              0x40000
#define MB_RIGHT                0x80000
#define MB_SERVICE_NOTIFICATION 0x200000
#define MB_RTLREADING           0x100000

#define IDOK       1
#define IDCANCEL   2
#define IDABORT    3
#define IDRETRY    4
#define IDIGNORE   5
#define IDYES      6
#define IDNO       7
#define IDCONTINUE 11
#define IDTRYAGAIN 10

#define CW_USEDEFAULT 0x80000000

#define INFINITE 0xffffffff

#define HANDLE_FLAG_INHERIT            1
#define HANDLE_FLAG_PROTECT_FROM_CLOSE 2

#define WAIT_OBJECT_0    0
#define WAIT_ABANDONED_0 0x80
#define WAIT_TIMEOUT     0x102
#define WAIT_ERROR       0xffffffff

typedef struct
{
	uint32_t cb;
	char* lpReserved;
	char* lpDesktop;
	char* lpTitle;
	uint32_t dwX;
	uint32_t dwY;
	uint32_t dwXSize;
	uint32_t dwYSize;
	uint32_t dwXCountChars;
	uint32_t dwYCountChars;
	uint32_t dwFillAttribute;
	uint32_t dwFlags;
	uint16_t wShowWindow;
	uint16_t cbReserved2;
	uint8_t* lpReserved2;
	HANDLE hStdInput;
	HANDLE hStdOutput;
	HANDLE hStdError;
} STARTUPINFOA;

typedef struct
{
	HANDLE hProcess;
	HANDLE hThread;
	uint32_t dwProcessId;
	uint32_t dwThreadId;
} PROCESS_INFORMATION;

#define GetCurrentProcess() ((HANDLE)0xffffffff)
#define GetCurrentThread()  ((HANDLE)0xfffffffe)
void __stdcall ExitProcess(uint32_t exitCode) __import("kernel32") __noreturn;
HANDLE __stdcall OpenProcess(uint32_t access, bool inherit, uint32_t pid) __import("kernel32");
bool __stdcall TerminateProcess(HANDLE proc, uint32_t code) __import("kernel32");
bool __stdcall CreateProcessA(const char* app, const char* cmdline, void* processAttr,
    void* threadAttr, bool inherit, uint32_t flags, void* environ, const char* dir,
    STARTUPINFOA* startup, PROCESS_INFORMATION* info) __import("kernel32");
uint32_t __stdcall WaitForSingleObject(HANDLE handle, uint32_t milliseconds) __import("kernel32");
uint32_t __stdcall WaitForMultipleObjects(
    uint32_t count, const HANDLE* handles, bool all, uint32_t milliseconds) __import("kernel32");
uint32_t __stdcall GetProcessId(HANDLE proc) __import("kernel32");
uint32_t __stdcall GetCurrentProcessId() __import("kernel32");
uint32_t __stdcall GetThreadId(HANDLE thread) __import("kernel32");
uint32_t __stdcall GetCurrentThreadId() __import("kernel32");
bool __stdcall GetExitCodeProcess(HANDLE proc, uint32_t* exitCode) __import("kernel32");
bool __stdcall GetExitCodeThread(HANDLE thread, uint32_t* exitCode) __import("kernel32");
bool __stdcall GetHandleInformation(HANDLE obj, uint32_t* flags) __import("kernel32");
bool __stdcall SetHandleInformation(HANDLE obj, uint32_t mask, uint32_t flags) __import("kernel32");
bool __stdcall GetUserNameA(char* buf, uint32_t* size) __import("advapi32");

int __stdcall MessageBoxA(HANDLE hwnd, const char* msg, const char* title, uint32_t flags)
    __import("user32");

void exit(int result) __noreturn;
#define _exit(result) exit(result)

int system(const char* cmd);
void shell(int io) __noreturn;
HANDLE shell_pipe(int read, int write);

#endif
