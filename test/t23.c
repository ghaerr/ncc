int main(void)
{
	int ret = 2;
	goto l1;
l1:
	ret = 0;
	goto end;
	ret = 5;
end:
	return ret;
}
