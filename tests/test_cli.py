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

import importlib.util
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest import mock


SCC = None
SOURCE_ROOT = pathlib.Path(__file__).resolve().parents[1]
DEFAULT_ARCH_WARNING = (
	"warning: no target architecture specified; defaulting to 32-bit x86 "
	"(use --arch, -m32, or -m64 to specify a target)"
)


class CliTests(unittest.TestCase):
	def setUp(self):
		self.temp_directory = tempfile.TemporaryDirectory(prefix="scc-cli-")
		self.temp_path = pathlib.Path(self.temp_directory.name)
		self.output_index = 0

	def tearDown(self):
		self.temp_directory.cleanup()

	def run_scc(self, *options, source=None):
		command = [str(SCC)] + list(options)
		return subprocess.run(command, input=source, text=True, capture_output=True)

	def compile(self, source="void main(void) {}\n", *options, with_map=False):
		self.output_index += 1
		stem = "output-%d" % self.output_index
		output = self.temp_path / (stem + ".bin")
		command = ["--stdin", "--format", "bin", "-o", str(output)] + list(options)
		map_file = None
		if with_map:
			map_file = self.temp_path / (stem + ".map")
			command += ["--map", str(map_file)]

		result = self.run_scc(*command, source=source)
		self.assertEqual(
			result.returncode,
			0,
			"SCC command failed:\n%s\n%s" % (" ".join([str(SCC)] + command), result.stderr + result.stdout),
		)
		self.assertTrue(output.is_file(), "SCC did not create the requested output")

		addresses = {}
		if map_file is not None:
			for line in map_file.read_text().splitlines():
				address, name = line.split(None, 1)
				addresses[name] = int(address, 16)

		return output.read_bytes(), addresses, result

	def test_help_omits_unavailable_features(self):
		result = self.run_scc("--help")
		self.assertEqual(result.returncode, 0)
		help_text = result.stderr
		for unavailable in (
			"__initial_", "--base ", "--base-reg", "--decoder", "--encoder",
			"--preserve", "--return-reg", "--return-high-reg",
		):
			with self.subTest(unavailable=unavailable):
				self.assertNotIn(unavailable, help_text)

		for available in (
			"--blacklist", "--concat", "quark", "Default: x86 (--exec auto-selects on x86/x64 hosts)",
		):
			with self.subTest(available=available):
				self.assertIn(available, help_text)

	def test_documentation_matches_available_features(self):
		for filename in ("scc.txt", "scc.html"):
			document = (SOURCE_ROOT / "docs" / filename).read_text()
			for unavailable in (
				"__initial_", "--base ", "--base-reg", "--decoder", "--encoder",
				"--preserve", "--return-reg", "--return-high-reg",
			):
				with self.subTest(filename=filename, unavailable=unavailable):
					self.assertNotIn(unavailable, document)

		for filename in ("issues.txt", "issues.html"):
			document = (SOURCE_ROOT / "docs" / filename).read_text()
			with self.subTest(filename=filename):
				self.assertNotIn("Blacklisted code generation is not implemented", document)

		for filename in ("python.txt", "python.html"):
			document = (SOURCE_ROOT / "docs" / filename).read_text()
			with self.subTest(filename=filename):
				self.assertIn("retained for compatibility", document)
				self.assertIn("but are not implemented", document)

	def test_blacklist_is_enforced(self):
		code, _, _ = self.compile(
			"void main(void) {}\n", "--arch", "x86", "--platform", "linux", "--blacklist", "0",
		)
		self.assertNotIn(0, code)

	def test_python_wrapper_uses_markov_chain_file_option(self):
		spec = importlib.util.spec_from_file_location("scc_wrapper_under_test", SOURCE_ROOT / "__init__.py")
		wrapper = importlib.util.module_from_spec(spec)
		spec.loader.exec_module(wrapper)

		process = mock.Mock()
		process.returncode = 0
		process.communicate.return_value = (b"compiled", b"")
		with mock.patch.object(wrapper.subprocess, "Popen", return_value=process) as popen:
			result, error = wrapper.compile_source(b"void main(void) {}\n", markov_chain="instructions.bin")

		self.assertEqual(result, b"compiled")
		self.assertEqual(error, b"")
		command = popen.call_args[0][0]
		self.assertNotIn("--markov-chain", command)
		option = command.index("--markov-chain-file")
		self.assertEqual(command[option + 1], "instructions.bin")

	def test_implicit_architecture_warns_and_uses_x86(self):
		implicit, _, implicit_result = self.compile(
			"void main(void) {}\n", "--platform", "none", "--allow-return",
		)
		explicit, _, explicit_result = self.compile(
			"void main(void) {}\n", "--arch", "x86", "--platform", "none", "--allow-return",
		)

		self.assertIn(DEFAULT_ARCH_WARNING, implicit_result.stderr)
		self.assertNotIn(DEFAULT_ARCH_WARNING, explicit_result.stderr)
		self.assertEqual(implicit, explicit)

	def test_invalid_architecture_stops_before_compilation(self):
		output = self.temp_path / "invalid-architecture.bin"
		result = self.run_scc(
			"--stdin", "--arch", "not-a-real-architecture", "--format", "bin", "-o", str(output),
			source="void main(void) {}\n",
		)
		self.assertNotEqual(result.returncode, 0)
		self.assertIn("error: unsupported architecture 'not-a-real-architecture'", result.stderr)
		self.assertFalse(output.exists())

	def test_architecture_shortcuts_are_explicit(self):
		for option in ("-m32", "-m64"):
			with self.subTest(option=option):
				_, _, result = self.compile(
					"void main(void) {}\n", option, "--platform", "none", "--allow-return",
				)
				self.assertNotIn(DEFAULT_ARCH_WARNING, result.stderr)

	def test_exec_uses_native_architecture_without_executing(self):
		common = ("--stdin", "--platform", "none", "--allow-return", "--max-length", "1")
		implicit = self.run_scc(*common, "--exec", source="void main(void) {}\n")
		self.assertNotEqual(implicit.returncode, 0)

		for bits in ("-m32", "-m64"):
			with self.subTest(bits=bits):
				exec_first = self.run_scc(
					*common, "--exec", bits, source="void main(void) {}\n",
				)
				bits_first = self.run_scc(
					*common, bits, "--exec", source="void main(void) {}\n",
				)
				self.assertEqual(exec_first.returncode, bits_first.returncode)
				self.assertEqual(exec_first.stderr, bits_first.stderr)

		if "unable to select a native x86/x64 target for --exec" in implicit.stderr:
			self.assertNotIn("Output is ", implicit.stderr)
			return

		self.assertTrue(
			("using native x86 for --exec" in implicit.stderr) or
			("using native x64 for --exec" in implicit.stderr),
			"implicit --exec target selection was silent",
		)
		self.assertIn("error: unable to satisfy size constraint", implicit.stderr)
		implicit_size = next(
			int(line.split()[2]) for line in implicit.stderr.splitlines() if line.startswith("Output is ")
		)
		explicit_sizes = []
		for architecture in ("x86", "x64"):
			explicit = self.run_scc(
				*common, "--arch", architecture, "--exec", source="void main(void) {}\n",
			)
			self.assertIn("error: unable to satisfy size constraint", explicit.stderr)
			explicit_sizes.append(next(
				int(line.split()[2]) for line in explicit.stderr.splitlines() if line.startswith("Output is ")
			))
		self.assertIn(implicit_size, explicit_sizes, "implicit --exec size matched neither x86 nor x64")

	def test_concat_jumps_to_output_end_instead_of_exiting(self):
		for platform in ("linux", "none"):
			with self.subTest(platform=platform):
				code, addresses, _ = self.compile(
					"int main(void) { return 0; }\n", "--arch", "x86", "--platform", platform, "--concat",
					with_map=True,
				)

				self.assertEqual(addresses["__end"], len(code))
				self.assertNotIn(b"\xcd\x80", code, "concatenation path still contains Linux syscall(exit)")
				self.assertGreaterEqual(len(code), 2)
				self.assertEqual(code[-2], 0xff)
				self.assertEqual(code[-1] & 0xf8, 0xe0, "output does not end in an indirect x86 jump")

	def test_concat_follows_branching_global_initializers(self):
		source = (
			"int condition;\n"
			"int initialized = condition ? 1 : 2;\n"
			"int main(void) { return initialized; }\n"
		)
		code, addresses, _ = self.compile(
			source, "--arch", "x86", "--platform", "none", "--concat", "-O0", with_map=True,
		)
		self.assertEqual(addresses["__end"], len(code))

	def test_concat_padding_keeps_end_at_true_output_boundary(self):
		for architecture, max_length, seed in (("x86", 100, 1), ("x86", 129, 7), ("arm", 129, 7)):
			with self.subTest(architecture=architecture, max_length=max_length, seed=seed):
				code, addresses, _ = self.compile(
					"int main(void) { return 0; }\n", "--arch", architecture, "--platform", "none", "--concat",
					"--pad", "--max-length", str(max_length), "--seed", str(seed), with_map=True,
				)
				self.assertEqual(len(code), max_length)
				self.assertEqual(addresses["__end"], len(code))


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
