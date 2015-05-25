// program to cause a breakpoint trap

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("Before int 3\n");
	asm volatile("int $3");
	cprintf("After int 3\n");
	asm volatile("int $3");
	cprintf("Another int 3\n");
}

