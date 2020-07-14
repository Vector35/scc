#!/usr/bin/env python3
import sys

data = open(sys.argv[1], 'rb').read()
fout = open(sys.argv[2], 'w')

name = sys.argv[3].replace("/", "_").replace("\\", "_").replace(".", "_")

fout.write("unsigned char %s[] = {" % name)

for i in range(0, len(data)):
	if ((i % 16) == 0) and (i > 0):
		fout.write("\n")
	val = data[i]
	if type(val) is not int:
		val = ord(data[i])
	fout.write("0x%.2x" % val)
	if (i + 1) < len(data):
		fout.write(", ")

fout.write("};\n")
fout.write("unsigned int %s_len = %d;\n" % (name, len(data)))
fout.close()
