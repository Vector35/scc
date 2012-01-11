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

