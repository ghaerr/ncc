/* Reported by Morten Brøns-Pedersen */
int f(int a)
{
	return 1 >= a;
}
int main() {
	return f(1) == 0;
}
