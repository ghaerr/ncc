#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int ac, char **av)
{
    write(1, "NEATCC DEMO!\n", 13);
    while (ac-- > 0)
        printf("%s ", *av++);
    printf("\n");
    return 0;
}
