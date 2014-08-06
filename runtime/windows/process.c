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

void exit(int result) __noreturn
{
	ExitProcess(result);
}

int system(const char* cmd)
{
	STARTUPINFOA startup;
	PROCESS_INFORMATION info;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);

	if (!CreateProcessA(NULL, cmd, NULL, NULL, false, 0, NULL, NULL, &startup, &info))
		return -1;

	CloseHandle(info.hThread);
	WaitForSingleObject(info.hProcess, INFINITE);
	uint32_t result;
	GetExitCodeProcess(info.hProcess, &result);
	CloseHandle(info.hProcess);
	return (int)result;
}

void shell(int io) __noreturn
{
	SetHandleInformation((HANDLE)io, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	STARTUPINFOA startup;
	PROCESS_INFORMATION info;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdInput = (HANDLE)io;
	startup.hStdOutput = (HANDLE)io;
	startup.hStdError = (HANDLE)io;

	CreateProcessA(NULL, "cmd.exe", NULL, NULL, true, 0, NULL, NULL, &startup, &info);
	ExitProcess(__undefined);
}

HANDLE shell_pipe(int read, int write)
{
	SetHandleInformation((HANDLE)read, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	SetHandleInformation((HANDLE)write, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	STARTUPINFOA startup;
	PROCESS_INFORMATION info;
	memset(&startup, 0, sizeof(startup));
	startup.cb = sizeof(startup);
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdInput = (HANDLE)read;
	startup.hStdOutput = (HANDLE)write;
	startup.hStdError = (HANDLE)write;

	if (!CreateProcessA(NULL, "cmd.exe", NULL, NULL, true, 0, NULL, NULL, &startup, &info))
		return INVALID_HANDLE_VALUE;
	CloseHandle(info.hThread);
	return info.hProcess;
}

