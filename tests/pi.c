char* pi(int n)
{
	int digits = n;
	int dec = 8;
	int extraDigits = dec + 8;
	int totalDigits = digits + extraDigits;
	int decimalIndex = totalDigits - dec;

	char* result = (char*)malloc(totalDigits);
	memset(result, 0, totalDigits);
	result[decimalIndex] = 4;

	int k = digits * 4;
	for (int i = 0; i < digits * 4; i++)
	{
		int carry = 0;
		for (int j = 0; j < totalDigits; j++)
		{
			int n = (result[j] * k) + carry;
			carry = n / 10;
			result[j] = (char)(n % 10);
		}

		carry = 0;
		int num = 2 * k + 1;
		for (int j = totalDigits - 1; j >= 0; j--)
		{
			int n = (result[j] + (carry * 10)) / num;
			carry = (result[j] + (carry * 10)) - (n * num);
			result[j] = (char)n;
		}

		result[decimalIndex] += 2;
		k--;
	}

	char* s = (char*)malloc(totalDigits);
	s[0] = '0' + result[decimalIndex];
	s[1] = '.';
	k = 2;
	for (int i = decimalIndex - 1; i >= 0; i--, k++)
		s[k] = '0' + result[i];
	s[k] = 0;
	return s;
}

int main()
{
	puts(pi(1000));
	puts("\n");
	return 0;
}
