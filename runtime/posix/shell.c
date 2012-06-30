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

void sh()
{
	char* argv[2];
	argv[0] = "/bin/sh";
	argv[1] = NULL;
	execve(argv[0], argv, NULL);
}

void interactive_sh()
{
	char* argv[3];
	argv[0] = "/bin/sh";
	argv[1] = "-i";
	argv[2] = NULL;
	execve(argv[0], argv, NULL);
}

void bash()
{
	char* argv[2];
	argv[0] = "/bin/bash";
	argv[1] = NULL;
	execve(argv[0], argv, NULL);
}

void interactive_bash()
{
	char* argv[3];
	argv[0] = "/bin/bash";
	argv[1] = "-i";
	argv[2] = NULL;
	execve(argv[0], argv, NULL);
}

void system(const char* cmd)
{
	char* argv[4];
	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[2] = cmd;
	argv[3] = NULL;

	pid_t pid = fork();
	if (pid == 0)
	{
		execve(argv[0], argv, NULL);
		exit(-1);
	}

	waitpid(pid, NULL, 0);
}

