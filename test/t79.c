#define NOFUNC		(int (*)()) 0

int main(void)
{
	void *v = NOFUNC;
	return v != 0;
}
