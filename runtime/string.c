typedef struct
{
	char* ptr;
	size_t len;
} sprintf_output_context;

char* strcpy(char* dest, const char* src)
{
	return (char*)memcpy(dest, src, strlen(src) + 1);
}

size_t strlen(const char* str)
{
	size_t result;
	for (result = 0; *str; str++)
		result++;
	return result;
}

static void sprintf_output(void* ctxt, char ch)
{
	sprintf_output_context* out = (sprintf_output_context*)ctxt;
	if (out->len == 0)
		return;
	if ((ch != 0) && (out->len <= 1))
		return;
	*(out->ptr++) = ch;
	out->len--;
}

int sprintf(char* out, const char* fmt, ...)
{
	va_list va;
	sprintf_output_context ctxt;
	ctxt.ptr = out;
	ctxt.len = -1;
	va_start(va, fmt);
	return __vaprintf(sprintf_output, &ctxt, fmt, va);
}

int snprintf(char* out, size_t len, const char* fmt, ...)
{
	va_list va;
	sprintf_output_context ctxt;
	ctxt.ptr = out;
	ctxt.len = len;
	va_start(va, fmt);
	return __vaprintf(sprintf_output, &ctxt, fmt, va);
}

int vsprintf(char* out, const char* fmt, va_list va)
{
	sprintf_output_context ctxt;
	ctxt.ptr = out;
	ctxt.len = -1;
	return __vaprintf(sprintf_output, &ctxt, fmt, va);
}

int vsnprintf(char* out, size_t len, const char* fmt, va_list va)
{
	sprintf_output_context ctxt;
	ctxt.ptr = out;
	ctxt.len = len;
	return __vaprintf(sprintf_output, &ctxt, fmt, va);
}

int __vaprintf(void (*output)(void* ctxt, char ch), void* ctxt, const char* fmt, va_list va)
{
	int outLen = 0;
	char ch;
	while ((ch = *(fmt++)) != 0)
	{
		if (ch == '%')
		{
			ch = *(fmt++);
			if (ch == 's')
			{
				char* str = va_arg(va, char*);
				while ((ch = *(str++)) != 0)
					output(ctxt, ch);
			}
			else
			{
				output(ctxt, ch);
			}
		}
		else
		{
			output(ctxt, ch);
		}
	}

	output(ctxt, 0);
	return outLen;
}

