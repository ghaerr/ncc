int main(void)
{
	unsigned short a = 0x8001;
	unsigned short b = 0x8002;
	int c = a + b;
	if (c != 0x00010003)
		return 1;
	if (sizeof(a - b) != 4)
		return 2;
	return 0;
}
