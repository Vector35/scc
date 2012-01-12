int fputc(int ch, FILE* fp)
{
	return write((int)fp, &ch, 1);
}

int fputs(const char* str, FILE* fp)
{
	return write((int)fp, str, strlen(str));
}

int puts(const char* str)
{
	return fputs(str, stdout);
}

int fgetc(FILE* fp)
{
	uint8_t ch;
	if (read((int)fp, &ch, 1) <= 0)
		return EOF;
	return ch;
}

char* fgets(char* result, int size, FILE* fp)
{
	if (size <= 0)
		return NULL;
	int offset;
	for (offset = 0; offset < (size - 1); offset++)
	{
		int ch = fgetc(fp);
		if (ch == EOF)
		{
			if (offset == 0)
				return NULL;
			break;
		}

		result[offset] = (char)ch;
		if (ch == '\n')
			break;
	}
	result[offset] = 0;
	return result;
}

static void fprintf_output(void* ctxt, char ch)
{
	if (ch != 0)
		fputc(ch, (FILE*)ctxt);
}

int printf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	return __vaprintf(fprintf_output, stdout, fmt, va);
}

int fprintf(FILE* fp, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	return __vaprintf(fprintf_output, fp, fmt, va);
}

int vprintf(const char* fmt, va_list va)
{
	return __vaprintf(fprintf_output, stdout, fmt, va);
}

int vfprintf(FILE* fp, const char* fmt, va_list va)
{
	return __vaprintf(fprintf_output, fp, fmt, va);
}

