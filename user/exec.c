

#include <inc/x86.h>
#include <inc/lib.h>

void umain(int argc, char **argv) {
	if (fork() > 0) {
		cprintf("ok\n");
		exec("/echo", NULL);
	}
	return;
}