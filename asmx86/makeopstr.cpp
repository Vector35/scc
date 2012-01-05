#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <ctype.h>

using namespace std;


vector<string> operations;
vector<string> operands;


#ifndef WIN32
void _strlwr(char* str)
{
	for (size_t i = 0; str[i]; i++)
		str[i] = tolower(str[i]);
}
#endif


void AddOperation(const char* name)
{
	operations.push_back(name);
}


void AddOperand(const char* name)
{
	operands.push_back(name);
}


void ComputeString(const vector<string>& list, vector<char>& chars, vector<int>& offsets)
{
	chars.clear();
	offsets.clear();

	for (size_t i = 0; i < list.size(); i++)
	{
		// First try to find string in existing data
		if (chars.size() > list[i].size())
		{
			bool found = false;
			for (size_t j = 0; j < chars.size() - list[i].size(); j++)
			{
				if (chars[j + list[i].size()] != 0)
					continue;
				bool match = true;
				for (size_t k = 0; k < list[i].size(); k++)
				{
					if (chars[j + k] != list[i][k])
					{
						match = false;
						break;
					}
				}
				if (match)
				{
					offsets.push_back((int)j);
					found = true;
					break;
				}
			}
			if (found)
				continue;
		}

		// Add the string to the end of the data
		offsets.push_back((int)chars.size());
		for (size_t j = 0; j < list[i].size(); j++)
			chars.push_back(list[i][j]);
		chars.push_back(0);
	}
}


int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: makeopstr <header-file> <output-file>\n");
		return -1;
	}

	FILE* fp = fopen(argv[1], "r");
	if (!fp)
	{
		printf("Error: Unable to open input file\n");
		return -1;
	}

	bool operation = false;
	bool operand = false;
	while (!feof(fp))
	{
		char line[1024];
		if (!fgets(line, 1024, fp))
			break;

		if ((strstr(line, "enum InstructionOperation")) && (!strstr(line, "typedef")))
			operation = true;
		else if ((strstr(line, "enum OperandType")) && (!strstr(line, "typedef")))
			operand = true;
		else if (strstr(line, "}"))
		{
			operation = false;
			operand = false;
		}
		else if ((operation || operand) && (!strstr(line, "{")))
		{
			char part[1024];
			int i, j;
			for (i = 0, j = 0; line[i]; i++)
			{
				if ((line[i] == ' ') || (line[i] == '\t') || (line[i] == '\r') || (line[i] == '\n'))
					continue;
				else if (line[i] == '/')
					break;
				else if (line[i] == ',')
				{
					part[j++] = 0;
					_strlwr(part);
					if (part[strlen(part) - 1] == ')')
						part[strlen(part) - 1] = 0;
					if (operation)
					{
						if (!strcmp(part, "invalid"))
							AddOperation("");
						else
							AddOperation(part);
					}
					else
					{
						if ((!strcmp(part, "none")) || (!strcmp(part, "imm")) || (!strcmp(part, "mem")))
							AddOperand("");
						else if (!strncmp(part, "__x86_oper(reg_", 15))
							AddOperand(part + 15);
						else
						{
							printf("Error: Operand '%s' does not follow REG_* format\n", part);
							return -1;
						}
					}
					j = 0;
				}
				else if (line[i] == '=')
					part[j++] = 0;
				else
					part[j++] = line[i];
			}
			if (j != 0)
			{
				part[j++] = 0;
				_strlwr(part);
				if (part[strlen(part) - 1] == ')')
					part[strlen(part) - 1] = 0;
				if (operation)
				{
					if (!strcmp(part, "invalid"))
						AddOperation("");
					else
						AddOperation(part);
				}
				else
				{
					if ((!strcmp(part, "none")) || (!strcmp(part, "imm")) || (!strcmp(part, "mem")))
						AddOperand("");
					else if (!strncmp(part, "__x86_oper(reg_", 15))
						AddOperand(part + 15);
					else
					{
						printf("Error: Operand '%s' does not follow REG_* format\n", part);
						return -1;
					}
				}
			}
		}
	}
	fclose(fp);

	vector<char> str;
	vector<int> offset;
	ComputeString(operations, str, offset);

	fp = fopen(argv[2], "w");
	if (!fp)
	{
		printf("Error: Unable to open output file\n");
		return -1;
	}

	fprintf(fp, "static const char operationString[] = \"");
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == 0)
			fprintf(fp, "\\0");
		else
			fprintf(fp, "%c", str[i]);
	}
	fprintf(fp, "\";\n");
	fprintf(fp, "static const %s operationOffsets[] = {", (str.size() < 256) ? "uint8_t" : "uint16_t");
	for (size_t i = 0; i < offset.size(); i++)
	{
		if (i != 0)
			fprintf(fp, ", ");
		fprintf(fp, "%d", offset[i]);
	}
	fprintf(fp, "};\n\n");

	ComputeString(operands, str, offset);

	fprintf(fp, "static const char operandString[] = \"");
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == 0)
			fprintf(fp, "\\0");
		else
			fprintf(fp, "%c", str[i]);
	}
	fprintf(fp, "\";\n");
	fprintf(fp, "static const %s operandOffsets[] = {", (str.size() < 256) ? "uint8_t" : "uint16_t");
	for (size_t i = 0; i < offset.size(); i++)
	{
		if (i != 0)
			fprintf(fp, ", ");
		fprintf(fp, "%d", offset[i]);
	}
	fprintf(fp, "};\n\n");

	fclose(fp);
	return 0;
}

