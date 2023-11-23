#define HASH_BITS	13
#define BITS		16

#if HASH_BITS > BITS-1
	error: this should not be parsed!
#endif

int main(void)
{
	return 0;
}
