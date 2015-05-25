// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if((err & FEC_WR) == 0 || ((uvpd[ PDX(addr) ] & PTE_P) == 0 
		||(uvpt [ PGNUM(addr) ] & PTE_COW) == 0)){
		panic("fork.c/pgfault: illegal page fault.");
	}
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	addr = ROUNDDOWN(addr, PGSIZE);
	if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
		panic("fork.c/pgfault: sys_page_alloc: %e", r);
	memcpy(PFTEMP, addr, PGSIZE);
	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W)) < 0)
		panic("fork.c/pgfault: sys_page_map: %e", r);
	if ((r = sys_page_unmap(0, PFTEMP)) < 0)
		panic("fork.c/pgfault: sys_page_unmap: %e", r);
	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	// panic("duppage not implemented");
	void *addr = (void *)(pn * PGSIZE);
	if ((uvpt[PGNUM(addr)] & PTE_SHARE) != 0){
		int maskcode = uvpt[PGNUM(addr)] & PTE_SYSCALL;
		if((r = sys_page_map(0, addr, envid, addr, maskcode)) < 0)
			panic("fork.c/duppage: sys_page_map: %e\n", r);
	}else if ((uvpt[PGNUM(addr)] & PTE_W) != 0|| (uvpt[PGNUM(addr)] & PTE_COW) != 0){
		if((r = sys_page_map(0, addr, envid, addr, PTE_COW|PTE_P|PTE_U)) < 0)
			panic("fork.c/duppage: sys_page_map: %e\n", r);
		if((r = sys_page_map(0, addr, 0, addr, PTE_COW|PTE_P|PTE_U)) < 0)
			panic("fork.c/duppage: sys_page_map: %e\n", r);
	}else{
		if((r = sys_page_map(0, addr, envid, addr, PTE_P|PTE_U)) < 0)
			panic("fork.c/duppage: sys_page_map: %e\n", r);
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);

	envid_t envid;
	int addr;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("fork.c/fork: sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	for (addr = UTEXT; addr < UXSTACKTOP - PGSIZE; addr += PGSIZE){
		if (( uvpd[ PDX(addr) ] & PTE_P) != 0 && (uvpt[ PGNUM(addr) ] & PTE_P) != 0
			&& (uvpt[ PGNUM(addr) ] & PTE_U) != 0){
			duppage(envid, PGNUM(addr));
		}
	}

	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U|PTE_W|PTE_P)) < 0)
		panic("fork.c/fork: sys_page_alloc: %e", r);
	
	extern void _pgfault_upcall();
	sys_env_set_pgfault_upcall( envid , _pgfault_upcall );
	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("fork.c/fork: sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
static int sduppage(envid_t, unsigned);

int
sfork(void)
{
	set_pgfault_handler(pgfault);

	envid_t envid;
	int addr;
	int r;

	envid = sys_exofork();
	if (envid < 0)
		panic("fork.c/sfork: sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// We're the parent.
	for (addr = UTEXT; addr < USTACKTOP - PGSIZE ; addr += PGSIZE){
		if (( uvpd[ PDX(addr) ] & PTE_P) != 0 && (uvpt[ PGNUM(addr) ] & PTE_P) != 0
			&& (uvpt[ PGNUM(addr) ] & PTE_U) != 0){
			sduppage(envid, PGNUM(addr));
		}
	}

	// Map the normal user stack as COW
	duppage(envid, PGNUM(USTACKTOP - PGSIZE));

	// Allocate the exception stack
	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U|PTE_W|PTE_P)) < 0)
		panic("fork.c/sfork: sys_page_alloc: %e", r);
	
	extern void _pgfault_upcall();
	sys_env_set_pgfault_upcall( envid , _pgfault_upcall );
	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("fork.c/sfork: sys_env_set_status: %e", r);

	return envid;
	// panic("sfork not implemented");
	// return -E_INVAL;
}

static int
sduppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	void *addr = (void *)(pn * PGSIZE);
	if ((uvpt[PGNUM(addr)] & PTE_W) != 0){
		if((r = sys_page_map(0, addr, envid, addr, PTE_P|PTE_U|PTE_W)) < 0)
			panic("fork.c/sduppage: sys_page_map: %e\n", r);
	}
	else if((uvpt[PGNUM(addr)] & PTE_COW) != 0){
		if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
			panic("fork.c/sduppage: sys_page_alloc: %e", r);
		memcpy(PFTEMP, addr, PGSIZE);
		if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W)) < 0)
			panic("fork.c/sduppage: sys_page_map: %e", r);
		if ((r = sys_page_map(0, PFTEMP, envid, addr, PTE_P|PTE_U|PTE_W)) < 0)
			panic("fork.c/sduppage: sys_page_map: %e", r);
		if ((r = sys_page_unmap(0, PFTEMP)) < 0)
			panic("fork.c/sduppage: sys_page_unmap: %e", r);
	}else{
		if((r = sys_page_map(0, addr, envid, addr, PTE_P|PTE_U)) < 0)
			panic("fork.c/duppage: sys_page_map: %e\n", r);
	}
	return 0;
}