typedef struct
{
	char* ptr;
	size_t len;
} sprintf_output_context;

char* strcpy(char* dest, const char* src)
{
	return (char*)memcpy(dest, src, strlen(src) + 1);
}

char* strncpy(char* dest, const char* src, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		char ch = src[i];
		dest[i] = ch;
		if (ch == 0)
			break;
	}
	return dest;
}

size_t __strlen(const char* str)
{
	size_t result;
	for (result = 0; *str; str++)
		result++;
	return result;
}

int strcmp(const char* a, const char* b)
{
	for (size_t i = 0; ; i++)
	{
		int diff = a[i] - b[i];
		if (diff != 0)
			return diff;
		if (a[i] == 0)
			return 0;
	}
}

char* strcat(char* dest, const char* src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
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

static void output_pad(void (*output)(void* ctxt, char ch), void* ctxt, size_t padding)
{
	for (size_t i = 0; i < padding; i++)
		output(ctxt, ' ');
}

static void output_str(void (*output)(void* ctxt, char ch), void* ctxt, const char* str)
{
	char ch;
	while ((ch = *(str++)) != 0)
		output(ctxt, ch);
}

static void output_pad_str(void (*output)(void* ctxt, char ch), void* ctxt, const char* str, bool left, size_t width)
{
	size_t len = strlen(str);
	char ch;

	if (len < width)
	{
		size_t pad = width - len;
		if (left)
		{
			output_str(output, ctxt, str);
			output_pad(output, ctxt, pad);
		}
		else
		{
			output_pad(output, ctxt, pad);
			output_str(output, ctxt, str);
		}
	}
	else
	{
		output_str(output, ctxt, str);
	}
}

static void output_uint(void (*output)(void* ctxt, char ch), void* ctxt, uint32_t val, char prefix, bool left, size_t width, size_t prec)
{
	char str[24];
	char* pos = &str[24];
	*(--pos) = 0;

	if (prec > 23)
		prec = 23;

	if (val == 0)
		*(--pos) = '0';
	else
	{
		while (val != 0)
		{
			*(--pos) = (char)((val % 10) + '0');
			val /= 10;
		}
	}

	size_t len = strlen(pos);
	size_t prefixLen = prefix ? 1 : 0;
	while ((len + prefixLen) < prec)
	{
		*(--pos) = '0';
		len++;
	}

	if (prefix)
		*(--pos) = prefix;

	output_pad_str(output, ctxt, pos, left, width);
}

static void output_int(void (*output)(void* ctxt, char ch), void* ctxt, int32_t val, bool left, size_t width, size_t prec)
{
	if (val < 0)
		output_uint(output, ctxt, -val, '-', left, width, prec);
	else
		output_uint(output, ctxt, val, 0, left, width, prec);
}

static void output_hex(void (*output)(void* ctxt, char ch), void* ctxt, size_t val, bool left, size_t width, size_t prec, bool caps)
{
	char str[24];
	char* pos = &str[24];
	*(--pos) = 0;

	if (prec > 23)
		prec = 23;

	if (val == 0)
		*(--pos) == 0;
	else
	{
		while (val != 0)
		{
			char digit = (char)(val & 0xf);
			if (digit < 10)
				*(--pos) = digit + '0';
			else if (caps)
				*(--pos) = digit + 'A' - 10;
			else
				*(--pos) = digit + 'a' - 10;
			val >>= 4;
		}
	}

	size_t len = strlen(pos);
	while (len < prec)
	{
		*(--pos) = '0';
		len++;
	}

	output_pad_str(output, ctxt, pos, left, width);
}

int __vaprintf(void (*output)(void* ctxt, char ch), void* ctxt, const char* fmt, va_list va)
{
	int outLen = 0;
	char ch;
	while ((ch = *(fmt++)) != 0)
	{
		if (ch == '%')
		{
			size_t width = 0;
			size_t prec = 0;
			bool left = false;
			bool dot = false;
			while ((ch = *(fmt++)) != 0)
			{
				if (ch == '-')
					left = true;
				else if (ch == '.')
					dot = true;
				else if ((ch >= '0') && (ch <= '9'))
				{
					if (dot)
					{
						prec *= 10;
						prec += ch - '0';
					}
					else
					{
						width *= 10;
						width += ch - '0';
					}
				}
				else if (ch == 's')
				{
					char* str = va_arg(va, char*);
					output_pad_str(output, ctxt, str, left, width);
					break;
				}
				else if (ch == 'd')
				{
					ssize_t val = va_arg(va, int32_t);
					output_int(output, ctxt, val, left, width, prec);
					break;
				}
				else if (ch == 'u')
				{
					size_t val = va_arg(va, uint32_t);
					output_uint(output, ctxt, val, 0, left, width, prec);
					break;
				}
				else if (ch == 'x')
				{
					size_t val = va_arg(va, size_t);
					output_hex(output, ctxt, val, left, width, prec, false);
					break;
				}
				else if (ch == 'X')
				{
					size_t val = va_arg(va, size_t);
					output_hex(output, ctxt, val, left, width, prec, true);
					break;
				}
				else
				{
					output(ctxt, ch);
					break;
				}
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

int atoi(const char* str)
{
	int result = 0;
	bool neg = false;
	char ch = *(str++);
	if (ch == '-')
	{
		neg = true;
		ch = *(str++);
	}

	do
	{
		result *= 10;
		result += ch - '0';
		ch = *(str++);
	} while ((ch >= '0') && (ch <= '9'));

	if (neg)
		result = -result;
	return result;
}

