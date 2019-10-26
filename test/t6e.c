#define ABC	"ABC"

char abc[80] = {ABC};

int main(void)
{
	if (abc[0] != 'A' || abc[1] != 'B' || abc[2] != 'C' || abc[3] != '\0')
		return 1;
	return 0;
}
