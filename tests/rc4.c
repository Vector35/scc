int main()
{
	rc4_state_t rc4;
	char* key = "thereisnocowlevel";
	rc4_init(&rc4, key, strlen(key));
	char* str = "pwniesinstead";
	size_t len = strlen(str);
	rc4_crypt(&rc4, str, len);
	write(1, str, len);
	return 0;
}
