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

	return 0;
}

