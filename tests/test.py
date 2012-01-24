import subprocess
import sys
import os

args_testcase = {"source": "tests/args.c", "inputfile": None, "outputfile": "tests/args_output"}
shift_testcase = {"source": "tests/shift.c", "inputfile": None, "outputfile": "tests/shift_output"}
fortress_testcase = {"source": "tests/fortress.c", "inputfile": "tests/fortress_input", "outputfile": "tests/fortress_output"}
shellcode_mmap_testcase = {"source": "tests/shellcode.c", "inputfile": None, "outputfile": "tests/shellcode_output", "target": "tests/sploit_mmap.c"}
shellcode_stack_testcase = {"source": "tests/shellcode.c", "inputfile": None, "outputfile": "tests/shellcode_output", "target": "tests/sploit_stack.c", "targetoptions": ["-O0", "--exec-stack"]}

tests = [
	["args.c, normal", args_testcase, []],
	["args.c, stack grows up", args_testcase, ["--stack-grows-up"]],
	["shift.c, normal", shift_testcase, []],
	["shellcode, mmap buffer", shellcode_mmap_testcase, []],
	["shellcode, stack buffer", shellcode_stack_testcase, []],
	["fortress.c, normal", fortress_testcase, []],
	["fortress.c, position indepedent", fortress_testcase, ["--pie"]],
	["fortress.c, polymorphic", fortress_testcase, ["--polymorph", "--seed", "<SEED>"]],
	["fortress.c, stack grows up", fortress_testcase, ["--stack-grows-up"]],
	["fortress.c, alternate stack ptr", fortress_testcase, ["--stack-reg", "ebx"]],
	["fortress.c, alternate frame ptr", fortress_testcase, ["--frame-reg", "ecx"]],
	["fortress.c, alternate stack and frame ptr", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx"]],
	["fortress.c, encode pointers", fortress_testcase, ["--encode-pointers"]],
	["fortress.c, everything", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx", "--base-reg", "esi", "--encode-pointers", "--stack-grows-up", "--pie", "--polymorph", "--seed", "<SEED>"]],
]

seeds = [17, 42, 1024, 1337, 4096, 4141, 7331, 31337, 65536, 1048576]

def test(arch_name, name, testcase, arch_options, options):
	if "target" in testcase:
		compile_cmd = ["./scc", "-o", "Obj/target", "-f", "elf", testcase["target"]] + arch_options
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

		os.chmod("Obj/target", 0o700)

	if "target" in testcase:
		compile_cmd = ["./scc", "-o", "Obj/test", "-f", "bin", testcase["source"]] + arch_options + options
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
		input_contents = open("Obj/test", "r").read()
		cmd = ["Obj/target"]
		if "targetparams" in testcase:
			cmd += testcase["targetparams"]
	else:
		if testcase["inputfile"]:
			input_contents = open(testcase["inputfile"], "r").read()
		else:
			input_contents = ""
		os.chmod("Obj/test", 0o700)
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

def test_all(arch_name, arch_options):
	sys.stdout.write("\033[01;33m" + arch_name + "\033[00m\n")

	failed = 0
	for t in tests:
		padded_name = t[0] + (" " * (60 - len(t[0])))
		sys.stdout.write("\033[01;34m" + padded_name + "\033[00m")
		sys.stdout.flush()

		if "<SEED>" in t[2]:
			this_failed = False
			for seed in seeds:
				options = list(t[2])
				options[options.index("<SEED>")] = str(seed)
				if not test(arch_name, t[0], t[1], arch_options, options):
					this_failed = True
					break
				
			if this_failed:
				failed += 1
			else:
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
		else:
			if test(arch_name, t[0], t[1], arch_options, t[2]):
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
			else:
				failed += 1

	return failed

failed = 0
failed += test_all("Linux x86", ["--platform", "linux", "--arch", "x86"])
failed += test_all("Linux x64", ["--platform", "linux", "--arch", "x64"])

if failed != 0:
	sys.stdout.write("\033[01;31m%d test(s) failed\033[00m\n" % failed)
	sys.exit(1)

sys.stdout.write("\033[01;32mAll tests passed\033[00m\n")
sys.exit(0)

