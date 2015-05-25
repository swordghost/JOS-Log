// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>
#include <kern/env.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Backtrace", mon_backtrace},
	{ "enablecolor", "Enable Color", mon_color},
	{ "showmappings", "Show the physical pages mapping at a range of virtual pages", mon_showmem},
	{ "permission", "Set, change, clear permissions", mon_permission},
	{ "dump", "Dump the contents of a range of memory", mon_dump},
	{ "c", "Continue execution untill next breakpoint", mon_continue},
	{ "si", "Single stepping", mon_singlestep},
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint32_t *ebp , *eip;
	cprintf( "Stack backtrace:\n" );
	ebp = ( uint32_t * ) read_ebp();
	while( ebp != 0 )
	{
		eip = ( uint32_t * ) ebp[1];
		cprintf( " ebp %08x eip %08x args %08x %08x %08x %08x %08x\n" ,
			ebp , eip , ebp[2] , ebp[3] , ebp[4] , ebp[5] , ebp[6] );
		struct Eipdebuginfo info;
		if( debuginfo_eip( ( uintptr_t )( eip ) , &info ) == 0 ){
		cprintf( "         %s:%d: %.*s+%d\n" , info.eip_file , info.eip_line 
			,info.eip_fn_namelen , info.eip_fn_name , ( int )eip -info.eip_fn_addr );
		}
		else{
		panic( "kern/monitor.c: mon_backtrace(): 80: " );
			return 0;
		}
		ebp = ( uint32_t * ) ebp[0];
	}
	return 0;
}

/* mon_color: control the color */
int
mon_color(int argc, char **argv, struct Trapframe *tf){
	extern int enable_color;
	if(enable_color){
		enable_color = 0;
	}
	else{
		enable_color = 1;
	}
	return 0;
}

/* mon_showmem: show the mapping pages 
	Usage: "showmappings address1 [address2]",1 or 2 address(es).
*/
int
mon_showmem(int argc, char **argv, struct Trapframe *tf){
	if(argc == 1 || argc > 3){
		cprintf("Command Format Error: 1 or 2 address(es) needed.\n");
		return 0;
	}
	uintptr_t va1 = 0, va2 = 0;
	if(argc == 2){
		va1 = ROUNDDOWN(strtol(argv[1], NULL, 0), PGSIZE);
		va2 = va1;
	}
	else if(argc == 3){
		va1 = ROUNDDOWN(strtol(argv[1], NULL, 0), PGSIZE);
		va2 = ROUNDDOWN(strtol(argv[2], NULL, 0), PGSIZE);
		if(va2 < va1){
			uintptr_t temp = va1;
			va1 = va2;
			va2 = temp;
		}
	}
	size_t i;
	pte_t* pageinfo;
	for(i = va1; i<=va2; i += PGSIZE){
		pageinfo = pgdir_walk(kern_pgdir, (void *)i, 0);
		cprintf("0x%x\t\t", i);
		if(!pageinfo){
			cprintf("This vitual page has not mapped!\n");
		}else{
			cprintf("0x%x\t\tPermission: ", PTE_ADDR(*pageinfo));
			if(*pageinfo & PTE_U){
				cprintf("User, ");
			}else{
				cprintf("Kern, ");
			}
			if(*pageinfo & PTE_W){
				cprintf("Write\n");
			}else{
				cprintf("Read\n");
			}
		}
	}
	return 0;
}

/* mon_permission: change, set, clear permission bits
	Usage: permission [address] [-U/-K] [-R/-W]
 */
int
mon_permission(int argc, char **argv, struct Trapframe *tf){
	if(argc != 4){
		cprintf("Command Format Error.\n");
		cprintf("Usage: permission [address] [-U/-K] [-R/-W]\n");
		return 0;
	}
	long addr = ROUNDDOWN(strtol(argv[1], NULL, 0), PGSIZE);
	pte_t *pageinfo = pgdir_walk(kern_pgdir, (void *)addr, 0);
	if(!pageinfo){
		cprintf("This vitual page has not mapped!\n");
		return 0;
	}
	if(strcmp(argv[2],"-U") == 0){
		if(strcmp(argv[3],"-R") == 0){
			*pageinfo |= PTE_U;
			*pageinfo &= ~PTE_W;
		}else if(strcmp(argv[3],"-W") == 0){
			*pageinfo |= PTE_U;
			*pageinfo |= PTE_W;
		}else{
			cprintf("Command Format Error.\n");
			cprintf("Usage: permission [address] [-U/-K] [-R/-W]\n");
			return -1;
		}
	}else if(strcmp(argv[2],"-K") == 0){
		if(strcmp(argv[3],"-R") == 0){
			*pageinfo &= ~PTE_U;
			*pageinfo &= ~PTE_W;
		}else if(strcmp(argv[3],"-W") == 0){
			*pageinfo &= ~PTE_U;
			*pageinfo |= PTE_W;
		}else{
			cprintf("Command Format Error.\n");
			cprintf("Usage: permission [address] [-U/-K] [-R/-W]\n");
			return -1;
		}
	}else{
		cprintf("Command Format Error.\n");
		cprintf("Usage: permission [address] [-U/-K] [-R/-W]\n");
		return -1;
	}
	cprintf("Change command successfully!\n");
	return 0;
}

/* mon_dump: Dump a range of memory given a virtual address or physical address.
	Usage: dump [-v/-p] [address] [num]*/
int
mon_dump(int argc, char **argv, struct Trapframe *tf){
	if(argc != 4){
		cprintf("Command Format Error.\n");
		cprintf("Usage: dump [-v/-p] [address] [num]\n");
		return 0;
	}
	long addr = ROUNDDOWN(strtol(argv[2], NULL, 0), 4);
	int num = strtol(argv[3], NULL, 0);
	if(strcmp(argv[1], "-p") == 0){
		if(PGNUM(addr) >= npages){
			cprintf("Physical address is out of range.\n");
			return 0;
		}
		addr = (int)KADDR(addr);
	}else if(strcmp(argv[1], "-v") == 0){
		int i;
		for(i = addr; i < addr + num; i += PGSIZE){
			pte_t *pageinfo = pgdir_walk(kern_pgdir, (void *)i, 0);
			if(!pageinfo){
				cprintf("The vitual page has not mapped!\n");
				return 0;
			}// For virtual addresses, check whether mapped first
		}
	}else{
		cprintf("Command Format Error.\n");
		cprintf("Usage: dump [-v/-p] [address] [num]\n");
		return 0;
	}
	int i;
	for(i = 0; i < num; i++){
		if(i % 4 == 0){
			cprintf("0x%x:\t", addr + i);
		}// print 4 bytes a line
		cprintf("0x%08x ", *(char *)(addr + i));
		if(i % 4 == 3){
			cprintf("\n");
		}
	}
	if(i % 4 == 0){
		cprintf("\n");
	}
	return 0;
}

int
mon_continue(int argc, char **argv, struct Trapframe *tf){
	if( argc != 1 ){
		cprintf("Command Format Error.\n");
		cprintf("Usage: c\n");
		return -1;
	}
	if( tf == NULL ){
		cprintf("Can't continue.\n");
		return -1;
	}
	if( tf -> tf_trapno != T_DEBUG && tf -> tf_trapno != T_BRKPT ){
		cprintf("Not in debug mode.\n");
		return -1;
	}
	// extern struct Env *curenv;
	tf -> tf_eflags &= ~FL_TF;
	cprintf("Continue successfully!\n");
	env_run(curenv);
	return 0;
}

int
mon_singlestep(int argc, char **argv, struct Trapframe *tf){
	if(argc != 1){
		cprintf("Command Format Error.\n");
		cprintf("Usage: si\n");
		return -1;
	}
	if( tf == NULL ){
		cprintf("Can't single step.\n");
		return -1;
	}
	if( tf -> tf_trapno != T_DEBUG && tf -> tf_trapno != T_BRKPT ){
		cprintf("Not in debug mode.\n");
		return -1;
	}
	// extern struct Env *curenv;
	tf -> tf_eflags |= FL_TF;
	cprintf("Single step successfully!\n");
	env_run(curenv);
	return 0;
}

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
