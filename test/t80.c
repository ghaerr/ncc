int a;

void f(int v)
{
	a = v;
}

int main(void)
{
	a ? f(2) : f(1);
	return a != 1;
}
