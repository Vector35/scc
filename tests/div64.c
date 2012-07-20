void print64(uint64_t a)
{
	uint32_t high = (uint32_t)(a >> 32);
	uint32_t low = (uint32_t)a;
	printf("%.8x%.8x\n", high, low);
}

void test_unsigned(uint64_t a, uint64_t b)
{
	uint64_t c = a / b;
	uint64_t d = a % b;
	print64(c);
	print64(d);
}

void test_signed(int64_t a, int64_t b)
{
	int64_t c = a / b;
	int64_t d = a % b;
	print64(c);
	print64(d);
}

int main()
{
	test_unsigned(1337, 42);
	test_unsigned(0x4141414141414141, 7777);
	test_unsigned(0x4141414141414141, 0x0707070707070707);
	test_signed(1337, 42);
	test_signed(-1337, 42);
	test_signed(1337, -42);
	test_signed(-1337, -42);
	test_signed(0x4141414141414141, 7777);
	test_signed(-0x4141414141414141, 7777);
	test_signed(0x4141414141414141, -7777);
	test_signed(-0x4141414141414141, -7777);
	test_signed(0x4141414141414141, 0x0707070707070707);
	test_signed(-0x4141414141414141, 0x0707070707070707);
	test_signed(0x4141414141414141, -0x0707070707070707);
	test_signed(-0x4141414141414141, -0x0707070707070707);
	return 0;
}

