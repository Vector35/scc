char* strcpy(char* dest, const char* src)
{
	char* start = dest;
	char ch;
	while ((ch = *(src++)) != 0)
		*(dest++) = ch;
	*dest = 0;
	return start;
}

size_t strlen(const char* str)
{
	size_t result;
	for (result = 0; *str; str++)
		result++;
	return result;
}

