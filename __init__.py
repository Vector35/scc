import os
import subprocess

NormalOptimization = 0
Unoptimized = 1
SizeOptimization = 2

def compile_source(source, platform="linux", arch="x86", blacklist=None, allow_return=False, assume_safe_stack=False,
	base=None, base_reg=None, concat=False, encode_pointers=False, frame_reg=None, max_length=None,
	optimization=NormalOptimization, pad=False, polymorph=False, preserve_regs=None, return_reg=None,
	return_high_reg=None, seed=None, stack_grows_up=False, stack_reg=None, additional_options=None):
	cmd = [os.path.abspath(os.path.join(os.path.dirname(__file__), "scc")), "--stdin", "--stdout", "-f", "bin",
		"--platform", platform, "--arch", arch]
	if blacklist:
		for byte in blacklist:
			cmd += ["--blacklist", str(byte)]
	if allow_return:
		cmd += ["--alow-return"]
	if assume_safe_stack:
		cmd += ["--assume-safe-stack"]
	if base:
		cmd += ["--base", base]
	if base_reg:
		cmd += ["--base-reg", base_reg]
	if concat:
		cmd += ["--concat"]
	if encode_pointers:
		cmd += ["--encode-pointers"]
	if frame_reg:
		cmd += ["--frame_reg", frame_reg]
	if max_length:
		cmd += ["--max-length", str(max_length)]
	if optimization == Unoptimized:
		cmd += ["-O0"]
	if optimization == SizeOptimization:
		cmd += ["-Os"]
	if pad:
		cmd += ["--pad"]
	if polymorph:
		cmd += ["--polymorph"]
	if preserve_regs:
		for reg in preserve_regs:
			cmd += ["--preserve", reg]
	if return_reg:
		cmd += ["--return-reg", return_reg]
	if return_high_reg:
		cmd += ["--return-high-reg", return_high_reg]
	if seed:
		cmd += ["--seed", str(seed)]
	if stack_grows_up:
		cmd += ["--stack-grows-up"]
	if stack_reg:
		cmd += ["--stack-reg", stack_reg]
	if additional_options:
		cmd += additional_options

	proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	data, error = proc.communicate(input=source)
	if proc.returncode != 0:
		data = None
	return data, error

