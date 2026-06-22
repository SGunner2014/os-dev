#include <stdio.h>
#include <unistd.h>

void main()
{
    write(1, "This works!\n", 12);
    write(1, "Again\n", 6);
    puts("Hello from the program\n");

    for (;;)
        ;
}
