# Copyright (c) 2011-2012 Rusty Wagner
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import subprocess
import sys
import os

args_testcase = {"source": "tests/args.c", "inputfile": None, "outputfile": "tests/args_output"}
shift_testcase = {"source": "tests/shift.c", "inputfile": None, "outputfile": "tests/shift_output"}
string_testcase = {"source": "tests/string.c", "inputfile": None, "outputfile": "tests/string_output"}
pi_testcase = {"source": "tests/pi.c", "inputfile": None, "outputfile": "tests/pi_output"}
rc4_testcase = {"source": "tests/rc4.c", "inputfile": None, "outputfile": "tests/rc4_output"}
crc32_testcase = {"source": "tests/crc32.c", "inputfile": None, "outputfile": "tests/crc32_output"}
div64_testcase = {"source": "tests/div64.c", "inputfile": None, "outputfile": "tests/div64_output"}
mul64_testcase = {"source": "tests/mul64.c", "inputfile": None, "outputfile": "tests/mul64_output"}
fortress_testcase = {"source": "tests/fortress.c", "inputfile": "tests/fortress_input", "outputfile": "tests/fortress_output"}
shellcode_mmap_testcase = {"source": "tests/shellcode.c", "inputfile": None, "outputfile": "tests/shellcode_output", "target": "tests/sploit_mmap.c"}
shellcode_stack_testcase = {"source": "tests/shellcode.c", "inputfile": None, "outputfile": "tests/shellcode_output", "target": "tests/sploit_stack.c", "targetoptions": ["-O0", "--exec-stack"]}

tests = [
	["args.c, normal", args_testcase, []],
#	["args.c, stack grows up", args_testcase, ["--stack-grows-up"]],
	["shift.c, normal", shift_testcase, []],
	["string.c, normal", string_testcase, []],
	["pi.c, normal", pi_testcase, []],
	["rc4.c, normal", rc4_testcase, []],
	["rc4.c, position independent", rc4_testcase, ["--pie"]],
	["rc4.c, polymorphic", rc4_testcase, ["--polymorph", "--seed", "<SEED>"]],
	["crc32.c, normal", crc32_testcase, []],
	["crc32.c, polymorphic", crc32_testcase, ["--polymorph", "--seed", "<SEED>"]],
	["div64.c, normal", div64_testcase, []],
	["mul64.c, normal", mul64_testcase, []],
	["shellcode, mmap buffer", shellcode_mmap_testcase, []],
	["shellcode, stack buffer", shellcode_stack_testcase, ["--unsafe-stack"]],
	["fortress.c, normal", fortress_testcase, []],
	["fortress.c, position indepedent", fortress_testcase, ["--pie"]],
	["fortress.c, polymorphic", fortress_testcase, ["--polymorph", "--seed", "<SEED>"]],
	["fortress.c, anti-disassembly", fortress_testcase, ["--anti-disasm", "--seed", "<SEED>"]],
#	["fortress.c, stack grows up", fortress_testcase, ["--stack-grows-up"]],
#	["fortress.c, alternate stack ptr", fortress_testcase, ["--stack-reg", "ebx"]],
#	["fortress.c, alternate frame ptr", fortress_testcase, ["--frame-reg", "ecx"]],
#	["fortress.c, alternate stack and frame ptr", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx"]],
#	["fortress.c, encode pointers", fortress_testcase, ["--encode-pointers"]],
#	["fortress.c, everything", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx", "--encode-pointers", "--stack-grows-up", "--pie", "--polymorph", "--seed", "<SEED>"]],
]

seeds = [17, 42, 1024, 1337, 4096, 4141, 7331, 31337, 65536, 1048576]

quarkvm_built = False

def test(arch_name, name, testcase, arch_options, options, arch_type):
	global quarkvm_built
	if arch_type == "quark" and not quarkvm_built:
		interpret_arch_options = list(arch_options)
		interpret_arch_options[interpret_arch_options.index("quark")] = "x86"
		if os.uname()[0] == "Darwin":
			compile_cmd = ["./scc", "-o", "Obj/testvm", "-f", "macho", "tests/vm.c", "-m32"] + interpret_arch_options
		else:
			compile_cmd = ["./scc", "-o", "Obj/testvm", "-f", "elf", "tests/vm.c", "-m32"] + interpret_arch_options
		proc = subprocess.Popen(compile_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		output = proc.communicate()[0]
		if proc.returncode != 0:
			sys.stdout.write("\033[01;31mFAILED\033[00m\n")
			sys.stdout.write("Compiler command line: " + (" ".join(compile_cmd)) + "\n")
			sys.stdout.write("VM interpreter compilation failed with exit code %d:\n" % proc.returncode)
			sys.stdout.write(output)
			sys.stdout.write("\n")
			return False

		os.chmod("Obj/testvm", 0o700)
		quarkvm_built = True

	if "target" in testcase:
		if arch_type == "quark":
			compile_cmd = ["./scc", "-o", "Obj/test", "-f", "bin", testcase["target"]] + arch_options
		elif os.uname()[0] == "Darwin":
			compile_cmd = ["./scc", "-o", "Obj/test", "-f", "macho", testcase["target"]] + arch_options
		else:
			compile_cmd = ["./scc", "-o", "Obj/test", "-f", "elf", testcase["target"]] + arch_options
		if "targetoptions" in testcase:
			compile_cmd += testcase["targetoptions"]
		proc = subprocess.Popen(compile_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		output = proc.communicate()[0]
		if proc.returncode != 0:
			sys.stdout.write("\033[01;31mFAILED\033[00m\n")
			sys.stdout.write("Compiler command line: " + (" ".join(compile_cmd)) + "\n")
			sys.stdout.write("Target compilation failed with exit code %d:\n" % proc.returncode)
			sys.stdout.write(output)
			sys.stdout.write("\n")
			return False

		os.chmod("Obj/test", 0o700)

	if "target" in testcase:
		compile_cmd = ["./scc", "-o", "Obj/testsc", "-f", "bin", testcase["source"]] + arch_options + options
	elif arch_type == "quark":
		compile_cmd = ["./scc", "-o", "Obj/test", "-f", "bin", testcase["source"]] + arch_options + options
	elif os.uname()[0] == "Darwin":
		compile_cmd = ["./scc", "-o", "Obj/test", "-f", "macho", testcase["source"]] + arch_options + options
	else:
		compile_cmd = ["./scc", "-o", "Obj/test", "-f", "elf", testcase["source"]] + arch_options + options
	proc = subprocess.Popen(compile_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	output = proc.communicate()[0]
	if proc.returncode != 0:
		sys.stdout.write("\033[01;31mFAILED\033[00m\n")
		sys.stdout.write("Compiler command line: " + (" ".join(compile_cmd)) + "\n")
		sys.stdout.write("Compiler failed with exit code %d:\n" % proc.returncode)
		sys.stdout.write(output)
		sys.stdout.write("\n")
		return False

	if testcase["outputfile"]:
		output_contents = open(testcase["outputfile"], "r").read()
	else:
		output_contents = ""

	if "target" in testcase:
		input_contents = open("Obj/testsc", "r").read()
		if arch_type == "quark":
			cmd = ["Obj/testvm"]
		else:
			cmd = ["Obj/test"]
		if "targetparams" in testcase:
			cmd += testcase["targetparams"]
	else:
		if testcase["inputfile"]:
			input_contents = open(testcase["inputfile"], "r").read()
		else:
			input_contents = ""
		os.chmod("Obj/test", 0o700)
		if arch_type == "quark":
			if os.uname()[0] == "Darwin":
				cmd = ["/bin/bash", "-c", "Obj/testvm"]
			else:
				cmd = ["Obj/testvm"]
		else:
			if os.uname()[0] == "Darwin":
				cmd = ["/bin/bash", "-c", "Obj/test"]
			else:
				cmd = ["Obj/test"]

	proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	output = proc.communicate(input_contents)[0]
	if proc.returncode != 0:
		sys.stdout.write("\033[01;31mFAILED\033[00m\n")
		sys.stdout.write("Program failed with exit code %d:\n" % proc.returncode)
		sys.stdout.write("Compiler command line: " + (" ".join(compile_cmd)) + "\n")
		out_file = ("Obj/testoutput " + arch_name + " " + name).replace(" ", "_")
		sys.stdout.write("Output in " + out_file + "\n\n")
		open(out_file, "w").write(output)
		return False

	if output != output_contents:
		sys.stdout.write("\033[01;31mFAILED\033[00m\n")
		sys.stdout.write("Output does not match what was expected\n")
		sys.stdout.write("Compiler command line: " + (" ".join(compile_cmd)) + "\n")
		if testcase["inputfile"]:
			sys.stdout.write("Input in " + testcase["inputfile"] + "\n")
		if testcase["outputfile"]:
			sys.stdout.write("Expected output in " + testcase["outputfile"] + "\n")
		out_file = ("Obj/testoutput " + arch_name + " " + name).replace(" ", "_")
		sys.stdout.write("Actual output in " + out_file + "\n\n")
		open(out_file, "w").write(output)
		return False

	return True

def test_all(arch_name, arch_options, arch_type, native):
	global only_archs

	if (only_archs is not None) and (arch_type not in only_archs):
		return 0

	sys.stdout.write("\033[01;33m" + arch_name + "\033[00m\n")

	failed = 0
	for t in tests:
		if (not native) and ("--pie" in list(t[2])):
			# Don't try to run ELF files that require dynamic loader on non-native archs
			continue

		padded_name = t[0] + (" " * (60 - len(t[0])))
		sys.stdout.write("\033[01;34m" + padded_name + "\033[00m")
		sys.stdout.flush()

		if "<SEED>" in t[2]:
			this_failed = False
			for seed in seeds:
				options = list(t[2])
				options[options.index("<SEED>")] = str(seed)
				if not test(arch_name, t[0], t[1], arch_options, options, arch_type):
					this_failed = True
					break
				
			if this_failed:
				failed += 1
			else:
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
		else:
			if test(arch_name, t[0], t[1], arch_options, t[2], arch_type):
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
			else:
				failed += 1

	return failed

if len(sys.argv) > 1:
	only_archs = sys.argv[1:]
else:
	only_archs = None

failed = 0
if os.uname()[0] == "Darwin":
	failed += test_all("Mac OS X x86", ["--platform", "mac", "--arch", "x86"], "x86", True)
	failed += test_all("Mac OS X x64", ["--platform", "mac", "--arch", "x64"], "x64", True)
	failed += test_all("Mac OS X Quark", ["--platform", "mac", "--arch", "quark"], "quark", False)
elif os.name != "nt":
	failed += test_all("Linux x86", ["--platform", "linux", "--arch", "x86"], "x86", True)
	failed += test_all("Linux Quark", ["--platform", "linux", "--arch", "quark"], "quark", False)
	if os.uname()[0] == "FreeBSD":
		failed += test_all("FreeBSD x86", ["--platform", "freebsd", "--arch", "x86"], "x86", True)
		failed += test_all("FreeBSD x64", ["--platform", "freebsd", "--arch", "x64"], "x64", True)
		failed += test_all("FreeBSD Quark", ["--platform", "freebsd", "--arch", "quark"], "quark", False)
	else:
		failed += test_all("Linux x64", ["--platform", "linux", "--arch", "x64"], "x64", True)
		failed += test_all("Linux MIPS little-endian", ["--platform", "linux", "--arch", "mipsel"], "mipsel", False)
		failed += test_all("Linux MIPS big-endian", ["--platform", "linux", "--arch", "mips"], "mips", False)
		failed += test_all("Linux AArch64", ["--platform", "linux", "--arch", "aarch64"], "aarch64", False)
		failed += test_all("Linux ARM little-endian", ["--platform", "linux", "--arch", "arm"], "arm", False)
		failed += test_all("Linux ARM big-endian", ["--platform", "linux", "--arch", "armeb"], "armeb", False)
		failed += test_all("Linux PowerPC", ["--platform", "linux", "--arch", "ppc"], "ppc", False)

if failed != 0:
	sys.stdout.write("\033[01;31m%d test(s) failed\033[00m\n" % failed)
	sys.exit(1)

sys.stdout.write("\033[01;32mAll tests passed\033[00m\n")
sys.exit(0)

