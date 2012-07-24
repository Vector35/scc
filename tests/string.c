int main()
{
	char* str = "The quick brown fox jumps over the lazy dog!";
	printf("%d\n", strlen(str));
	printf("%d\n", strchr(str, 'e') - str);
	printf("%d\n", strchr(str, 'T') - str);
	printf("%d\n", strchr(str, '!') - str);
	printf("%d\n", strrchr(str, 'e') - str);
	printf("%d\n", strrchr(str, 'T') - str);
	printf("%d\n", strrchr(str, '!') - str);

	if (strchr(str, '@'))
		printf("strchr bad\n");
	if (strrchr(str, '@'))
		printf("strrchr bad\n");

	memmove(str, &str[4], 5);
	printf("%s\n", str);

	memmove(&str[11], &str[10], 5);
	printf("%s\n", str);

	return 0;
}

