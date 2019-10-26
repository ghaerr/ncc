#define A

#ifndef A
	#ifndef C
		#define B	10
	#else
		#define B	20
	#endif
#else
	#if defined(A) && !defined(B)
		#define B	30
	#endif
#endif

#define D(a, b)		((a) + (b))

int main(void)
{
	if (B != 30)
		return 1;
	if (D(B, 10) != 40)
		return 2;
	return 0;
}
