#include <stdio.h>
#include <unistd.h>

void main()
{
    // write(1, "This works!\n", 12);
    // write(1, "Again\n", 6);
    // puts("Hello from the program\n");
    // printf("Hello, world\n");
    // write(1, "After\n", 6);
    
    // static volatile int data_canary = 0xCAFEBABE;  /* .data  */
    // static volatile int bss_canary;                /* .bss   */

    // if (data_canary == 0xCAFEBABE) {
    //     write(1, "Was cafe\n", 9);
    // }

    // if (bss_canary == 0) {
    //     write(1, "was bss 0", 10);
    // }

    // put_hex((unsigned long)data_canary);        /* want 0xcafebabe */
    // put_hex((unsigned long)bss_canary);         /* want 0x0        */
    // put_hex((unsigned long)_impure_ptr);        /* want &impure_data */

    printf("Hello, world\n");


    for (;;)
        ;
}
