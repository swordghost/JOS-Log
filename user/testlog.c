
/* Test the log system */
#include <inc/lib.h>

#define Nbuf 17

void umain(int argc, char **argv) {
	int f, i, r;
	char buf[512], nbuf[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '\0'};
	if (argc == 1) {
		cprintf("Please use s or c to choose to begin the write or check the file.\n");
		return;
	}
	if (argv[1][0] == 's') {
		// write /big but don't close the file, so writing isn't commited
		if ((f = open("/big", O_WRONLY|O_CREAT)) < 0)
			panic("creat /big: %e", f);
		for (i = 0; i < Nbuf; ++i) {
			memset(buf, nbuf[i], sizeof(buf));
			if ((r = write(f, buf, sizeof(buf))) < 0)
				panic("write /big@%d: %e", i, r);
		}
		cprintf("test start\n");
		asm volatile("int $3");
	} else if (argv[1][0] == 'c') {
		// read /big and check the data
		if ((f = open("/big", O_RDONLY)) < 0)
			panic("read /big: %e", f);
		for (i = 0; i < Nbuf; ++i) {
			if ((r = readn(f, buf, sizeof(buf))) < 0)
				panic("read /big@%d: %e", i, r);
			if (r != sizeof(buf))
				panic("read /big from %d returned %d < %d bytes", i, r, sizeof(buf));
			if (*buf != nbuf[i] || buf[511] != nbuf[i])
				panic("%d, read /big from %c returned bad data %c|%c", i, nbuf[i], *buf, buf[511]);
		}
		close(f);
		cprintf("test pass!\n");
	} else {
		cprintf("Please use s or c to choose to begin the write or check the file.\n");
	}
}
