#include <stdio.h>

int f(int x)
{
    printf("f(%d)\n", x);
    return x + 1;
}

int main(int argc, char *argv[])
{
    int x = 0;
    unsigned int cs;

    // Read the value of the control state.
    asm volatile("mov %%cs, %0" : "=r"(cs));

    // Print the value of the control state.
    printf("cs = %u\n", cs);

    // Call f() with a constant argument, and print the result.
    x = f(1);
    printf("f(1) = %d\n", x);

    return 0;
}