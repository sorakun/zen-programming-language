/*
 * Coroutines.
 * See Copyright Notice in azure.h.
*/

#include <memory.h>
#include "zen.h"
#include "agc.h"
#include "aerror.h"

/* size is the # of words in the stack. */
long newstack(avm* vm, uint size)
{
	long soff = allocblock(vm, size*sizeof(word));
	objheader* obj = (objheader*)getobj(vm->hp.heap, soff);
	obj->type = TSTACK;
	obj->size = size*sizeof(word);
	obj->offset = (long)sizeof(mblock)+soff;
	obj->refcount = 0;
	return soff;
}

/*
	Create a coroutine object and return its offset in the heap. entry is the
	entry of the function associated with the coroutine.
*/
long newcoroutine(avm* vm, uint entry, uint stacksize)
{
	long croff = allocblock(vm, sizeof(thread));
	long stoff = newstack(vm, stacksize);
	thread* th = (thread*)getdata(vm->hp.heap, croff);
	th->s = (stack*)calloc(1, sizeof(stack));
	th->regs[rd] = vm->regs[rd];
	th->entry = th->pc = entry;
	th->s->soff = stoff;
	th->s->sbody = (word*)getdata(vm->hp.heap, th->s->soff);
	th->s->size = stacksize;
	return croff;
}

/* Save the current thread's context to the stack. */
void savecurrentthread(avm* vm)
{
	thread* th = (thread*)calloc(1, sizeof(thread));
	memcpy(th, vm->regs, sizeof(word)*NUM_REGS);
	th->pc = vm->pc;
	th->s = vm->cs;
	vm->ts.threads[vm->ts.nextslot++] = th;
}

/* Restore last saved thread's context from the stack. */
thread* restorelastthread(avm* vm)
{
	thread* th = vm->ts.threads[--vm->ts.nextslot];
	thread* oth;
	memcpy(vm->regs, th->regs, sizeof(word)*NUM_REGS);
	vm->pc = th->pc;
	vm->cs = th->s;
	vm->ofp = th->ofp;
	oth = vm->ct;
	vm->ct = th;
	free(th);
	return oth;
}

/* Switch thread th's context as current context. */
void switchtocoroutine(avm* vm, thread* th)
{
	savecurrentthread(vm);
	vm->ofp = th->ofp;
	vm->cs = th->s;
	memcpy(vm->regs, th->regs, sizeof(word)*NUM_REGS);
	vm->pc = th->pc;
	vm->ct = th;
}

/* Yield from within current thread. */
void yield(avm* vm)
{
	/* save current thread context*/
	thread* th = (thread*)getdata(vm->hp.heap, vm->thoff);
	memcpy(th->regs, vm->regs, sizeof(word)*NUM_REGS);
	th->pc = vm->pc+1;
	th->s = vm->cs;
	restorelastthread(vm);
	/* Copy the yield value to current thread's stack. */
	vm->cs->sbody[vm->regs[rf].entity.ival+1] = th->s->sbody[th->regs[rs].entity.ival];
}

/* Push a word w onto thread th's stack. */
void pushthread(avm* vm, thread* th, word* w)
{
	if (th->regs[rs].entity.ival < th->s->size)
		*(th->s->sbody+th->regs[rs].entity.ival++) = (*w);
	else
		setvmerror(vm, ZEN_VM_THREAD_STACK_OVERFLOW);
}
