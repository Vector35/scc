struct Info;
typedef uint32_t (*CallbackFunction)(const struct Info* info, uint32_t value);

typedef struct Info
{
	uint32_t value;
	uint32_t mask;
	CallbackFunction function;
} Info;

uint32_t g_global;

int BitCount(uint32_t value)
{
	int result = 0;
	for (size_t i = 0; i < sizeof(value) * 8; i++)
	{
		if (value & (1 << i))
			result++;
	}
	return result;
}

uint32_t Callback(const Info* info, uint32_t value)
{
	return value & info->mask;
}

int main(int argc, char* argv[])
{
	Info a;
	a.value = 42;
	a.mask = 0xffffffff;
	a.function = Callback;
	g_global = a.value;
	if (a.function(&a, a.value) != a.value)
	{
		uint32_t invert = a.mask ^ 0xffffffff;
		a.mask = invert;
	}
	return BitCount(a.value & a.mask);
}

