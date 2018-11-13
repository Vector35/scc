// Copyright (c) 2014 Rusty Wagner
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

#ifdef WIN32
#define YY_NO_UNISTD_H
#endif
#include <string>
#include <stdio.h>
#include <string.h>
#include "PreprocessState.h"
#include "ParserState.h"
#include "CodegenParser.h"
#include "CodegenLexer.h"
#include "OutputGenerator.h"

using namespace std;


extern int Codegen_parse(ParserState* state);
extern void Codegen_set_lineno(int line, void* yyscanner);


int main(int argc, char* argv[])
{
	string inFile, outFile;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-o"))
		{
			if ((++i) >= argc)
			{
				fprintf(stderr, "error: expected output filename\n");
				return 1;
			}

			outFile = argv[i];
		}
		else
		{
			inFile = argv[i];
		}
	}

	if (inFile.size() == 0)
	{
		fprintf(stderr, "error: input file not specified\n");
		return 1;
	}
	if (outFile.size() == 0)
	{
		fprintf(stderr, "error: output file not specified\n");
		return 1;
	}

	FILE* fp = fopen(inFile.c_str(), "rb");
	if (!fp)
	{
		fprintf(stderr, "%s: error: file not found\n", inFile.c_str());
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* data = new char[size + 2];
	if (fread(data, 1, size, fp) != (size_t)size)
	{
		fprintf(stderr, "%s: error: unable to read file\n", inFile.c_str());
		return 1;
	}
	data[size++] = '\n';
	data[size] = 0;
	fclose(fp);

	string source = data;
	delete[] data;

	string preprocessed;
	if (!PreprocessState::PreprocessSource(source, inFile, preprocessed, NULL))
		return 1;

	string out;
	yyscan_t scanner;
	Codegen_lex_init(&scanner);
	ParserState parser(inFile, scanner);

	YY_BUFFER_STATE buf = Codegen__scan_string(preprocessed.c_str(), scanner);
	Codegen__switch_to_buffer(buf, scanner);
	Codegen_set_lineno(1, scanner);

	bool ok = true;
	if (Codegen_parse(&parser) != 0)
		ok = false;
	if (parser.HasErrors())
		ok = false;

	if (!ok)
		return 1;

	parser.ExpandTempRegisterClasses();

	if (parser.GetArchName().size() == 0)
	{
		fprintf(stderr, "%s: error: no architecture name specified\n", inFile.c_str());
		return 1;
	}

	if (!parser.GetDefaultRegisterClass())
	{
		fprintf(stderr, "%s: error: no default register class specified\n", inFile.c_str());
		return 1;
	}

	OutputGenerator gen(&parser);
	if (!gen.Generate(out))
		return 1;

	fp = fopen(outFile.c_str(), "w");
	if (!fp)
	{
		fprintf(stderr, "%s: error: unable to write output file\n", outFile.c_str());
		return 1;
	}

	fwrite(out.c_str(), 1, strlen(out.c_str()), fp);
	fclose(fp);
	return 0;
}

