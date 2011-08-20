/*
 * The corelib functions for language-level construct handling.
 * See Copyright Notice in azure.h.
*/

#include "zen.h"
#include "aparse.h"
#include "atable.h"
#include "acoroutine.h"

void tablelength(avm* vm)
{
	word *w = getarg(vm, 0), w1;
	ctable* table = (ctable*)getdata(vm->hp.heap, ind2off(w->entity.ival));
	seti(&w1, table->nextslot);
	returnv(vm, &w1);
}

void type(avm* vm)
{
	word *w = getarg(vm, 0), w1;
	switch (gettypew(w))
	{
	    case TNIL: sets(&w1, newstring(vm, "void")); break;
	    case TINTEGER: sets(&w1, newstring(vm, "integer")); break;
	    case TFLOAT: sets(&w1, newstring(vm, "float")); break;
	    case TSTRING: sets(&w1, newstring(vm, "string")); break;
	    case TTABLE: sets(&w1, newstring(vm, "table")); break;
	    case TBLOB: sets(&w1, newstring(vm, "blob")); break;
        case TFUNCTION: sets(&w1, newstring(vm, "func")); break;
	    case THFUNCTION: sets(&w1, newstring(vm, "hfunc")); break;
	    case TNODE: sets(&w1, newstring(vm, "node")); break;
        case TSTACK: sets(&w1, newstring(vm, "stack")); break;
        case TTHREAD: sets(&w1, newstring(vm, "thread")); break;
        case TDIR: sets(&w1, newstring(vm, "directory")); break;
        default: break;
	}
	returnv(vm, &w1);
}

/*
	Get a specific argument from the stack. Used for variable-length argument
	functions.
*/
void vargs(avm* vm)
{
	word* w = getarg(vm, 0);
	word w1;
	int oldfp = vm->cs->sbody[vm->regs[rf].entity.ival-1].entity.ival;
	w1 = vm->cs->sbody[oldfp-2-w->entity.ival];
	returnv(vm, &w1);
}

/* Create a coroutine object. */
void create(avm* vm)
{
	int i;
	word *w = getarg(vm, 0), *wn;
	word wv = {0};
	symbol* s = getinsfunc(vm->ctx, w->entity.ival);
	long croff = newcoroutine(vm, w->entity.ival, ZEN_DEFAULT_STACK_SIZE);
	thread* th = (thread*)getdata(vm->hp.heap, croff);
	wn = getarg(vm, 1);				/* wn->entity.ival: number of arguments */
	for (i=0; i<wn->entity.ival; i++)					/* push all args */
		pushthread(vm, th, getarg(vm, wn->entity.ival-i+1));
	pushthread(vm, th, &wv);							/* push a dumb word */
	th->regs[rf] = th->regs[rs];
	th->regs[rs].entity.ival+=s->tempsize+s->localsize;	/* expand the stack */
	th->ofp = th->regs[rf];								/* save original fp */
	settypew(&wv, TTHREAD);
	wv.entity.ival = croff;
	returnv(vm, &wv);
}

void resume(avm* vm)
{
	word* w = getarg(vm, 0);
	thread* th = (thread*)getdata(vm->hp.heap, w->entity.ival);
	if (th->dead)
		return;	/* do not resume a dead thread! */
	switchtocoroutine(vm, th);
	vm->thoff = w->entity.ival;
}

/* Use this function to suggest the vm to collect garbage. */
void ccollectgarbage(avm* vm)
{
	vm->gcapplied = 1;
}

static fptrname libstd[] =
{	{tablelength, "tablelength"},
	{type, "type"},
	{vargs, "vargs"},
	{create, "create"},
	{resume, "resume"},
	{ccollectgarbage, "collectgarbage"},
};

void zen_openlibstd()
{
	int i;
	for (i=0; i<sizeof(libstd)/sizeof(fptrname); i++)
		zen_regfunc(libstd[i].ptr, libstd[i].name);
}
