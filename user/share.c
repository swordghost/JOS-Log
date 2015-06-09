#include<inc/lib.h>

int share = 0;

void umain(int argc, char **argv) {
	cprintf("I'm parent, the value of share = %d\n", share);
	share ++;
	if (sfork() == 0)
		cprintf("I'm child, the value of share = %d\n", share);
}

