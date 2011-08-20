/*
 * Virtual machine code generator.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include "zen.h"
#include "aerror.h"
#include "aparse.h"

/* Emit an instruction to the assembly program. */
void emit(context* ctx, assembly* casm,
		  int op, operand* op1, operand* op2, operand* opdest)
{
	int nextslot = casm->nextslot;
	operand o = {0};			/* a dummy operand for padding */
#ifdef ZEN_ENABLE_OPTIMIZATION
	/* a simple optimizing trick */
	if (op==opLD && nextslot &&
		casm->instructions[nextslot-1].op==opST &&
		casm->instructions[nextslot-1].op1.ival==op1->ival &&
		casm->instructions[nextslot-1].op2.ival==op2->ival &&
		casm->instructions[nextslot-1].opdest.ival==opdest->ival &&
		!(ctx->inleader&&ctx->insord==0))	/* can't be a leader's first ins */
		return;
#endif
	if (nextslot >= casm->size)	/* buffer is full, reallocate */
	{
		casm->size = casm->size*2+1;
		if ((casm->instructions =
			realloc(casm->instructions, casm->size*sizeof(instruction))) == NULL)
			setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Not enough memory.\n");
	}
	casm->instructions[nextslot].op = op;
	casm->instructions[nextslot].op1 = (op1!=NULL?*op1:o);
	casm->instructions[nextslot].op2 = (op2!=NULL?*op2:o);
	casm->instructions[nextslot].opdest = (opdest!=NULL?*opdest:o);
	casm->nextslot++;
	ctx->insord++;
}

/*
	Get the local variable size and tempororary variable size of the function
	which a quadruple sepcified by quad is in.
*/
symbol* getquadfunc(context* ctx, int quad)
{
	int i;
	for (i=0; i<ctx->sbt.nextslot; i++)
		if (ctx->sbt.table[i].isfunction && !ishostfunction(ctx->sbt.table[i].name)
		&&quad>=ctx->sbt.table[i].firstquad&&quad<=ctx->sbt.table[i].lastquad)
			return &ctx->sbt.table[i];
	return NULL;
}

/* access == 0 load; access == 1 store. */
void accessreg(context* ctx, assembly* casm, int access, qoperand* q,
			   int reg, int local, int temp)
{
	operand o = {0}, o1 = {0}, o2 = {0};
	if (!q) return;
	switch (q->scope)
	{
	case SGLOBAL:
		o.ival = rd;	o1.ival = q->entity.entity.offset;	o2.ival = reg;
		access==0?emit(ctx,casm,opLD,&o,&o1,&o2):emit(ctx,casm,opST,&o,&o1,&o2);
		break;
	case SLOCAL:
		o.ival = rf;	o1.ival = q->entity.entity.offset+1;	o2.ival = reg;
		access==0?emit(ctx,casm,opLD,&o,&o1,&o2):emit(ctx,casm,opST,&o,&o1,&o2);
		break;
	case SPARAMETER:	/* add the pass-by-address option here */
		o.ival = rf;	o1.ival = -q->entity.entity.offset-2;	o2.ival = reg;
		access==0?emit(ctx,casm,opLD,&o,&o1,&o2):emit(ctx,casm,opST,&o,&o1,&o2);
		break;
	case STEMP:
		o.ival = rf;	o1.ival = local+temp-q->entity.entity.offset;
		o2.ival = reg;
		access==0?emit(ctx,casm,opLD,&o,&o1,&o2):emit(ctx,casm,opST,&o,&o1,&o2);
		break;
	case SCONSTANT:
		switch (gettype(q))
		{
		case TINTEGER:
			o.ival = q->entity.entity.ival;		o1.ival = reg;
			emit(ctx, casm, opLCI, &o, NULL, &o1);
			break;
		case TFLOAT:
			o.fval = q->entity.entity.fval;		o1.ival = reg;
			emit(ctx, casm, opLCF, &o, NULL, &o1);
			break;
		case TSTRING:
			o.ival = q->entity.entity.ival;		o1.ival = reg;
			emit(ctx, casm, opNewstring, &o,0,&o1);
			break;
		case TNIL:
			o1.ival = reg;
			emit(ctx, casm, opLNIL, 0, 0, &o1);
			break;
		case THFUNCTION:
			o.ival = q->entity.entity.ival;		o1.ival = reg;
			emit(ctx, casm, opLHF, &o, 0, &o1);
			break;
		}
		break;
	case SFUNCTION:
		o.ival = q->entity.entity.ival;		o1.ival = reg;
		emit(ctx, casm, opLFUNC, &o, NULL, &o1);
		break;
	case SHFUNCTION:
		o.ival = q->entity.entity.ival;		o1.ival = reg;
		emit(ctx, casm, opLHF, &o, NULL, &o1);
		break;
	}
}

void loadreg(context* ctx, assembly* casm, qoperand* q, int reg, int local, int temp)
{
	accessreg(ctx, casm, 0, q, reg, local, temp);
}

void storereg(context* ctx, assembly* casm, qoperand* q, int reg, int local, int temp)
{
	accessreg(ctx, casm, 1, q, reg, local, temp);
}

void transquad(context* ctx, assembly* casm, int quadnum)
{
	operand o = {0}, o1 = {0}, o2 = {0};
	int local, temp;	/* this frame's (caller) size */
	int dlocal, dtemp;	/* next frame's (callee) size, used by calls */
	quadruple* quad;
	symbol* s = getquadfunc(ctx, quadnum);
	local = (s==0?0:s->localsize);
	temp = (s==0?0:s->tempsize);
	quad = &ctx->prog->quadruples[quadnum];
	s = getquadfunc(ctx, quad->opdest.entity.entity.ival);
	dlocal = (s==0?0:s->localsize);
	dtemp = (s==0?0:s->tempsize);
	switch (quad->op)
	{
	case opAssign:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		storereg(ctx, casm, &quad->opdest, ra, local, temp);
		break;
	case opNeg:			case opNot:			case opBitcom: case opFact:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		o.ival = ra;
		emit(ctx, casm, quad->op, &o, &o, &o);
		storereg(ctx, casm, &quad->opdest, ra, local, temp);
		break;
	case opLogand:		case opLogor:		case opBitand:	case opBitor:
	case opBitxor:		case opLshift:		case opRshift:	case opAdd:
	case opMinus:		case opMultiply:	case opDivide:	case opMod:
	case opEqual:		case opNeq:			case opGreater:	case opGeq:
	case opLess:		case opLeq:			case opConcat: case opPower:
	case opClass:       case opTEqual:		case opTNeq:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		loadreg(ctx, casm, &quad->op2, rb, local, temp);
		o.ival = ra;	o1.ival = rb;
		emit(ctx, casm, quad->op, &o, &o1, &o);
		storereg(ctx, casm, &quad->opdest, ra, local, temp);
		break;
	case opPush:
		loadreg(ctx, casm, &quad->opdest, ra, local, temp);
		o.ival = ra;
		emit(ctx, casm, quad->op, 0, 0, &o);
		break;
	case opPop:
		o.ival = quad->op2.entity.entity.ival+1;		/* shrink offset */
		o1.ival = quad->opdest.entity.entity.offset;/* pop target temporary offset */
		o2.ival = local+temp;	/* frame size */
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		break;
	case opJz:			case opJnz:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		o.ival = ra;	o2.ival = quad->opdest.entity.entity.ival;
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		break;
	case opJmp:		case opHalt:	case opCall:	case opCallhost:
	case opTRCall:
		o.ival = quad->op2.entity.entity.ival;
		o1.ival = dlocal+dtemp;	/* for opCall to grow the stack */
		o2.ival = quad->opdest.entity.entity.ival;
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		break;
	case opCallin:
		loadreg(ctx, casm, &quad->opdest, rb, local, temp);
		o1.ival = dlocal+dtemp;	/* for opCall to grow the stack */
		o2.ival = rb;
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		break;
	case opRet:
		loadreg(ctx, casm, &quad->opdest, ra, local, temp);
		o.ival = rf;	o1.ival = 1;	o2.ival = ra;
		emit(ctx, casm, opST, &o, &o1, &o2);	/* [rf+1] = ret val */
		emit(ctx, casm, quad->op, NULL, NULL, NULL);
		break;
	case opYield:
		loadreg(ctx, casm, &quad->opdest, ra, local, temp);
		o.ival = rs;	o1.ival = 0;	o2.ival = ra;
		emit(ctx, casm, opST, &o, &o1, &o2);	/* [rs] = ret val */
		emit(ctx, casm, quad->op, NULL, NULL, NULL);
		break;
	case opSaveexc:
		storereg(ctx, casm, &quad->opdest, re, local, temp);
		break;
	case opThrow:
		loadreg(ctx, casm, &quad->opdest, ra, local, temp);
		o.ival = ra;
		emit(ctx, casm, quad->op, NULL, NULL, &o);
		break;
	case opStart:
		emit(ctx, casm, quad->op, NULL, NULL, &o);
		break;
	case opUpdate:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		o.ival = ra; o1.ival = rb;
		emit(ctx, casm, quad->op, &o, NULL, &o1);
		storereg(ctx, casm, &quad->opdest, rb, local, temp);
		break;
	case opCopy:
		emit(ctx, casm, quad->op, 0, 0, 0);
		storereg(ctx, casm, &quad->op1, ra, local, temp);
		storereg(ctx, casm, &quad->op2, rb, local, temp);
		break;
	case opSave:
		emit(ctx, casm, quad->op, 0, 0, 0);
		storereg(ctx, casm, &quad->op1, ra, local, temp);
		storereg(ctx, casm, &quad->op2, rb, local, temp);
		break;
	case opRestore:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		loadreg(ctx, casm, &quad->op2, rb, local, temp);
		emit(ctx, casm, quad->op, 0, 0, 0);
		break;
	case opNewtable:
		o2.ival = ra;
		emit(ctx, casm, quad->op, NULL, NULL, &o2);
		storereg(ctx, casm, &quad->opdest, ra, local, temp);
		break;
	case opIR:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		loadreg(ctx, casm, &quad->op2, rb, local, temp);
		o.ival = ra;	o1.ival = rb;	o2.ival = ra;
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		storereg(ctx, casm, &quad->opdest, ra, local, temp);
		break;
	case opIA:
		loadreg(ctx, casm, &quad->op1, ra, local, temp);
		loadreg(ctx, casm, &quad->op2, rb, local, temp);
		loadreg(ctx, casm, &quad->opdest, rc, local, temp);
		o.ival = ra;	o1.ival = rb;	o2.ival = rc;
		emit(ctx, casm, quad->op, &o, &o1, &o2);
		break;
	default:
		error("Unknown quadruple.\n");
	}
}

/* Partition quadruples into basic blocks. */
void partition(context* ctx, intermediate* icode)
{
	int i;
	for (i=0; i<icode->nextslot; i++)
		icode->quadruples[i].isleader = 0;
	for (i=0; i<ctx->sbt.nextslot; i++)	/* first statement is leader */
		if (ctx->sbt.table[i].isfunction)
			icode->quadruples[ctx->sbt.table[i].firstquad].isleader = 1;
	for (i=0; i<icode->nextslot; i++)
	{
		if (isbranch(&icode->quadruples[i]))	/* branch target is leader */
			icode->quadruples[icode->quadruples[i].opdest.entity.entity.ival].isleader = 1;
		/* branch follower is leader */
		if (isbbranch(&icode->quadruples[i]) && i+1<icode->nextslot)
		{
			icode->quadruples[i].isleader = 1;
			icode->quadruples[i+1].isleader = 1;
		}
	}
}

void translate(context* ctx, assembly* casm)
{
	int i,j,k,l,m;
	symbol *s, *t;
	intermediate* icode = ctx->prog;
	tryarr* ta;
	if (!icode) return;
	partition(ctx, icode);
	/*printquadruples(icode);*/
	for (i=0; i<icode->nextslot; i++)
	{
		icode->quadruples[i].firstinstruction = casm->nextslot;
		ctx->inleader = icode->quadruples[i].isleader;
		ctx->insord = 0;
		transquad(ctx, casm, i);
		icode->quadruples[i].lastinstruction = casm->nextslot-1;
	}
	/* Adjust branch instructions' target address. */
	for (i=0; i<casm->nextslot; i++)
		if (isbranch(&casm->instructions[i]))
		{
			j = casm->instructions[i].opdest.ival;
			casm->instructions[i].opdest.ival = icode->quadruples[j].firstinstruction;
		}
	/* Adjust functional variables. */
	for (i=0; i<ctx->funcs.nextslot; i++)
	{
			j = ctx->funcs.funcs[i].slot;
			s = lookup(ctx, &ctx->sbt, ctx->funcs.funcs[i].caller);
			k = icode->quadruples[j+s->firstquad].firstinstruction;
			l = icode->quadruples[j+s->firstquad].lastinstruction;
			if (!ishostfunction(ctx->funcs.funcs[i].callee))
			{
				t = lookup(ctx, &ctx->sbt, ctx->funcs.funcs[i].callee);
				for (m=k; m<=l; m++)
					if (casm->instructions[m].op == opLFUNC)
						casm->instructions[m].op1.ival = icode->quadruples[t->firstquad].firstinstruction;
			}
			else
				casm->instructions[k].op1.ival = (int)zen_getfunc(ctx->funcs.funcs[i].callee);
	}

	/* Adjust tryblock records for each of the functions. */
	for (i=0; i<ctx->sbt.nextslot; i++)
		if (ctx->sbt.table[i].isfunction)
		{
			if ((ta=ctx->sbt.table[i].ta) == 0)
				continue;
			for (j=0; j<ta->nextslot; j++)
			{
				ta->tr[j].first += ctx->sbt.table[i].firstquad;
				ta->tr[j].first = icode->quadruples[ta->tr[j].first].firstinstruction;
				ta->tr[j].last += ctx->sbt.table[i].firstquad;
				ta->tr[j].last = icode->quadruples[ta->tr[j].last].lastinstruction;
				ta->tr[j].catchentry += ctx->sbt.table[i].firstquad;
				ta->tr[j].catchentry = icode->quadruples[ta->tr[j].catchentry].firstinstruction;
			}
		}
	casm->stacksize = ZEN_DEFAULT_STACK_SIZE;
	/*printinstructions(casm);*/
}
