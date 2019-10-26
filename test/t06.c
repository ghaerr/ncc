int main(void)
{
	int a = 0x100 | 0x001;
	int b = 'a';
	return !(b == 'a' && a == 0x101);
}
