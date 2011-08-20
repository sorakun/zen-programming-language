/*
 * The virtual machine.
 * See Copyright Notice in azure.h.
*/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>
#include "zen.h"
#include "aparse.h"
#include "aerror.h"
#include "agc.h"
#include "atable.h"
#include "acoroutine.h"
#include "aexception.h"

void setvmerror(avm* vm, int errorno)
{
	vm->lasterror = errorno;
	if (!vm->errorreported)
		error((const char*)errorno);
	vm->errorreported = 1;
}

void initheap(avm* vm, heap* hp)
{
	hp->size = hp->availsize = ZEN_DEFAULT_HEAP_SIZE;
	if ((hp->heap=(char*)calloc(hp->size,sizeof(char))) == NULL)
	{
		setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
		return;
	}
	hp->avtable.size = hp->altable.size = 1024;
	hp->avtable.mbt = calloc(hp->avtable.size, sizeof(mblock));
	hp->altable.mbt = calloc(hp->altable.size, sizeof(mblock));
	hp->avtable.mbt[0].boff = 1;	/* the first cell is reserved */
	hp->avtable.mbt[0].eoff = hp->size-1;
	hp->avtable.nextslot = 1;
}

void deinitheap(heap* hp)
{
	free(hp->avtable.mbt);
	free(hp->altable.mbt);
	free(hp->heap);
}

void show(struct _context* ctx, avm* vm, assembly* p)
{
    printf("context: %d\n", p->instructions);
    printf("avm: %d\n", p->nextslot);
    printf("assembly: %d\n", p->entry);
    //printf("stack size: %d\n", p->stacksize);
}

void save_all(struct _context* ctx, assembly* p)
{
    FILE* ctxt = fopen("context.dat", "wb");
    FILE* asmb = fopen("assembly.dat", "wb");


    fwrite(&ctx, sizeof(ctx), 1, ctxt);
    fwrite(&p, sizeof(p), 1, asmb);


    fclose(ctxt);
    fclose(asmb);
}

void load_all(struct _context* ctx, avm* vm, assembly* p, const char* source)
{

    FILE* ctxt = fopen("context.dat", "rb");
    FILE* asmb = fopen("assembly.dat", "rb");


    fread(&ctx, sizeof(ctx), 1, ctxt);
    fread(&p, sizeof(p), 1, asmb);

    fclose(ctxt);
    fclose(asmb);
}

/* Load a program into the vm and initialize its state. */
void load(struct _context* ctx, avm* vm, assembly* p )
{
	int i;
	symbol* s;
	if (!p) return;
	vm->ctx = ctx;
	vm->instructions = p->instructions;
	vm->numins = p->nextslot;
	vm->pc = p->entry;
	for (i=0; i<NUM_REGS; i++)
		seti(&vm->regs[i], 0);
	if ((vm->hs=(stack*)calloc(1, sizeof(stack))) == 0)
	{
		setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
		return;
	}
	vm->hs->size = p->stacksize;
	vm->hs->sbody = (word*)calloc(vm->hs->size, sizeof(word));
	vm->hs->fp = vm->hs->sp = 0;
	vm->cs = vm->hs;
	vm->lasterror = ZEN_NO_ERROR;
	initheap(vm, &vm->hp);
	s = lookupall(0, ctx, GLOBAL_NAME);
	vm->globalsize = s->localsize+s->tempsize;
	vm->regs[rs].entity.ival += vm->globalsize;
	vm->tot.size = ZEN_INITIALTOTSIZE;
	if (!(vm->tot.table = calloc(vm->tot.size, sizeof(pair))))
	{
		setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
		return;
	}
	vm->loaded = 1;
}

/* Is garbage collection safe when instruction ins is to be executed? */
int gcsafe(instruction* ins)
{
	return ins->op == opRet;
}

void run(context* ctx, avm* vm)
{
	int res = ZEN_NO_ERROR;
	if (vm->numins==0)
		return;
	while (res==ZEN_NO_ERROR && vm->lasterror==ZEN_NO_ERROR)
	{
		res = step(ctx, vm);
		if (vm->gcapplied && gcsafe(&vm->instructions[vm->pc]))
			collect(vm);
	}
}

/* Create a string in the heap. */
long newstring(avm* vm, char* s)
{
	objheader* obj;
	heap* hp = &vm->hp;
	long size = strlen(s)+1;
	long offset = allocblock(vm, size);
	obj = (objheader*)getobj(hp->heap, offset);
	obj->type = TSTRING;
	obj->size = size;
	obj->offset = (long)sizeof(mblock)+offset;
	strcpy(getdata(hp->heap, offset), (const char*)s);
	return offset;
}

/*
	Get the local variable size and tempororary variable size of the function
	which a instruction sepcified by ins is in.
*/
symbol* getinsfunc(context* ctx, uint ins)
{
	int i;
	long firstquad, lastquad;
	for (i=0; i<ctx->sbt.nextslot; i++)
	{
		firstquad = ctx->sbt.table[i].firstquad;
		lastquad = ctx->sbt.table[i].lastquad;
		if (ctx->sbt.table[i].isfunction && !ishostfunction(ctx->sbt.table[i].name)
			&&ins>=ctx->prog->quadruples[firstquad].firstinstruction
			&&ins<=ctx->prog->quadruples[lastquad].lastinstruction)
			return &ctx->sbt.table[i];
	}
	return NULL;
}

/* Set a global's value by the name. */
void setglobal(avm* vm, const char* idname, word* w)
{
	symbol* s = lookup(vm->ctx, &vm->ctx->sbt, idname);
	vm->cs->sbody[s->offset] = *w;
}

/* Set an integer global's value by the name. */
void setglobali(avm* vm, const char* idname, int ival)
{
	word w;
	seti(&w, ival);
	setglobal(vm, idname, &w);
}

/* Set a float global's value by the name. */
void setglobalf(avm* vm, const char* idname, float fval)
{
	word w;
	setf(&w, fval);
	setglobal(vm, idname, &w);
}

/* Set a string global's value by the name. */
void setglobals(avm* vm, const char* idname, const char* sval)
{
	word w;
	float soff = newstring(vm, (char*)sval);
	sets(&w, soff);
	setglobal(vm, idname, &w);
}

/* Push a word onto the stack. */
void push(avm* vm, stack* s, word* w)
{
	if (vm->regs[rs].entity.ival < s->size)
		s->sbody[vm->regs[rs].entity.ival++] = *w;
	else
		setvmerror(vm, ZEN_VM_STACK_OVERFLOW);
}

/* Pop a word off the stack. */
word* pop(avm* vm, stack* s)
{
	if (vm->regs[rs].entity.ival > 0)	/* stack not empty */
		return &s->sbody[vm->regs[rs].entity.ival--];
	setvmerror(vm, ZEN_VM_STACK_UNDERFLOW);
	return NULL;
}

/*
	Get the value of an array's element. breg: base register; ireg: index
	register; dreg: destination register.
*/
void ir(avm* vm, int breg, int ireg, int dreg)
{
	ctable* table;
	long toff;
	if (!istable(&vm->regs[breg]))
	{
		setvmerror(vm, ZEN_VM_INVALID_TABLE);
		return;
	}
	toff = vm->tot.table[vm->regs[breg].entity.ival].offset;
	table = (ctable*)getdata(vm->hp.heap, toff);
	IR(vm, table, &vm->regs[ireg], &vm->regs[dreg]);
}

void ia(avm* vm, int breg, int ireg, int sreg)
{
	ctable* table;
	long toff;
	if (!istable(&vm->regs[breg]))
	{
		setvmerror(vm, ZEN_VM_INVALID_TABLE);
		return;
	}
	toff = vm->tot.table[vm->regs[breg].entity.ival].offset;
	table = (ctable*)getdata(vm->hp.heap, toff);
	IA(vm, table, vm->regs[breg].entity.ival, &vm->regs[ireg], &vm->regs[sreg]);
	if (vm->oldheap)
	{
		free(vm->oldheap);
		vm->oldheap = 0;
	}
}

/* Do stack adjusting job when returning from a host. */
void rethost(avm* vm)
{
	vm->regs[rs] = vm->regs[rf];	/* sp = fp */
	/* load control link into fp */
	vm->regs[rf] = vm->cs->sbody[vm->regs[rf].entity.ival-1];
}

int step(context* ctx, avm* vm)
{
	int m, iop1, iop2, iopdest, i, res = ZEN_NO_ERROR;
	int framesize;	/* used for high order functions */
	char *str, *str1, *str2, ibuf[32];
	symbol* s;
	thread* th;
	ctable* table;
	instruction* ins = &vm->instructions[vm->pc];
	iop1 = ins->op1.ival;	iop2 = ins->op2.ival;	iopdest = ins->opdest.ival;
#ifdef ZEN_ENABLE_TRAP
	if (vm->traphook)
		(vm->traphook)(vm);
#endif
	if (ins->op == opHalt)
		return ZEN_VM_HALT;
	switch (ins->op)
	{
	case opLogand:
		seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival==0||vm->regs[iop2].entity.ival==0)?0:1);
		break;
	case opLogor:
		seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival==1||vm->regs[iop2].entity.ival==1)?1:0);
		break;
	case opBitcom:
		if (gettypew(&vm->regs[iopdest])==TINTEGER)
			seti(&vm->regs[iopdest], ~vm->regs[iopdest].entity.ival)
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opBitand:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival & vm->regs[iop2].entity.ival))
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opBitor:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival | vm->regs[iop2].entity.ival))
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opBitxor:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival ^ vm->regs[iop2].entity.ival)
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opLshift:
		seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival<<vm->regs[iop2].entity.ival));
		break;
	case opRshift:
		seti(&vm->regs[iopdest], (vm->regs[iop1].entity.ival>>vm->regs[iop2].entity.ival));
		break;
	case opNeg:
		if (gettypew(&vm->regs[iopdest])==TINTEGER)
			seti(&vm->regs[iopdest], -vm->regs[iopdest].entity.ival)
		else
			setf(&vm->regs[iopdest], -vm->regs[iopdest].entity.fval)
		break;
	case opNot:
		vm->regs[iopdest].entity.ival = !vm->regs[iopdest].entity.ival;
		break;
	case opFact:
		vm->regs[iopdest].entity.ival = factorial(vm->regs[iopdest].entity.ival);
		break;
	case opAdd:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival + vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival + vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival))+64, sizeof(char));
			sprintf(str, "%d%s", vm->regs[iop1].entity.ival, getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival));
			sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
			free(str);
		}
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval + (float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval + vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival))+64, sizeof(char));
			sprintf(str, "%f%s", vm->regs[iop1].entity.fval, getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival));
			sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
			free(str);
		}
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TINTEGER)
		{
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival))+64, sizeof(char));
			sprintf(str, "%s%d", getdata(vm->hp.heap, vm->regs[iop1].entity.ival), vm->regs[ins->op2.ival].entity.ival);
			sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
			free(str);
		}
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TFLOAT)
		{
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival))+64, sizeof(char));
			sprintf(str, "%s%f", getdata(vm->hp.heap, vm->regs[iop1].entity.ival), vm->regs[ins->op2.ival].entity.fval);
			sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
			free(str);
		}
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TSTRING)
		{
			char* s1 = getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival);
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival))
				+strlen(getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival))+1, sizeof(char));
			strcpy(str, (char*)getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival));
			strcat(str, (char*)getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival));
			sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
			free(str);
		}
		//- added for testing
		else if (gettypew(&vm->regs[iop1])== TTABLE && gettypew(&vm->regs[iop2])==TINTEGER)
		{/*
	        long tindex = TOTinsert(vm, off);
	        ctable* tbl = (ctable*)getdata(vm->hp.heap, ind2off(vm->regs[iop1].entity.ival));
	        word index, source;

	        seti(&index, tbl->nextslot);
            seti(&source, vm->regs[iop2].entity.ival);
	        IA(vm, tbl, tindex, &index, &source);

	        sett(&vm->regs[iopdest], tindex);*/
		}
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;

	case opMinus:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival - vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival - vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval - (float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval - vm->regs[iop2].entity.fval)
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opMultiply:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival * vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival * vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval * (float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval * vm->regs[iop2].entity.fval)
        //- String repeat
        else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TINTEGER)
        {
			char* s1 = string_repeat(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival), vm->regs[iop2].entity.ival);
			sets(&vm->regs[iopdest], newstring(vm, s1))
        }
        else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TSTRING)
        {
			char* s1 = string_repeat(getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival), vm->regs[iop1].entity.ival);
			sets(&vm->regs[iopdest], newstring(vm, s1))
        }
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
    //- added for testing
    case opPower:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], pow(vm->regs[iop1].entity.ival, vm->regs[iop2].entity.ival))
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], pow((float)vm->regs[iop1].entity.ival, vm->regs[iop2].entity.fval))
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			setf(&vm->regs[iopdest], pow(vm->regs[iop1].entity.fval, (float)vm->regs[iop2].entity.ival))
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], pow(vm->regs[iop1].entity.fval, vm->regs[iop2].entity.fval))
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opDivide:
		if (vm->regs[iop2].entity.ival == 0)
		{
			throwdbz(vm);
			return ZEN_NO_ERROR;
		}
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival / vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival / vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval / (float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			setf(&vm->regs[iopdest], vm->regs[iop1].entity.fval / vm->regs[iop2].entity.fval)
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opMod:
		if (vm->regs[iop2].entity.ival == 0)
			return ZEN_VM_DIV_ZERO;
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival % vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival % (int)vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], (int)vm->regs[iop1].entity.fval % vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], (int)vm->regs[iop1].entity.fval % (int)vm->regs[iop2].entity.fval)
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		break;
	case opEqual:	case opNeq:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival==vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival==vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval==(float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval==vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str1 = (char*)getdata(vm->hp.heap, vm->regs[iop1].entity.ival);
			str2 = (char*)getdata(vm->hp.heap, vm->regs[iop2].entity.ival);
			seti(&vm->regs[iopdest], !strcmp(str1,str2));
		}
		else if (gettypew(&vm->regs[iop1])==TNIL && gettypew(&vm->regs[iop2])==TNIL)
			seti(&vm->regs[iopdest], 1)
		else seti(&vm->regs[iopdest], 0);
		if (ins->op == opNeq)	/* opposite operation */
			seti(&vm->regs[iopdest], !vm->regs[iopdest].entity.ival);
		break;
    case opTEqual:    case opTNeq:
        if(gettypew(&vm->regs[iop1]) == gettypew(&vm->regs[iop2]))
            seti(&vm->regs[iopdest], 1)
        else
            seti(&vm->regs[iopdest], 0)
        if (ins->op == opTNeq)
           seti(&vm->regs[iopdest], !vm->regs[iopdest].entity.ival);
		break;
	case opGreater:	case opLeq:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival>vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival>vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval>(float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval>vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str1 = (char*)getdata(vm->hp.heap, vm->regs[iop1].entity.ival);
			str2 = (char*)getdata(vm->hp.heap, vm->regs[iop2].entity.ival);
			seti(&vm->regs[iopdest], strcmp(str1,str2)>0);
		}
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		if (ins->op == opLeq)
			seti(&vm->regs[iopdest], !vm->regs[iopdest].entity.ival);
		break;
	case opGeq:	case opLess:
		if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.ival>=vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TINTEGER && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], (float)vm->regs[iop1].entity.ival>=vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TINTEGER)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval>=(float)vm->regs[iop2].entity.ival)
		else if (gettypew(&vm->regs[iop1])==TFLOAT && gettypew(&vm->regs[iop2])==TFLOAT)
			seti(&vm->regs[iopdest], vm->regs[iop1].entity.fval>=vm->regs[iop2].entity.fval)
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str1 = (char*)getdata(vm->hp.heap, vm->regs[iop1].entity.ival);
			str2 = (char*)getdata(vm->hp.heap, vm->regs[iop2].entity.ival);
			seti(&vm->regs[iopdest], strcmp(str1,str2)>=0);
		}
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		if (ins->op == opLess)
			seti(&vm->regs[iopdest], !vm->regs[iopdest].entity.ival);
		break;
	case opConcat:
		if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TSTRING)
		{
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival))
				+strlen(getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival))+1, sizeof(char));
			strcpy(str, getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival));
			strcat(str, getdata(vm->hp.heap, vm->regs[ins->op2.ival].entity.ival));
		}
		else if (gettypew(&vm->regs[iop1])==TSTRING && gettypew(&vm->regs[iop2])==TINTEGER)
		{
			sprintf(ibuf, "%d", vm->regs[ins->op2.ival].entity.ival);
			str = (char*)calloc(strlen(getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival))
				+strlen(ibuf)+1, sizeof(char));
			strcpy(str, getdata(vm->hp.heap, vm->regs[ins->op1.ival].entity.ival));
			strcat(str, ibuf);
		}
		else setvmerror(vm, ZEN_VM_INVALID_OPERATION);
		sets(&vm->regs[ins->opdest.ival], newstring(vm,str));
		free(str);
		break;
	case opPush: push(vm, vm->cs, &vm->regs[iopdest]); break;
	case opPop:
		vm->cs->sbody[vm->regs[rf].entity.ival+ins->opdest.ival-ins->op2.ival] = vm->cs->sbody[vm->regs[rs].entity.ival+1];
		vm->regs[rs].entity.ival -= ins->op1.ival;
		break;
	case opCall: case opCallhost:	case opCallin:
		if (ins->op==opCallin)	/* note that the lookup is very slow! */
		{
			if (gettypew(&vm->regs[ins->opdest.ival])==THFUNCTION)
				framesize = ins->op2.ival;
			else
			{
				s = getinsfunc(ctx, vm->regs[ins->opdest.ival].entity.ival);
				if (!s)
					setvmerror(vm, ZEN_VM_CONTEXT_CORRUPTED);
				else
					framesize = s->localsize+s->tempsize;
			}
		}
		else framesize = ins->op2.ival;
		pushr(vm, vm->cs, rf);			/* push fp */
		vm->regs[rf] = vm->regs[rs];	/* fp = sp */
		seti(&vm->regs[ra], vm->pc+1);
		pushr(vm, vm->cs, ra);			/* save return address */
		if (vm->regs[rs].entity.ival+framesize>vm->cs->size-1)
			res = ZEN_VM_STACK_OVERFLOW;
#ifndef ZEN_ENABLE_OPTIMIZATION
		/* zero out the new stack frame */
		memset(&vm->cs->sbody[vm->regs[rs].entity.ival], 0, sizeof(word)*framesize);
#endif
		/* allocate a new stack frame */
		vm->regs[rs].entity.ival += framesize;
		switch (ins->op)
		{
		case opCall: vm->pc = ins->opdest.ival; break;
		case opCallhost:
			(*(userfunc)ins->opdest.ival)(vm);
			/* if "resume" is called, do not adjust the stack */
			if (zen_getfunc("resume") != *(userfunc)ins->opdest.ival)
			{
				rethost(vm);
				vm->pc++;
			}
			break;
		case opCallin:
			if (gettypew(&vm->regs[ins->opdest.ival])!=THFUNCTION)
				vm->pc = vm->regs[ins->opdest.ival].entity.ival;
			else
			{
				(*(userfunc)vm->regs[ins->opdest.ival].entity.ival)(vm);
				rethost(vm);
				vm->pc++;
			}
			break;
		}
		return res;
	case opTRCall:	/* tail recursion call, no new frame created */
		for (i=0; i<ins->op1.ival; i++)	/* copy all the arguments */
			vm->cs->sbody[vm->regs[rf].entity.ival-2-i]
				= vm->cs->sbody[(vm->regs[rs].entity.ival--)-1];
		vm->pc = ins->opdest.ival;		/* then jump to the destination */
		return res;
	case opRet:
		/* check if this is the return of thread's function */
		if (withinmainthread(vm) || vm->regs[rf].entity.ival != vm->ofp.entity.ival)
		{
			vm->regs[rs] = vm->regs[rf];	/* sp = fp */
			/* load control link into fp */
			vm->regs[rf] = vm->cs->sbody[vm->regs[rf].entity.ival-1];
			/* restore the pc */
			vm->pc = vm->cs->sbody[vm->regs[rs].entity.ival].entity.ival;
			/* is it at the bottom? */
			if (vm->regs[rf].entity.ival==0 && vm->execfunc)
				return ZEN_VM_HALT;
			return res;
		}
		else	/* returning from a thread */
		{
			vm->ct->dead = 1;
			th = restorelastthread(vm);
			if (th->s != vm->hs)
				free(th->s);
		}
		break;
	case opYield:
		yield(vm);
		vm->regs[rs] = vm->regs[rf];	/* sp = fp */
		/* load control link into fp */
		vm->regs[rf] = vm->cs->sbody[vm->regs[rf].entity.ival-1];
		break;
	case opThrow:
		throwit(vm, &vm->regs[ins->opdest.ival]);
		return ZEN_NO_ERROR;
	case opStart:	/* initialize slot and node */
		vm->slot = vm->node = 0;
		break;
	case opUpdate:	/* proceed to the next element */
		table = (ctable*)getdata(vm->hp.heap,
			vm->tot.table[vm->regs[iop1].entity.ival].offset);
		seti(&vm->regs[iopdest], update(vm, table));
		break;
	case opCopy:	/* copy the key/value pair */
		vm->regs[ra] = vm->wkey;
		vm->regs[rb] = vm->wval; break;
	case opSave:	/* save the slot node values */
		seti(&vm->regs[ra], vm->slot);
		seti(&vm->regs[rb], vm->node);
		break;
	case opRestore:	/* restore the slot node values */
		vm->slot = vm->regs[ra].entity.ival;
		vm->node = vm->regs[rb].entity.ival;
		break;
	case opLCI: seti(&vm->regs[iopdest], iop1); break;
	case opLCF: setf(&vm->regs[iopdest], ins->op1.fval); break;
	case opLNIL: memset(&vm->regs[iopdest], 0, sizeof(word)); break;
	case opLFUNC: seti(&vm->regs[iopdest], iop1); break;
	case opLHF:
		vm->regs[iopdest].entity.ival = iop1;
		settypew(&vm->regs[iopdest],THFUNCTION);
		break;
	case opLD:
		m = vm->regs[iop1].entity.ival + iop2;
		if (!withinmainthread(vm) && iop1==rd)	/* coroutine accessing global */
			vm->regs[iopdest] = vm->ts.threads[0]->s->sbody[m];
		else
			vm->regs[iopdest] = vm->cs->sbody[m];
		break;
	case opST:
		m = vm->regs[iop1].entity.ival + iop2;
		if (!withinmainthread(vm) && iop1==rd)	/* coroutine accessing global */
			vm->ts.threads[0]->s->sbody[m] = vm->regs[iopdest];
		else
			vm->cs->sbody[m] = vm->regs[iopdest];
		break;
	case opNewstring:
		sets(&vm->regs[ins->opdest.ival], newstring(vm,(char*)ins->op1.ival));
		break;
	case opNewtable: sett(&vm->regs[ins->opdest.ival],
			TOTinsert(vm, newtable(vm,ZEN_INITIALTABLESIZE))); break;
	case opIR: ir(vm, ins->op1.ival, ins->op2.ival, ins->opdest.ival); break;
	case opIA: ia(vm, ins->op1.ival, ins->op2.ival,ins->opdest.ival); break;
	case opJz:
		if (vm->regs[iop1].entity.ival!=0)
			break;
		vm->pc = iopdest;
		return ZEN_NO_ERROR;
	case opJnz:
		if (vm->regs[iop1].entity.ival==0)
			break;
		vm->pc = iopdest;
		return ZEN_NO_ERROR;
	case opJmp: vm->pc = iopdest; return ZEN_NO_ERROR;
	default: error("Unknown instruction. \n"); break;
	}
	vm->pc++;
	return res;
}

/* Call a function with the name func. */
void callf(avm* vm, const char* func)
{
	symbol* s;
	context* ctx = vm->ctx;
	pushr(vm, vm->cs, rf);			/* push the ofp */
	vm->regs[rf] = vm->regs[rs];	/* fp = sp */
	seti(&vm->regs[ra], vm->pc+1);	/* push the return address */
	pushr(vm, vm->cs, ra);
	if ((s=lookup(ctx, &ctx->sbt, func)) == NULL)
		return;
	if (strcmp(func, GLOBAL_NAME) != 0)
		vm->regs[rs].entity.ival += s->tempsize+s->localsize;
	vm->pc = ctx->prog->quadruples[s->firstquad].firstinstruction;
	vm->execfunc = 1;
	run(ctx, vm);
	vm->execfunc = 0;
}

/* Return the nth return value. */
word* getret(avm* vm, int n)
{
	return &vm->cs->sbody[vm->regs[rs].entity.ival+1+n];
}

/* Get an integer argument by the index from vm. */
int getint(avm* vm, int ind)
{
	word* w = getarg(vm, ind);
	if (gettypew(w) != TINTEGER)
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
	return w->entity.ival;
}

/* Get an float argument by the index from vm. */
float getfloat(avm* vm, int ind)
{
	word* w = getarg(vm, ind);
	if (gettypew(w) != TFLOAT)
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
	return w->entity.fval;
}

/* Get an string argument by the index from vm. */
char* getstring(avm* vm, int ind)
{
	word* w = getarg(vm, ind);
	if (gettypew(w) != TSTRING)
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
	return (char*)getdata(vm->hp.heap, w->entity.ival);
}

/* Get an file handle argument by the index from vm. */
FILE* gethandle(avm* vm, int ind)
{
	word* w = getarg(vm, ind);
	if (gettypew(w) != THANDLE)
	{
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
		return (FILE*)0;
	}
	return (FILE*)w->entity.ival;
}

/* Get a directory handle argument by the index from vm. */
DIR* getdirectory(avm* vm, int ind)
{
	word* w = getarg(vm, ind);
	if (gettypew(w) != TDIR)
	{
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
		return (DIR*)0;
	}
	return (DIR*)w->entity.ival;
}

/* Get a global's value by the name. */
word* getglobal(avm* vm, const char* idname)
{
	symbol* s = lookup(vm->ctx, &vm->ctx->sbt, idname);
	return &vm->cs->sbody[s->offset];
}

/* Get an integer global's value by the name. */
int getglobali(avm* vm, const char* idname)
{
	word* w = getglobal(vm, idname);
	if (gettypew(w) != TINTEGER)
		setvmerror(vm, ZEN_VM_INCORRECT_TYPE);
	return w->entity.ival;
}

/* Get a float global's value by the name. */
float getglobalf(avm* vm, const char* idname)
{
	word* w = getglobal(vm, idname);
	if (gettypew(w) != TFLOAT)
		setvmerror(vm, ZEN_VM_INCORRECT_TYPE);
	return w->entity.fval;
}

/* Get a string global's value by the name. */
char* getglobals(avm* vm, const char* idname)
{
	word* w = getglobal(vm, idname);
	char* str = (char*)getdata(vm->hp.heap, w->entity.ival);
	if (gettypew(w) != TSTRING)
		setvmerror(vm, ZEN_VM_INCORRECT_TYPE);
	return str;
}

/*
	Get a level-nested local variable's word by its name.
	Steps:
	1. dive into the correct level of stack frames; if level==0, ins=pc;
	   otherwise, go to level-1 in the stack, and ins=return address;
	2. find out the function record in the sbt that contains ins;
	3. find out the local variable's offset using its name and the record;
	4. use the offset to locate the word in the corresponding stack frame.
*/
word* getlocbylev(avm* vm, const char* idname, int level)
{
	return (word*)0;
}

/* Push an integer onto the stack. */
void pushi(avm* vm, int ival)
{
	word w;
	seti(&w, ival);
	push(vm, vm->cs, &w);
}

void pushf(avm* vm, float fval)
{
	word w;
	setf(&w, fval);
	push(vm, vm->cs, &w);
}

void pushs(avm* vm, const char* str)
{
	word w;
	float soff = newstring(vm, (char*)str);
	sets(&w, soff);
	push(vm, vm->cs, &w);
}

/* Print a single word. */
void printw(avm* vm, word* w)
{
	switch (gettypew(w))
	{
	case TINTEGER: printf("%d", w->entity.ival); break;
	case TFLOAT: printf("%f", w->entity.fval); break;
	case TFUNCTION: printf("Function: %d", w->entity.ival); break;
	case THFUNCTION: printf("Host function: %d", w->entity.ival); break;
	case TSTRING:printf("%s", getdata(vm->hp.heap,w->entity.ival)); break;
	case TTABLE:
		printf("Table: %X", vm->tot.table[w->entity.ival].offset);
		break;
	case TTHREAD: printf("Thread: %X", w->entity.ival); break;
	case THANDLE: printf("File handle: %X", w->entity.ival); break;
	case TDIR: printf("Directory handle: %X", w->entity.ival); break;
	case TBLOB: printf("%s", (char*)w->entity.ival); break;
	default: printf("void"); break;
	}
}

/* If w's type does not match, mark an error vm. */
void verifytype(avm* vm, word* w, int type)
{
	if (gettypew(w) != type)
		setvmerror(vm, ZEN_VM_INCORRECT_TYPE);
}

/* Install a callback function for instruction interrupt (II). */
void settraphook(avm* vm, userfunc hook)
{
	vm->traphook = hook;
}

void releasevm(avm* vm)
{
	if (!vm->loaded) return;
	freeextobjs(vm);
	free(vm->hs->sbody);
	free(vm->hs);
	free(vm->tot.table);
	if (vm->oldheap)
		free(vm->oldheap);
	deinitheap(&vm->hp);
	free(vm->instructions);
}
