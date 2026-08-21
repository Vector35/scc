#!/usr/bin/env python3

# Copyright 2026 Vector 35 Inc.
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

import pathlib
import struct
import subprocess
import sys
import tempfile
import unittest


SCC = None


def elf_virtual_address_to_file_offset(binary, address):
	if binary[:6] != b"\x7fELF\x02\x01":
		raise AssertionError("expected a little-endian ELF64 binary")

	program_header_offset = struct.unpack_from("<Q", binary, 32)[0]
	program_header_size = struct.unpack_from("<H", binary, 54)[0]
	program_header_count = struct.unpack_from("<H", binary, 56)[0]
	for index in range(program_header_count):
		offset = program_header_offset + (index * program_header_size)
		segment_type = struct.unpack_from("<I", binary, offset)[0]
		if segment_type != 1:  # PT_LOAD
			continue

		file_offset, virtual_address, file_size = struct.unpack_from("<QQ8xQ", binary, offset + 8)
		if virtual_address <= address < virtual_address + file_size:
			return file_offset + (address - virtual_address)

	raise AssertionError("virtual address 0x%x is not backed by an ELF load segment" % address)


class X64RegressionTests(unittest.TestCase):
	def compile(self, source, *options):
		with tempfile.TemporaryDirectory(prefix="scc-x64-regression-") as directory:
			output = pathlib.Path(directory) / "output.bin"
			command = [str(SCC), "--stdin", "-o", str(output)] + list(options)
			result = subprocess.run(command, input=source, text=True, capture_output=True)
			self.assertEqual(
				result.returncode,
				0,
				"SCC command failed:\n%s\n%s" % (" ".join(command), result.stderr + result.stdout),
			)
			self.assertTrue(output.is_file(), "SCC did not create the requested output")
			return output.read_bytes()

	def test_issue_563_nested_aggregate_members_through_pointer(self):
		source = (pathlib.Path(__file__).parent / "issue_563.c").read_text()
		for optimization in ("default", "-O0"):
			with self.subTest(optimization=optimization):
				options = ["--arch", "x64", "--platform", "none", "--allow-return", "-f", "bin"]
				if optimization != "default":
					options.append(optimization)
				self.compile(source, *options)

	def test_issue_2181_predefined_windows_x64_apis(self):
		# The Windows runtime already declares both MessageBoxA and exit. A main
		# function that returns must compile without redeclaring either prototype.
		self.compile(
			'int main() { MessageBoxA(NULL, "Hello", "Hello World.", 0); return 0; }\n',
			"--arch", "x64", "--platform", "windows", "-f", "bin",
		)

	def test_issue_2181_compatible_import_redeclaration(self):
		# Keep accepting the original documented example as source compatibility
		# for callers that copied its now-redundant import declaration.
		self.compile(
			"int __stdcall MessageBoxA(HANDLE hwnd, const char* msg, const char* title, "
			"uint32_t flags) __import(\"user32\");\n"
			'int main() { MessageBoxA(NULL, "Hello", "Hello World.", 0); return 0; }\n',
			"--arch", "x64", "--platform", "windows", "-f", "bin",
		)

	def test_issue_2391_x64_pie_uses_full_width_address_math(self):
		source = (
			"char global_value;\n"
			"char* get_global(void) { return &global_value; }\n"
			"int main(void) { return *get_global(); }\n"
		)
		for output_format, target_platform in (("bin", "none"), ("elf", "linux")):
			with self.subTest(output_format=output_format):
				with tempfile.TemporaryDirectory(prefix="scc-x64-address-") as directory:
					path = pathlib.Path(directory)
					output = path / ("output." + output_format)
					map_file = path / "output.map"
					command = [
						str(SCC), "--stdin", "--arch", "x64", "-m64", "--platform", target_platform,
						"--allow-return", "--pie", "-O0", "-f", output_format, "--map", str(map_file),
						"-o", str(output),
					]
					result = subprocess.run(command, input=source, text=True, capture_output=True)
					self.assertEqual(
						result.returncode,
						0,
						"SCC command failed:\n%s\n%s" % (" ".join(command), result.stderr + result.stdout),
					)

					addresses = {}
					for line in map_file.read_text().splitlines():
						address, name = line.split(None, 1)
						addresses[name] = int(address, 16)

					code = output.read_bytes()
					if output_format == "elf":
						function_offset = elf_virtual_address_to_file_offset(code, addresses["get_global"])
						function_end = elf_virtual_address_to_file_offset(code, addresses["main"])
					else:
						function_offset = addresses["get_global"]
						function_end = addresses["main"]
					function = code[function_offset:function_end]

					# The original failure truncated both the frame pointer setup and the
					# global address calculation to 32 bits. Accept either MOV encoding,
					# but require its REX.W prefix.
					frame_setup = function[:8]
					self.assertTrue(
						b"\x48\x8b\xec" in frame_setup or b"\x48\x89\xe5" in frame_setup,
						"get_global did not set up rbp from rsp with a 64-bit MOV",
					)

					lea_offset = None
					for index in range(len(function) - 6):
						# REX.W + LEA with a mod=00, r/m=101 ModR/M byte is a 64-bit
						# RIP-relative address calculation.
						if ((function[index] & 0xf8) == 0x48 and
							function[index + 1] == 0x8d and
							(function[index + 2] & 0xc7) == 0x05):
							lea_offset = index
							break

					self.assertIsNotNone(lea_offset, "get_global did not use a 64-bit RIP-relative LEA")
					displacement = struct.unpack_from("<i", function, lea_offset + 3)[0]
					lea_end = addresses["get_global"] + lea_offset + 7
					self.assertEqual(
						lea_end + displacement,
						addresses["global_value"],
						"RIP-relative LEA does not resolve to the global variable",
					)


def main():
	global SCC
	if len(sys.argv) != 2:
		print("usage: %s /path/to/scc" % pathlib.Path(sys.argv[0]).name, file=sys.stderr)
		return 2

	SCC = pathlib.Path(sys.argv[1]).resolve()
	if not SCC.is_file():
		print("SCC executable does not exist: %s" % SCC, file=sys.stderr)
		return 2

	program = sys.argv[0]
	sys.argv[:] = [program]
	return 0 if unittest.main(exit=False).result.wasSuccessful() else 1


if __name__ == "__main__":
	sys.exit(main())
