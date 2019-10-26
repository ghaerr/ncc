int main(void)
{
#ifdef __x86_64__
	long a = 0x00001000000000ul;
	return a != (1l << 36);
#endif
	return 0;
}
