/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(intrpt_divide, T_DIVIDE)
TRAPHANDLER_NOEC(intrpt_debug, T_DEBUG)
TRAPHANDLER_NOEC(intrpt_nmi, T_NMI)
TRAPHANDLER_NOEC(intrpt_brkpt, T_BRKPT)
TRAPHANDLER_NOEC(intrpt_oflow, T_OFLOW)
TRAPHANDLER_NOEC(intrpt_bound, T_BOUND)
TRAPHANDLER_NOEC(intrpt_illop, T_ILLOP)
TRAPHANDLER_NOEC(intrpt_device, T_DEVICE)
TRAPHANDLER(intrpt_dblflt, T_DBLFLT)
/* TRAPHANDLER_NOEC(intrpt_coproc, T_COPROC) */
TRAPHANDLER(intrpt_tss, T_TSS)
TRAPHANDLER(intrpt_segnp, T_SEGNP)
TRAPHANDLER(intrpt_stack, T_STACK)
TRAPHANDLER(intrpt_gpflt, T_GPFLT)
TRAPHANDLER(intrpt_pgflt, T_PGFLT)
/* TRAPHANDLER_NOEC(intrpt_res, T_RES) */
TRAPHANDLER_NOEC(intrpt_fperr, T_FPERR)
TRAPHANDLER(intrpt_align, T_ALIGN)
TRAPHANDLER_NOEC(intrpt_mchk, T_MCHK)
TRAPHANDLER_NOEC(intrpt_simderr, T_SIMDERR)

/* External Interrupts */
TRAPHANDLER_NOEC(intrpt_irq0, IRQ_OFFSET)
TRAPHANDLER_NOEC(intrpt_irq1, IRQ_OFFSET + 1)
TRAPHANDLER_NOEC(intrpt_irq2, IRQ_OFFSET + 2)
TRAPHANDLER_NOEC(intrpt_irq3, IRQ_OFFSET + 3)
TRAPHANDLER_NOEC(intrpt_irq4, IRQ_OFFSET + 4)
TRAPHANDLER_NOEC(intrpt_irq5, IRQ_OFFSET + 5)
TRAPHANDLER_NOEC(intrpt_irq6, IRQ_OFFSET + 6)
TRAPHANDLER_NOEC(intrpt_irq7, IRQ_OFFSET + 7)
TRAPHANDLER_NOEC(intrpt_irq8, IRQ_OFFSET + 8)
TRAPHANDLER_NOEC(intrpt_irq9, IRQ_OFFSET + 9)
TRAPHANDLER_NOEC(intrpt_irq10, IRQ_OFFSET + 10)
TRAPHANDLER_NOEC(intrpt_irq11, IRQ_OFFSET + 11)
TRAPHANDLER_NOEC(intrpt_irq12, IRQ_OFFSET + 12)
TRAPHANDLER_NOEC(intrpt_irq13, IRQ_OFFSET + 13)
TRAPHANDLER_NOEC(intrpt_irq14, IRQ_OFFSET + 14)
TRAPHANDLER_NOEC(intrpt_irq15, IRQ_OFFSET + 15)

TRAPHANDLER_NOEC(intrpt_syscall, T_SYSCALL)

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
pushw $0x0
pushw %ds
pushw $0x0
pushw %es
pushal

movl $GD_KD, %eax
movw %ds, %ax
movw %es, %ax
pushl %esp
call trap
