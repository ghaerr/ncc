#define ELF32_ST_INFO(bind, type)	(((bind) << 4) + ((type) & 0xf))
#define ELF64_ST_INFO(bind, type)	ELF32_ST_INFO ((bind), (type))
#define ELF_ST_INFO			ELF64_ST_INFO

int main(void)
{
	if (ELF_ST_INFO(1, 2) != 0x12)
		return 1;
	return 0;
}
