// The implementation of Communicating Sequential Processes by Hoare
//
// Matrix Multiplication
//
// Author: Li Mingyang @ EECS.pku

#include <inc/lib.h>

// The processes in paper is numbered as follow:
//	(1)	2	3	4	(5)
//	6	7	8	9	10
//	11	12	13	14	15
//	16	17	18	19	20
//	(21)	22	23	24	(25)
// 1, 5, 21, 25 is useless.

envid_t envids[26];
int array[3][3] = {{1, 2, 1}, {4, 5, 10}, {12, 8, 9}};
// We don't have shell-like console now, so we use fixed matirx
// and fixed vector (1, 1, 1)

void matrixmulti(int i){
	int c, r1, r2, rank, row, j;
	envid_t id;
	for (j = 0; j < 25; j++){
		c = ipc_recv(&id, 0, 0);
		envids[j] = c;
	}// We accept "envid" from parent process
	switch (i){
		case 2:
		case 3:
		case 4:// North side processes
			ipc_send(envids[i + 5], 0, 0, 0);
			return;
		case 6:
		case 11:
		case 16:// East side processes
			ipc_send(envids[i + 1], 1, 0, 0);
			return;
		case 10:
		case 15:
		case 20:// West side processes
			ipc_recv(&id, 0, 0);
			return;
		case 22:
		case 23:
		case 24:// South side processes
			c = ipc_recv(&id, 0, 0);
			cprintf("The output vector %d is: %d\n", i - 21, c);
			return;
		case 1:
		case 5:
		case 21:
		case 25:// Useless processes
			return;
		default:	// Center processes
			rank = ((i - 7) / 5);
			row = i - rank * 5 - 7;
			r1 = ipc_recv(&id, 0, 0);
			if(id == envids[i - 1]){
				ipc_send(envids[i + 1], r1, 0, 0);
				r2 = ipc_recv(&id, 0, 0);
				r2 += array[rank][row] * r1;
				ipc_send(envids[i + 5], r2, 0, 0);
			} else {
				r2 = ipc_recv(&id, 0, 0);
				ipc_send(envids[i + 1], r2, 0, 0);
				r1 += array[rank][row] * r2;
				ipc_send(envids[i + 5], r1, 0, 0);
			}
			return;
	}
}

void
umain(int argc, char **argv)
{
	int i, j, r;
	cprintf("The Matrix is:\n");
	for(i = 0; i < 3; i ++){
		cprintf("%d %d %d\n", array[i][0], array[i][1], array[i][2]);
	}
	cprintf("The Vector is: (1, 1, 1)\n");

	for (i = 1; i <= 25; i ++){
		if (i == 1 || i == 5 || i == 21 || i == 25) {
			envids[i] = -1;
			continue;
		}
		if ((r = fork()) == 0){
			envids[i] = sys_getenvid();
			matrixmulti(i);
			return;
		}else {
			envids[i] = r;
		}
	}
	for (i = 0; i < 25; i ++){
		for (j = 2; j < 25; j ++){
			if (j == 5 || j == 21) continue;
			ipc_send(envids[j], envids[i], 0, 0);
		}
	}// This loop is for sending "envids" array to subprocess
	return;
}