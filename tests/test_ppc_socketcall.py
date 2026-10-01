#!/usr/bin/env python3

import pathlib
import struct
import subprocess
import sys
import tempfile


SOCKETCALLS = {
	"socket": (1, [101, 102, 103], "socket(101, 102, 103)"),
	"bind": (2, [101, 102, 103], "bind(101, (struct sockaddr*)102, 103)"),
	"connect": (3, [101, 102, 103], "connect(101, (struct sockaddr*)102, 103)"),
	"listen": (4, [101, 102], "listen(101, 102)"),
	"accept": (5, [101, 102, 103], "accept(101, (struct sockaddr*)102, (socklen_t*)103)"),
	"getsockname": (6, [101, 102, 103], "getsockname(101, (struct sockaddr*)102, (socklen_t*)103)"),
	"getpeername": (7, [101, 102, 103], "getpeername(101, (struct sockaddr*)102, (socklen_t*)103)"),
	"socketpair": (8, [101, 102, 103, 104], "socketpair(101, 102, 103, (int*)104)"),
	"send": (9, [101, 102, 103, 104], "send(101, (void*)102, 103, 104)"),
	"recv": (10, [101, 102, 103, 104], "recv(101, (void*)102, 103, 104)"),
	"sendto": (11, [101, 102, 103, 104, 105, 106],
		"sendto(101, (void*)102, 103, 104, (struct sockaddr*)105, 106)"),
	"recvfrom": (12, [101, 102, 103, 104, 105, 106],
		"recvfrom(101, (void*)102, 103, 104, (struct sockaddr*)105, (socklen_t*)106)"),
	"shutdown": (13, [101, 102], "shutdown(101, 102)"),
	"setsockopt": (14, [101, 102, 103, 104, 105],
		"setsockopt(101, 102, 103, (void*)104, 105)"),
	"getsockopt": (15, [101, 102, 103, 104, 105],
		"getsockopt(101, 102, 103, (void*)104, (socklen_t*)105)"),
	"accept4": (18, [101, 102, 103, 104],
		"accept4(101, (struct sockaddr*)102, (socklen_t*)103, 104)"),
}


def sign_extend_16(value):
	return value - 0x10000 if value & 0x8000 else value


def emulate_until_syscall(code, byte_order):
	"""Emulate the integer instructions SCC emits while preparing the first syscall."""
	registers = [0] * 32
	registers[1] = 0x100000
	memory = {}
	words = struct.iter_unpack(">I" if byte_order == "big" else "<I", code)

	for offset, (word,) in enumerate(words):
		opcode = word >> 26
		rt_rs = (word >> 21) & 31
		ra = (word >> 16) & 31
		immediate = word & 0xffff

		if word == 0x44000002:  # sc
			return registers, memory
		if opcode == 14:  # addi (including li)
			base = 0 if ra == 0 else registers[ra]
			registers[rt_rs] = (base + sign_extend_16(immediate)) & 0xffffffff
		elif opcode == 15:  # addis (including lis)
			base = 0 if ra == 0 else registers[ra]
			registers[rt_rs] = (base + (sign_extend_16(immediate) << 16)) & 0xffffffff
		elif opcode == 24:  # ori (including mr as emitted by SCC)
			registers[ra] = registers[rt_rs] | immediate
		elif opcode == 25:  # oris
			registers[ra] = registers[rt_rs] | (immediate << 16)
		elif opcode in (36, 37):  # stw, stwu
			address = ((0 if ra == 0 else registers[ra]) + sign_extend_16(immediate)) & 0xffffffff
			memory[address] = registers[rt_rs]
			if opcode == 37:
				registers[ra] = address
		elif opcode == 47:  # stmw; only the stack-frame save, irrelevant to arguments
			continue
		elif opcode == 31 and ((word >> 1) & 0x3ff) == 444:  # or (including mr)
			rb = (word >> 11) & 31
			registers[ra] = registers[rt_rs] | registers[rb]
		elif word & 0xfc1fffff == 0x7c0802a6:  # mflr
			registers[rt_rs] = 0
		else:
			raise AssertionError("unsupported PPC instruction 0x%08x at byte offset 0x%x" % (word, offset * 4))

	raise AssertionError("generated code did not contain a syscall")


def compile_wrapper(scc, arch, call, output):
	source = "void main() { %s; }\n" % call
	result = subprocess.run(
		[str(scc), "--stdin", "--platform", "linux", "--arch", arch, "-f", "bin", "-o", str(output)],
		input=source.encode("ascii"), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	if result.returncode != 0:
		raise AssertionError("SCC failed for %s:\n%s" % (call, result.stdout.decode("utf-8", "replace")))
	return output.read_bytes()


def validate_architecture(scc, arch, byte_order, directory):
	for name, (operation, expected_args, call) in SOCKETCALLS.items():
		code = compile_wrapper(scc, arch, call, directory / (name + ".bin"))
		registers, memory = emulate_until_syscall(code, byte_order)
		actual_args = [memory.get(registers[4] + (i * 4)) for i in range(len(expected_args))]

		if registers[0] != 102:
			raise AssertionError("%s/%s: r0 is %d, expected SYS_socketcall (102)" % (
				arch, name, registers[0]))
		if registers[3] != operation:
			raise AssertionError("%s/%s: r3 is %d, expected socketcall operation %d" % (
				arch, name, registers[3], operation))
		if registers[4] & 3:
			raise AssertionError("%s/%s: r4 argument array is not word-aligned" % (arch, name))
		if actual_args != expected_args:
			raise AssertionError("%s/%s: argument array is %r, expected %r" % (
				arch, name, actual_args, expected_args))


def main():
	if len(sys.argv) != 2:
		print("usage: %s /path/to/scc" % pathlib.Path(sys.argv[0]).name, file=sys.stderr)
		return 2

	scc = pathlib.Path(sys.argv[1]).resolve()
	with tempfile.TemporaryDirectory(prefix="scc-ppc-socketcall-") as directory:
		path = pathlib.Path(directory)
		validate_architecture(scc, "ppc", "big", path)
		validate_architecture(scc, "ppcel", "little", path)

	print("validated %d socketcall wrappers for ppc and ppcel" % len(SOCKETCALLS))
	return 0


if __name__ == "__main__":
	sys.exit(main())
