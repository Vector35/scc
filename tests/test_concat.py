#!/usr/bin/env python3

# Copyright 2026 Vector 35 Inc.
# SPDX-License-Identifier: MIT

import pathlib
import subprocess
import sys
import tempfile
import unittest


SCC = None
CODE_BASE = 0x10000
STACK_BASE = 0x20000
STACK_SIZE = 0x10000
RETURN_SENTINEL = 0x40000
MASK64 = (1 << 64) - 1
RAX, RSP, RBP = 0, 4, 5
CALLEE_SAVED = (3, 6, 7, 12, 13, 14, 15)


class X64Machine:
	"""Bounded interpreter for the integer/frame instructions used by these fixtures.

	This deliberately small subset needs no native execution or emulator package.
	An unsupported instruction or access outside mapped memory fails the test.
	"""

	def __init__(self, code, entry):
		self.code = bytearray(code)
		self.stack = bytearray(STACK_SIZE)
		self.registers = [0x1234000000000000 + index * 0x101010101 for index in range(16)]
		self.registers[RSP] = STACK_BASE + STACK_SIZE - 0x108
		self.initial_registers = self.registers[:]
		self.ip = CODE_BASE + entry
		self.write_memory(self.registers[RSP], 8, RETURN_SENTINEL)

	def memory(self, address, size):
		for base, data in ((CODE_BASE, self.code), (STACK_BASE, self.stack)):
			if base <= address and address + size <= base + len(data):
				return data, address - base
		raise AssertionError("unmapped memory access at 0x%x (%d bytes)" % (address, size))

	def read_memory(self, address, size):
		data, offset = self.memory(address, size)
		return int.from_bytes(data[offset:offset + size], "little")

	def write_memory(self, address, size, value):
		data, offset = self.memory(address, size)
		data[offset:offset + size] = (value & ((1 << (size * 8)) - 1)).to_bytes(size, "little")

	def fetch(self, size=1, signed=False):
		if not CODE_BASE <= self.ip <= CODE_BASE + len(self.code) - size:
			raise AssertionError("instruction fetch outside code at 0x%x" % self.ip)
		value = self.read_memory(self.ip, size)
		self.ip += size
		if signed and value & (1 << (size * 8 - 1)):
			value -= 1 << (size * 8)
		return value

	def push(self, value):
		self.registers[RSP] -= 8
		self.write_memory(self.registers[RSP], 8, value)

	def pop(self):
		value = self.read_memory(self.registers[RSP], 8)
		self.registers[RSP] += 8
		return value

	def modrm(self, rex):
		value = self.fetch()
		mode, field, low = value >> 6, (value >> 3) & 7, value & 7
		register = field + (8 if rex & 4 else 0)
		if mode == 3:
			return field, register, ("register", low + (8 if rex & 1 else 0))
		if low == 5 and mode == 0:
			return field, register, ("rip", self.fetch(4, signed=True))
		if low == 4:
			sib = self.fetch()
			scale, index, base = sib >> 6, (sib >> 3) & 7, sib & 7
			address = 0
			if index != 4 or rex & 2:
				address += self.registers[index + (8 if rex & 2 else 0)] << scale
			if base == 5 and mode == 0:
				address += self.fetch(4, signed=True)
			else:
				address += self.registers[base + (8 if rex & 1 else 0)]
		else:
			address = self.registers[low + (8 if rex & 1 else 0)]
		if mode == 1:
			address += self.fetch(signed=True)
		elif mode == 2:
			address += self.fetch(4, signed=True)
		return field, register, ("memory", address & MASK64)

	def address(self, operand):
		kind, value = operand
		if kind == "rip":
			return (self.ip + value) & MASK64
		if kind != "memory":
			raise AssertionError("expected a memory operand")
		return value

	def read(self, operand, size):
		if operand[0] == "register":
			return self.registers[operand[1]] & ((1 << (size * 8)) - 1)
		return self.read_memory(self.address(operand), size)

	def write(self, operand, size, value):
		value &= (1 << (size * 8)) - 1
		if operand[0] == "register":
			# A 32-bit register write clears the high half in 64-bit mode.
			self.registers[operand[1]] = value
		else:
			self.write_memory(self.address(operand), size, value)

	def run_until(self, stop):
		for _ in range(10000):
			if self.ip == stop:
				return
			instruction = self.ip
			opcode = self.fetch()
			rex = 0
			if 0x40 <= opcode <= 0x4f:
				rex, opcode = opcode, self.fetch()
			size = 8 if rex & 8 else 4
			if 0x50 <= opcode <= 0x57:
				self.push(self.registers[opcode - 0x50 + (8 if rex & 1 else 0)])
			elif 0x58 <= opcode <= 0x5f:
				self.registers[opcode - 0x58 + (8 if rex & 1 else 0)] = self.pop()
			elif opcode in (0x68, 0x6a):
				self.push(self.fetch(4 if opcode == 0x68 else 1, signed=True))
			elif 0xb8 <= opcode <= 0xbf:
				self.write(("register", opcode - 0xb8 + (8 if rex & 1 else 0)), size, self.fetch(size))
			elif opcode in (0xe8, 0xe9, 0xeb):
				displacement = self.fetch(1 if opcode == 0xeb else 4, signed=True)
				if opcode == 0xe8:
					self.push(self.ip)
				self.ip += displacement
			elif opcode in (0xc3, 0xc2):
				cleanup = self.fetch(2) if opcode == 0xc2 else 0
				self.ip = self.pop()
				self.registers[RSP] += cleanup
			elif opcode == 0xc9:
				self.registers[RSP] = self.registers[RBP]
				self.registers[RBP] = self.pop()
			elif opcode == 0x90:
				pass
			elif opcode in (0x01, 0x03, 0x31, 0x33, 0x81, 0x83, 0x89, 0x8b, 0x8d, 0xc7, 0xff):
				field, register, operand = self.modrm(rex)
				reg = ("register", register)
				if opcode == 0x8d:
					self.write(reg, size, self.address(operand))
				elif opcode in (0x89, 0x8b):
					destination, source = (operand, reg) if opcode == 0x89 else (reg, operand)
					self.write(destination, size, self.read(source, size))
				elif opcode in (0x01, 0x03, 0x31, 0x33):
					destination, source = (operand, reg) if opcode in (0x01, 0x31) else (reg, operand)
					a, b = self.read(destination, size), self.read(source, size)
					self.write(destination, size, a ^ b if opcode in (0x31, 0x33) else a + b)
				elif opcode in (0x81, 0x83) and field in (0, 5):
					immediate = self.fetch(1 if opcode == 0x83 else 4, signed=True)
					value = self.read(operand, size)
					self.write(operand, size, value + immediate if field == 0 else value - immediate)
				elif opcode == 0xc7 and field == 0:
					self.write(operand, size, self.fetch(4, signed=True))
				elif opcode == 0xff and field in (2, 4):
					target = self.read(operand, 8)
					if field == 2:
						self.push(self.ip)
					self.ip = target
				else:
					raise AssertionError("unsupported opcode 0x%x /%d at 0x%x" % (opcode, field, instruction))
			else:
				raise AssertionError("unsupported opcode 0x%x at 0x%x" % (opcode, instruction))
		raise AssertionError("instruction limit reached before 0x%x" % stop)


def spill_source():
	# Keeping twelve call results live forces _start to spill to its frame and
	# use every callee-saved general-purpose register when compiled with -O0.
	parameters = ",".join("int p%d" % index for index in range(12))
	addition = "+".join("p%d" % index for index in range(12))
	arguments = ",".join("identity(%d)" % index for index in range(12))
	return (
		"int identity(int value) { return value; }\n"
		"int sum(%s) { return %s; }\n"
		"int initialized = sum(%s);\n"
		"int main(void) { return initialized; }\n"
	) % (parameters, addition, arguments)


class ConcatTests(unittest.TestCase):
	def compile(self, source, *options):
		with tempfile.TemporaryDirectory(prefix="scc-concat-") as directory:
			output = pathlib.Path(directory) / "output.bin"
			map_file = pathlib.Path(directory) / "output.map"
			command = [str(SCC), "--stdin", "--arch", "x64", "--platform", "none", "-f", "bin",
				"-o", str(output), "--map", str(map_file)] + list(options)
			result = subprocess.run(command, input=source, text=True, capture_output=True)
			self.assertEqual(result.returncode, 0, " ".join(command) + "\n" + result.stderr + result.stdout)
			addresses = {}
			for line in map_file.read_text().splitlines():
				address, name = line.split(None, 1)
				addresses[name] = int(address, 16)
			return output.read_bytes(), addresses

	def assert_preserved_frame(self, machine, stack_delta=0):
		self.assertEqual(machine.registers[RSP], machine.initial_registers[RSP] + stack_delta, "stack pointer")
		for register in (RBP,) + CALLEE_SAVED:
			self.assertEqual(machine.registers[register], machine.initial_registers[register], "register %d" % register)
		self.assertEqual(machine.read_memory(machine.initial_registers[RSP], 8), RETURN_SENTINEL, "return slot")

	def test_concat_restores_entry_frame_at_output_end(self):
		for name, source in (("empty", "int main(void) { return 0; }\n"), ("spills", spill_source())):
			for allow_return in (False, True):
				with self.subTest(source=name, allow_return=allow_return):
					options = ["--concat", "-O0"] + (["--allow-return"] if allow_return else [])
					code, addresses = self.compile(source, *options)
					self.assertEqual(addresses["__end"], len(code))
					machine = X64Machine(code, addresses["_start"])
					machine.run_until(CODE_BASE + addresses["__end"])
					self.assert_preserved_frame(machine)
					if name == "spills":
						self.assertEqual(machine.read_memory(CODE_BASE + addresses["initialized"], 4), 66)

	def test_concat_continues_into_returning_fragment(self):
		second, second_addresses = self.compile("int main(void) { return 42; }\n", "--allow-return")
		self.assertEqual(second_addresses["_start"], 0)
		for name, source in (("empty", "int main(void) { return 0; }\n"), ("spills", spill_source())):
			with self.subTest(source=name):
				first, addresses = self.compile(source, "--concat", "-O0")
				self.assertEqual(addresses["__end"], len(first))
				machine = X64Machine(first + second, addresses["_start"])
				machine.run_until(CODE_BASE + len(first))
				self.assert_preserved_frame(machine)
				machine.run_until(RETURN_SENTINEL)
				self.assertEqual(machine.registers[RAX], 42)
				self.assert_preserved_frame(machine, stack_delta=8)


def main():
	global SCC
	if len(sys.argv) != 2:
		print("usage: %s /path/to/scc" % pathlib.Path(sys.argv[0]).name, file=sys.stderr)
		return 2
	SCC = pathlib.Path(sys.argv[1]).resolve()
	if not SCC.is_file():
		print("SCC executable does not exist: %s" % SCC, file=sys.stderr)
		return 2
	sys.argv[:] = [sys.argv[0]]
	return 0 if unittest.main(exit=False).result.wasSuccessful() else 1


if __name__ == "__main__":
	sys.exit(main())
