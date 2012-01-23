uint8_t shl_8(uint8_t val, uint8_t count)
{
	return val << count;
}

uint8_t shr_8(uint8_t val, uint8_t count)
{
	return val >> count;
}

int8_t sar_8(int8_t val, uint8_t count)
{
	return val >> count;
}

uint16_t shl_16(uint16_t val, uint8_t count)
{
	return val << count;
}

uint16_t shr_16(uint16_t val, uint8_t count)
{
	return val >> count;
}

int16_t sar_16(int16_t val, uint8_t count)
{
	return val >> count;
}

uint32_t shl_32(uint32_t val, uint8_t count)
{
	return val << count;
}

uint32_t shr_32(uint32_t val, uint8_t count)
{
	return val >> count;
}

int32_t sar_32(int32_t val, uint8_t count)
{
	return val >> count;
}

uint64_t shl_64(uint64_t val, uint8_t count)
{
	return val << count;
}

uint64_t shr_64(uint64_t val, uint8_t count)
{
	return val >> count;
}

int64_t sar_64(int64_t val, uint8_t count)
{
	return val >> count;
}

uint32_t reduce(uint64_t val)
{
	uint32_t result = (uint32_t)val;
	result ^= (uint32_t)(val >> 32);
	return result;
}

int main()
{
	printf("%d\n", shl_8(17, 2));
	printf("%d\n", shr_8(242, 5));
	printf("%d\n", sar_8(-42, 5));
	printf("%d\n", shl_16(1717, 2));
	printf("%d\n", shr_16(54242, 5));
	printf("%d\n", sar_16(-4242, 5));
	printf("%d\n", shl_32(17171717, 2));
	printf("%d\n", shr_32(2424242424, 5));
	printf("%d\n", sar_32(-42424242, 5));
	printf("%d\n", reduce(shl_64(1717171717171717, 2)));
	printf("%d\n", reduce(shr_64(8242424242424242424, 5)));
	printf("%d\n", reduce(sar_64(-4242424242424242, 5)));
	printf("%d\n", reduce(shl_64(1717171717171717, 32)));
	printf("%d\n", reduce(shr_64(8242424242424242424, 32)));
	printf("%d\n", reduce(sar_64(-4242424242424242, 32)));
	printf("%d\n", reduce(shl_64(1717171717171717, 42)));
	printf("%d\n", reduce(shr_64(8242424242424242424, 42)));
	printf("%d\n", reduce(sar_64(-4242424242424242, 42)));
	return 0;
}

