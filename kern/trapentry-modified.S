/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



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
#define TRAPHANDLER(name, num, dpl, istrap)				\
	.text;								\
	.globl __t_##name##_handler;					\
	.type __t_##name##_handler, @function;			\
	.align 2;								\
	__t_##name##_handler:					\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long __t_##name##_handler;					\
	.long ((dpl)<<8) | (istrap);

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num, dpl, istrap)			\
	.text;								\
	.globl __t_##name##_handler;					\
	.type __t_##name##_handler, @function;			\
	.align 2;								\
	__t_##name##_handler:					\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps;							\
	.data;								\
	.long __t_##name##_handler;					\
	.long ((dpl)<<8) | (istrap);

#define IDT_PADDING(prev, next)					\
	.data;								\
	.fill (next-prev-1),8;

.data
.align 2
.global idt

idt:

TRAPHANDLER_NOEC(divide, T_DIVIDE, 0, 1)
TRAPHANDLER_NOEC(debug, T_DEBUG, 0, 1)
TRAPHANDLER_NOEC(nmi, T_NMI, 0, 1)
TRAPHANDLER_NOEC(brkpt, T_BRKPT, 3, 1)
TRAPHANDLER_NOEC(oflow, T_OFLOW, 0, 1)
TRAPHANDLER_NOEC(bound, T_BOUND, 0, 1)
TRAPHANDLER_NOEC(illop, T_ILLOP, 0, 1)
TRAPHANDLER_NOEC(device, T_DEVICE, 0, 1)
TRAPHANDLER(dblflt, T_DBLFLT, 0, 1)
IDT_PADDING(T_DBLFLT, T_TSS)
TRAPHANDLER(tss, T_TSS, 0, 1)
TRAPHANDLER(segnp, T_SEGNP, 0, 1)
TRAPHANDLER(stack, T_STACK, 0, 1)
TRAPHANDLER(gpflt, T_GPFLT, 0, 1)
TRAPHANDLER(pgflt, T_PGFLT, 0, 1)
IDT_PADDING(T_PGFLT, T_FPERR)
TRAPHANDLER_NOEC(fperr, T_FPERR, 0, 1)
TRAPHANDLER(align, T_ALIGN, 0, 1)
TRAPHANDLER_NOEC(mchk, T_MCHK, 0, 1)
TRAPHANDLER_NOEC(simderr, T_SIMDERR, 0, 1)
IDT_PADDING(T_SIMDERR, T_SYSCALL)
TRAPHANDLER_NOEC(syscall, T_SYSCALL, 3, 1)
IDT_PADDING(T_SYSCALL, 256)

.data
.align 2
.globl idt_pd

idt_pd:
	.word 0x3fff
	.long idt

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