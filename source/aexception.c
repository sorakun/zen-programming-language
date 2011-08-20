/*
 * Exception handling.
 * See Copyright Notice in azure.h.
*/

#include "zen.h"
#include "avm.h"
#include "aparse.h"
#include "aerror.h"

/* Save the exception word to the virtual machine. */
void saveexception(avm* vm, word* w)
{
	vm->regs[re] = *w;
}

/* Get the corresponding catch's entry of an instruction for a function. */
uint getcatch(symbol* s, uint ins)
{
	int i;
	if (s->ta == 0) return 0;
	for (i=0; i<s->ta->nextslot; i++)
		if (ins>=s->ta->tr[i].first && ins<=s->ta->tr[i].last)
			return s->ta->tr[i].catchentry;
	return 0;
}

/*
	Throw an exception with w as the value.
	When an exception is thrown:
	1. Get the function where current instruction is in (ie vm->pc);
	2. Determine if it is within a try block, if so, set pc to the entry
	   of the associated catch clause;
	3. If not, pop up current stack frame (stack unwinding), get the next pc
	   of its caller. If the stack is not empty, go to 1; otherwise mark an
	   error in the virtual machine.
*/
void throwit(avm* vm, word* w)
{
	uint entry = 0;		/* catch entry */
	while (entry==0)
	{
		entry = getcatch(getinsfunc(vm->ctx, vm->pc), vm->pc);
		saveexception(vm, w);
		if (entry)	/* catch clause found, transfer control to its entry */
			vm->pc = entry;
		else if (vm->regs[rf].entity.ival==0)	/* at the stack bottom? */
			break;
		else	/* no catch clause, unwind the stack */
		{
			vm->regs[rs] = vm->regs[rf];	/* sp = fp */
			/* load control link into fp */
			vm->regs[rf] = vm->cs->sbody[vm->regs[rf].entity.ival-1];
			/* restore the pc */
			vm->pc = vm->cs->sbody[vm->regs[rs].entity.ival].entity.ival;
		}
	}
	if (!entry)
	{
	    error("unhandled exception, ");
		printw(vm, w);
		setvmerror(vm, ZEN_VM_UNHANDLED_EXCEPTION);
	}
}

/* Throw the divided-by-zero exception. */
void throwdbz(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "divide-by-zero.\n"));
	throwit(vm, &w);
}

/* Throw the file-not-found exception. */
void throwfnf(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "file not found.\n"));
	throwit(vm, &w);
}

/* Throw the cannot-load-module exception. */
void throwcnlm(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "cannot load module.\n"));
	throwit(vm, &w);
}

/* Throw the cannot-find-function exception. */
void throwcnff(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "cannot locate function entry.\n"));
	throwit(vm, &w);
}

/* Throw the out-of-range exception. */
void throwoor(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "index out of range.\n"));
	throwit(vm, &w);
}

/* Throw the invalid-argument exception. */
void throwia(avm* vm)
{
	word w;
	sets(&w,newstring(vm, "invalid argument.\n"));
	throwit(vm, &w);
}
