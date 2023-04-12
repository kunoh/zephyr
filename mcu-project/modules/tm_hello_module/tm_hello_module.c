#include <zephyr/kernel.h>
#include <stdio.h>

void print_hello(void)
{
	printf("Hello Trackman World!\n");
}

int return_arg(int arg)
{
    return arg;
}
