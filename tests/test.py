import subprocess
import sys
import os

fortress_testcase = {"source": "tests/fortress.c", "inputfile": "tests/fortress_input", "outputfile": "tests/fortress_output"}

tests = [
	["fortress.c, normal", fortress_testcase, []],
	["fortress.c, position indepedent", fortress_testcase, ["--pie"]],
	["fortress.c, polymorphic", fortress_testcase, ["--polymorph", "--seed", "<SEED>"]],
	["fortress.c, stack grows up", fortress_testcase, ["--stack-grows-up"]],
	["fortress.c, alternate stack ptr", fortress_testcase, ["--stack-reg", "ebx", "--base-reg", "esi"]],
	["fortress.c, alternate frame ptr", fortress_testcase, ["--frame-reg", "ecx", "--base-reg", "esi"]],
	["fortress.c, alternate stack and frame ptr", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx", "--base-reg", "esi"]],
	["fortress.c, encode pointers", fortress_testcase, ["--encode-pointers"]],
	["fortress.c, everything", fortress_testcase, ["--stack-reg", "ebx", "--frame-reg", "ecx", "--base-reg", "esi", "--encode-pointers", "--stack-grows-up", "--pie", "--polymorph", "--seed", "<SEED>"]],
]

seeds = [17, 42, 1024, 1337, 4096, 4141, 7331, 31337, 65536, 1048576]

def test(arch_name, name, testcase, arch_options, options):
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

	input_contents = open(testcase["inputfile"], "r").read()
	output_contents = open(testcase["outputfile"], "r").read()

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
		sys.stdout.write("Input in " + testcase["inputfile"] + "\n")
		sys.stdout.write("Expected output in " + testcase["outputfile"] + "\n")
		out_file = ("Obj/testoutput " + arch_name + " " + name).replace(" ", "_")
		sys.stdout.write("Actual output in " + out_file + "\n\n")
		open(out_file, "w").write(output)
		return False

	return True

def all(arch_name, arch_options):
	sys.stdout.write("\033[01;33m" + arch_name + "\033[00m\n")

	failed = False
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
				failed = True
			else:
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
		else:
			if test(arch_name, t[0], t[1], arch_options, t[2]):
				sys.stdout.write("\033[01;32mPASSED\033[00m\n")
				sys.stdout.flush()
			else:
				failed = True

	return not failed

failed = False
if not all("Linux x86", ["--platform", "linux", "--arch", "x86"]):
	failed = True
if not all("Linux x64", ["--platform", "linux", "--arch", "x64"]):
	failed = True

if failed:
	sys.exit(1)
sys.exit(0)

