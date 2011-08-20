/*
 * The parser.
 * See Copyright Notice in Zen.h.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "zen.h"
#include "aparse.h"
#include "agen.h"
#include "alib.h"
#include "aerror.h"
#include "autil.h"

#define clearlabel(si) (si)->nextslot=0
#define assignop(t) ((t)=='='||(t)==PASS||(t)==MASS||(t)==MULASS||(t)==DASS\
					||(t)==MODASS||(t)==LSASS||(t)==RSASS||(t)==BAASS\
					||(t)==BOASS||(t)==BXASS)
#define getlasterror(ctx) (ctx->lasterror)	/* Get context's current error. */

tree_node* expression(LexInfo* li, context* ctx);
tree_node* makelist(LexInfo* li, context* ctx, tree_node* (*f)(LexInfo* li, context* ctx), char seperate);
tree_node* postfix(LexInfo* li, context* ctx);
void dostatement(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si);
void docompound(LexInfo* li, context* ctx, intermediate** pp, symboltable* sbtp, int isfuncbody, tree_node* fn, statementinfo* si, int pls, int pts);
context* findcontextroot(context* ctx);
qoperand* genexpr(context* ctx, tree_node* t, intermediate** pp, qoperand* genO, int deltree);
tree_node* primary(LexInfo* li, context* ctx);
tree_node* conditional(LexInfo* li, context* ctx);
qoperand* genfunc(context* ctx, tree_node* t, intermediate** pp);

static const int synchronization[] =
{
	WHILE, FOR, FOREACH, IF, RETURN, BREAK, DEF, ADVICE, POINTCUT, ENDOFFILE
};

/* Mark the error in the context. */
void setlasterror(context* ctx, int errorno, const char* msg, ...)
{
	char temp[1024];
	va_list vl;
	va_start(vl, msg);
	vsprintf(temp, msg, vl);
	va_end(vl);
	error(temp);
	ctx->lasterror = errorno;
	return;
}

/*
	Check the context and all its parents' validity.
	If any of the contexts contains any error, return 0; otherwise 1.
*/
int validatecontext(context* ctx)
{
	return getlasterror(ctx)!=ZEN_NO_ERROR?
		0:(ctx->parent!=NULL?validatecontext(ctx->parent):1);
}

/* Each line is reported only once for errors. */
void parseerror(LexInfo* li, context* ctx)
{
	char* token_name;
	if (li->lineerrors[li->line_num] == 0)
	{
		token_name = clGetTokenName(li->expected);
		if (!token_name)	/* general error */
			setlasterror(findcontextroot(ctx), ZEN_PARSER_SYNTAX_ERROR,
		"%s: syntax error, line %d, pos %d\n", li->source, li->line_num, li->pos - strlen(li->token.dbuf));
		else
			setlasterror(findcontextroot(ctx), ZEN_PARSER_SYNTAX_ERROR,
		"%s: %s expected, line %d, pos %d\n", li->source, token_name, li->line_num, li->pos - strlen(li->token.dbuf));
		li->lineerrors[li->line_num]++;
	}
}

/* Get the last evaluated operand. */
qoperand* getlastoperand(intermediate* p)
{
	return (!p||!p->nextslot)?(qoperand*)0:
					&(p)->quadruples[(p)->nextslot-1].opdest;
}

/* Insert a new identifier. */
int insertid(context* ctx, symboltable* sbt, const char* name)
{
	int nextslot = sbt->nextslot;
	if (nextslot>=1024)
		return 0;
	strcpy(sbt->table[nextslot].name, name);
	sbt->table[nextslot].offset = sbt->offset++;
	sbt->table[nextslot].isfunction = 0;
	sbt->nextslot++;
	return 1;
}

/* Look up an identifier in a symbol table. */
symbol* lookup(context* ctx, symboltable* sbt, const char* name)
{
	int i;
	if (!sbt) return NULL;
	for (i=0; i<sbt->nextslot; i++)
		if (strcmp(sbt->table[i].name, name) == 0)
			return &sbt->table[i];
	return NULL;
}

/* Look up an identifier in the context list. */
symbol* lookupall(LexInfo* li, context* ctx, const char* name)
{
	symbol* s;
	context* lctx = ctx;
	if (s=lookup(lctx, ctx->sbtp, name))
	{
		s->scope = SPARAMETER;
		return s;
	}
	while (lctx!=NULL && (s=lookup(ctx, &lctx->sbt, name))==NULL)
		lctx = lctx->parent;
	if (s!=NULL)
		s->compoundlevel = lctx->compoundlevel;
	return s;
}

/* Insert a new function into the symbol table. */
int insertfunc(context* ctx, symboltable* sbt, const char* name,
			   intermediate* p, int first, int last, int localsize,
			   int tempsize, char numnamedparams, char paramsvariable)
{
	symbol* s = lookup(ctx, sbt, name);
	if (s == NULL)
	{
		if (sbt->nextslot>=1024) return 0;
		s = &sbt->table[sbt->nextslot++];
	}
	s->isfunction = 1;
	s->ishost = (ishostfunction((char*)name)?1:0);
	strcpy(s->name, name);
	s->prog = p;
	s->firstquad = first;
	s->lastquad = last;
	s->tempsize = tempsize;
	s->localsize = localsize;
	s->numnamedparams = numnamedparams;
	s->paramsvariable = paramsvariable;
	return 1;
}

/* Scan to the next synchronized token. */
void synchronize(LexInfo* li)
{
	int i;
l:	for (i=0; i<sizeof(synchronization)/sizeof(int); i++)
		if (li->next_token.token == synchronization[i])
			return;
	clStep(li);
	goto l;
}

void match(LexInfo* li, context* ctx, int token)
{
	clStep(li);
	if (li->token.token != token)
	{
		li->expected = token;
		parseerror(li, ctx);
		li->expected = UNDEFINED;	/* reset the error */
		synchronize(li);
	}
}

/* Returns the operator info given a token. */
_operator* getop(const _operator* operators, int numop, int token)
{
	int i;
	for (i=0; i<numop; i++)
		if (operators[i].op == token)
			return (_operator*)&operators[i];
	return NULL;
}

void freenode(tree_node* t)
{
	if (!t) return;
	freetoken(&t->token);
	if (t->next) freenode(t->next);
	if (t->left) freenode(t->left);
	if (t->right) freenode(t->right);
	if (t->sibling) freenode(t->sibling);
	free(t);
}

tree_node* makenode(context* ctx, int token, tree_node* left, tree_node* right, tree_node* sibling)
{
	tree_node* t = (tree_node*)calloc(1, sizeof(tree_node));
	if (t == NULL)
	{
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
		return NULL;
	}
	t->isleaf = 0;
	t->op = token;
	t->left = left;
	t->right = right;
	t->sibling = sibling;
	if (left) left->parent = t;
	if (right) right->parent = t;
	return t;
}

tree_node* makeleaf(context* ctx, _token token)
{
	tree_node* t = (tree_node*)calloc(1, sizeof(tree_node));
	if (!t)
	{
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
		return NULL;
	}
	t->isleaf = 1;
	copytoken(&t->token, &token);
	return t;
}

tree_node* exprlist(LexInfo* li, context* ctx)
{
	return makelist(li, ctx, expression, ',');
}

tree_node* id(LexInfo* li, context* ctx)
{
	match(li, ctx, IDENTIFIER);
	return makeleaf(ctx, li->token);
}

tree_node* idlist(LexInfo* li, context* ctx)
{
	return makelist(li, ctx, id, ',');
}

/* var: IDENTIFIER | postfixexpr "[" expr "]" | postfixexpr "." IDENTIFIER */
tree_node* var(LexInfo* li, context* ctx)
{
	tree_node* t;
	t = postfix(li, ctx);
	if (li->next_token.token == '[')
	{
		clStep(li);
		t = makenode(ctx, INDEX, t, expression(li, ctx), NULL);
		match(li, ctx, ']');
	}
	else if (li->next_token.token == '.')
	{
		clStep(li);
		match(li, ctx, IDENTIFIER);
		t = makenode(ctx, INDEX, t, makeleaf(ctx, li->token), NULL);
	}
	return t;
}

/* varlist:	var "," varlist | var */
tree_node* varlist(LexInfo* li, context* ctx)
{
	return makelist(li, ctx, var, ',');
}

tree_node* field(context* ctx, LexInfo* li)
{
	tree_node* t;
	match(li, ctx, IDENTIFIER);
	t = makeleaf(ctx, li->token);
	while (li->next_token.token == '.')
	{
		clStep(li);
		match(li, ctx, IDENTIFIER);
		t = makenode(ctx, INDEX, t, makeleaf(ctx, li->token), NULL);
	}
	return t;
}

tree_node* funcname(context* ctx, LexInfo* li)
{
	tree_node* t;
	t = field(ctx, li);
	if (li->next_token.token == BEGIN)
	{
		clStep(li);
		match(li, ctx, IDENTIFIER);
		t = makenode(ctx, SCOPE, t, makeleaf(ctx, li->token), NULL);
	}
	return t;
}

char paramlist(LexInfo* li, context* ctx, symboltable* sbt)
{
	tree_node *t, *t1;
	char paramsvariable = 0;
	t = t1 = idlist(li, ctx);
	for (;t1!=0&&t1->token.token!=TRIPLEDOT;t1->scope=SPARAMETER,t1=t1->next)
		insertid(ctx, sbt, t1->token.dbuf);
	if (t1 && t1->token.token == TRIPLEDOT)
		paramsvariable = 1;
	if (t)
		freenode(t);
	return paramsvariable;
}

tree_node* cell(LexInfo* li, context* ctx)
{
	tree_node* t;
	t = conditional(li, ctx);
	if (li->next_token.token == CELL)
	{
		clStep(li);
		t = makenode(ctx, ':', t, conditional(li, ctx), NULL);
	}
	return t;
}

tree_node* celllist(LexInfo* li, context* ctx)
{
	return makelist(li, ctx, cell, ',');
}

/* Construct a tree node t from symbol s. */
void sym2node(symbol* s, tree_node* t)
{
	t->scope = (s->scope==SPARAMETER?SPARAMETER:(s->compoundlevel==0?SGLOBAL:SLOCAL));
	if (s->isfunction)
		t->scope = SFUNCTION;
	if (s->ishost)
		t->scope = SHFUNCTION;
	t->offset = s->offset;
	t->type = s->isfunction?IDFUNCTION:IDIDENTIFIER;
}

tree_node* primary(LexInfo* li, context* ctx)
{
	int token;
	symbol* s;
	char msg[1024] = "";
	tree_node* t=NULL;
	token = li->next_token.token;
	li->expected = UNDEFINED;
	switch (token)
	{
	case IDENTIFIER:
		clStep(li);
		if (li->next_token.token == '=')	/* a possible declaration */
			if (!lookupall(li, ctx, li->token.dbuf))
			{
				insertid(ctx, &ctx->sbt, li->token.dbuf);
				sprintf(msg, "%s: implicit variable declaration, line %d, pos %d\n", li->source, li->line_num, li->pos - strlen(li->token.dbuf));
				warning(msg);
			}
		if (!(s=lookupall(li, ctx, li->token.dbuf)) && !ishostfunction(li->token.dbuf))
			setlasterror(findcontextroot(ctx), ZEN_PARSER_UNDEFINED,
			"%s: identifier \"%s\" undefined, line %d, pos %d.\n", li->source, li->token.dbuf, li->line_num, li->pos - strlen(li->token.dbuf));
		t = makeleaf(ctx, li->token);
		if (s) sym2node(s, t);
		t->iscall = (li->next_token.token=='(');
		if (ishostfunction(li->token.dbuf))
			t->type = IDFUNCTION;
		break;
	case INTEGER: case FLOATING: case LITERAL:	case NIL:
		clStep(li);
		t = makeleaf(ctx, li->token);
		t->scope = SCONSTANT;
		break;
	case '(':
		clStep(li);
		t = expression(li, ctx);
		match(li, ctx, ')');
		break;
	case '[':	/* a table constructor */
		clStep(li);
		if (li->next_token.token == ']')
			clStep(li);
		else
		{
			t = celllist(li, ctx);
			match(li, ctx, ']');
		}
		t = makenode(ctx, TCONSTRUCT, t, NULL, NULL);
		break;
	default:
		parseerror(li, ctx);
	}
	return t;
}

/* args: "(" ")" | "(" exprlist ")" */
tree_node* args(LexInfo* li, context* ctx)
{
	tree_node* t = NULL;
	match(li, ctx, '(');
	if (li->next_token.token != ')')
		t = exprlist(li, ctx);
	match(li, ctx, ')');
	return t;
}

/* Count the number of siblings(including itself) of a tree node. */
int numofsiblings(tree_node* t)
{
	int num = 0;
	tree_node* tt = t;
	for (; tt!=NULL; num++, tt=tt->next);
	return num;
}

/* Check arguments against a function definition. */
void checkargs(LexInfo* li, context* ctx, const char* funcname, tree_node* args)
{
	symbol* s = lookupall(li, ctx, funcname);
	/* indirect calls and host functions are exempt from this checking */
	if (!s->isfunction || s->ishost) return;
	if (!s->paramsvariable&&s->numnamedparams!=numofsiblings(args) ||
		s->paramsvariable&&s->numnamedparams>numofsiblings(args))
		setlasterror(ctx, ZEN_PARSER_DIFFERENT_NUMOFARGS,
			"%s: number of arguments is different from definition, line %d, pos %d\n",
			li->source, li->line_num, li->pos - strlen(li->token.dbuf));
}

/*
	postfixexpr: primexpr | postfixexpr args | postfixexpr ":" IDENTIFIER args
				postfixexpr "[" expr "]" | postfixexpr "." IDENTIFIER
				| postfixexpr INC | postfixexpr DEC
*/
tree_node* postfix(LexInfo* li, context* ctx)
{
	tree_node *t, *t1, *t2, *ot;
	char argschecked = 0;	/* check arguments only once */
	t1 = t = primary(li, ctx);
l:
	switch (li->next_token.token)
	{
    case '[':
		clStep(li);
		t = makenode(ctx, INDEX, t, expression(li, ctx), NULL);
		match(li, ctx, ']');
		break;

    case '.':
		clStep(li);
		clStep(li);
		li->token.token = LITERAL;
		t2 = makeleaf(ctx, li->token);
		t2->scope = SCONSTANT;
		t = makenode(ctx, INDEX, t, t2, NULL);
		break;

	case '(':	/* it's a function call, check # of arguments here */
		ot = t;
		t = makenode(ctx, FUNC, t, NULL, args(li, ctx));
		if (!argschecked && !ishostfunction(ot->token.dbuf)
			&& lookupall(li, ctx, ot->token.dbuf))
		{
			checkargs(li, ctx, t1->token.dbuf, t->sibling);
			argschecked = 1;
		}
		break;

		/*
	case ':':
		clStep(li);
		match(li, ctx, IDENTIFIER);
		t = makenode(ctx, SCOPE, t, makeleaf(ctx, li->token), NULL);
		t = makenode(ctx, CALLARGS, t, args(li, ctx), NULL);
		break;
		*/
	case INC:
		clStep(li);
		t = makenode(ctx, POSTINC, t, NULL, NULL);
		break;
	case DEC:
		clStep(li);
		t = makenode(ctx, POSTDEC, t, NULL, NULL);
		break;
	default:
		return t;
	}
	goto l;
	return t;
}

/*
	unaryexpr: postfixexpr | INC unaryexpr | DEC unaryexpr | ~ unaryexpr
*/
tree_node* unary(LexInfo* li, context* ctx)
{
	tree_node* t;
	_operator* op;
	op = getop(unaoperators, NUMOP(unaoperators), li->next_token.token);
	if (op != NULL)	/* a unary operator */
	{
		clStep(li);
		t = unary(li, ctx);
		switch (op->op)
		{
		case INC: t = makenode(ctx, PREINC, t, NULL, NULL); break;
		case DEC: t = makenode(ctx, PREDEC, t, NULL, NULL); break;
		default: t = makenode(ctx, op->op, t, NULL, NULL); break;
		}
	}
	else	/* a postfix expression */
		t = postfix(li, ctx);
	return t;
}

/* Refer to the document for this parameterized recursive descent parsing. */
tree_node* expression1(LexInfo* li, int precedence, context* ctx)
{
	int q;	/* precedence for next level expression */
	tree_node *t, *t1;
	_operator* op;
	t = unary(li, ctx);
	op = getop(binoperators, NUMOP(binoperators), li->next_token.token);
	while (op != NULL && op->precedence >= precedence)
	{
		clStep(li);
		q = (op->associativity==LEFT?op->precedence+1:op->precedence);
		t1 = expression1(li, q, ctx);
		t = makenode(ctx, op->op, t, t1, NULL);
		op = getop(binoperators, NUMOP(binoperators), li->next_token.token);
	}
	if (t)
		t->line_num = li->line_num;
	return t;
}

tree_node* conditional(LexInfo* li, context* ctx)
{
	tree_node *t, *t1;
	t = expression1(li, 0, ctx);
	if (li->next_token.token == '?')
	{
		clStep(li);
		t1 = expression(li, ctx);
		match(li, ctx, BEGIN);
		t = makenode(ctx, '?', t, t1, conditional(li, ctx));
	}
	return t;
}

/* Insert the id on the left into the symbol table. */
tree_node* assign(LexInfo* li, context* ctx)
{
	tree_node *t, *t1;
	int token;
	t = conditional(li, ctx);
	if (assignop(li->next_token.token))
	{
		token = li->next_token.token;
		clStep(li);
		t1 = assign(li, ctx);
		t = makenode(ctx, token, t, t1, NULL);
	}
	return t;
}

/* Entry function of expression parsing. */
tree_node* expression(LexInfo* li, context* ctx)
{
	tree_node *t,*t1;
	t = t1 = assign(li, ctx);
	while (li->next_token.token == ',')
	{
		clStep(li);
		if ((t1->next=assign(li, ctx)) == NULL)
			return NULL;
		t1->next->prev = t1;
		t1 = t1->next;
	}
	return t;
}

void setivalue(qoperand* o, int ivalue)
{
	o->entity.tag = TINTEGER;
	o->entity.entity.ival = ivalue;
}

void setfvalue(qoperand* o, float fvalue)
{
	o->entity.tag = TFLOAT;
	o->entity.entity.fval = fvalue;
}

void setsvalue(qoperand* o, const char* svalue)
{
	o->entity.tag = TSTRING;
	o->entity.entity.sval = (char*)svalue;
}

/* Deposit an operand for later deallocation. They are stored globally. */
void depositoperand(context* ctx, qoperand* o)
{
	context* ctxroot = findcontextroot(ctx);
	if (ctxroot->qophead == NULL)
		ctxroot->qophead = ctxroot->qcurop = o;
	else
		ctxroot->qcurop = (ctxroot->qcurop->next = o);
}

/* Free the operand list in ctx. */
void freeoperands(context* ctx)
{
	qoperand* o = ctx->qophead, *o1;
	while (o!=NULL)
	{
		o1=o->next;
		if (gettype(o) == TSTRING)
			free(o->entity.entity.sval);
		free(o);
		o=o1;
	}
}

/* Allocate and deposit an operand.*/
qoperand* newoperand(context* ctx)
{
	qoperand* o = calloc(1, sizeof(qoperand));
	if (o == NULL)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory!\n");
	depositoperand(ctx, o);
	return o;
}

qoperand* newtemporary(context* ctx)
{
	qoperand* o = newoperand(ctx);
	if (!o) return NULL;
	o->scope = STEMP;
	o->entity.entity.offset = ctx->temp++;
	return o;
}

/* Create an integer constant with a value v. */
qoperand* newconstanti(context* ctx, int v)
{
	qoperand* o = newoperand(ctx);
	settype(o, TINTEGER);
	if (!o) return NULL;
	o->scope = SCONSTANT;
	o->entity.entity.ival = v;
	return o;
}

/* Build an instruction into the quadruple program by *p. */
void qbuild(context* ctx, intermediate** pp, eOpcode op, qoperand* op1, qoperand* op2, qoperand* opdest)
{
	qoperand o = {0};	/* a dummy operand for padding */
	intermediate* p = *pp;
	int nextslot = p->nextslot;
	if (nextslot >= p->size)	/* buffer is full */
	{
		p->size = p->size*2+1;
		if ((p->quadruples=realloc(p->quadruples, p->size*sizeof(quadruple))) == NULL)
			setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Not enought memory.\n");
	}
	p->quadruples[nextslot].op = op;
	p->quadruples[nextslot].op1 = (op1!=NULL?*op1:o);
	p->quadruples[nextslot].op2 = (op2!=NULL?*op2:o);
	p->quadruples[nextslot].opdest = (opdest!=NULL?*opdest:o);
	p->nextslot++;
}

/* qbuild a a branching instruction with destination address slot. */
void qbuildbranch(context* ctx, intermediate** pp, eOpcode op, qoperand* qcon, int slot)
{
	qoperand o={0};
	o.scope = SCONSTANT;
	settype(&o, TINTEGER);
	o.entity.entity.ival = slot;
	qbuild(ctx, pp, op, qcon, NULL, &o);
}

/* Change opdest of instruction at slot1 to pos for branching quadruples. */
void patch(context* ctx, intermediate* p, int slot, int pos)
{
	p->quadruples[slot].opdest.scope = SCONSTANT;
	settype(&p->quadruples[slot].opdest, TINTEGER);
	p->quadruples[slot].opdest.entity.entity.ival = pos;
}

/* Find the root of an expression tree node. */
tree_node* findexprroot(tree_node* t)
{
	tree_node* r;
	for (r=t; r!=NULL && r->parent!=NULL; r=r->parent);
	return r;
}

/* Find the topmost(global) context. */
context* findcontextroot(context* ctx)
{
	context* c;
	for (c=ctx; c!=NULL && c->parent!=NULL; c=c->parent);
	return c;
}

/* Find the context of the function where ctx is in. */
context* findcontextfunction(context* ctx)
{
	context* c;
	for (c=ctx; !c->funcname&&c->parent!=NULL; c=c->parent);
	return c;
}

char* findname(context* ctx)
{
	context* c = findcontextfunction(ctx);
	return c->funcname?c->funcname->token.dbuf:NULL;
}

/* Mark "break" or "continue" statements for backpatching. */
void addtag(context* ctx, statementinfo* si, char type, float slot, tree_node* expr)
{
	/* check bounds here */
	si->tags[si->nextslot].type = type;
	si->tags[si->nextslot].slot = slot;
	si->tags[si->nextslot++].expr = expr;
}

/* si1 = si1+si2 */
void mergetags(context* ctx, statementinfo* si1, statementinfo* si2)
{
	int i;
	for (i=0; i<si2->nextslot; i++)
		addtag(ctx, si1, si2->tags[i].type, si2->tags[i].slot, si2->tags[i].expr);
}

/* Patch case and default statements. */
void patchlabels(context* ctx, intermediate** pp, statementinfo* si, qoperand* caseop)
{
	int i;
	qoperand *q, *q1;
	for (i=0; i<si->nextslot; i++)
		switch (si->tags[i].type)
		{
		case LCASE:
			q = genexpr(ctx, si->tags[i].expr, pp, NULL, 1);
			q1 = newtemporary(ctx);	/* validity must be checked here */
			qbuild(ctx, pp, opEqual, q, caseop, q1);
			qbuildbranch(ctx, pp, opJnz, q1, si->tags[i].slot);
			break;
		case LDEFAULT:
			qbuildbranch(ctx, pp, opJmp, 0, si->tags[i].slot);
			break;
		}
}

/* Patch break and continue statements. */
void patchtransfers(context* ctx, intermediate** pp, statementinfo* si, int breakslot, int continueslot, qoperand* caseop)
{
	int i;
	for (i=0; i<si->nextslot; i++)
	{
		switch (si->tags[i].type)
		{
		case TBREAK:
			(*pp)->quadruples[si->tags[i].slot].opdest.entity.entity.ival = breakslot;
			break;
		case TCONTINUE:
			(*pp)->quadruples[si->tags[i].slot].opdest.entity.entity.ival = continueslot;
			break;
		}
	}
}

void addcall(context* ctx, float slot, const char* caller, const char* callee)
{
	callarr* ca = &ctx->calls;
	if (ishostfunction((char*)callee))
		return;
	if (ca->nextslot>=ca->size)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	ca->calls[ca->nextslot].slot = slot;
	strcpy(ca->calls[ca->nextslot].caller, caller);
	strcpy(ca->calls[ca->nextslot].callee, callee);
	ca->nextslot++;
}

/* Patch all the call quadruples using the call array. */
void patchcalls(LexInfo* li, context* ctx)
{
	int i;
	symbol *s, *t;
	if (!validatecontext(ctx)) return;
	for (i=0; i<ctx->calls.nextslot; i++)
	{
		s = lookupall(li, ctx, ctx->calls.calls[i].caller);
		t = lookupall(li, ctx, ctx->calls.calls[i].callee);
		if (s == NULL)
			setlasterror(ctx, ZEN_PARSER_UNKNOWN_ERROR, "Unknown error occured\n.");
		else
			ctx->prog->quadruples[s->firstquad+ctx->calls.calls[i].slot].opdest.entity.entity.ival = t->firstquad;
	}
}

/* Add a function address record for backpatching. */
void addfunc(context* ctx, float slot, char operand, const char* caller, const char* callee)
{
	funcarr* fa = &ctx->funcs;
	if (fa->nextslot>=fa->size)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	fa->funcs[fa->nextslot].slot = slot;
	fa->funcs[fa->nextslot].operand = operand;
	strcpy(fa->funcs[fa->nextslot].caller, caller);
	strcpy(fa->funcs[fa->nextslot].callee, callee);
	fa->nextslot++;
}

/* Append a node n to the end of a tree node's "next" list. */
void appendnode(tree_node** tt, tree_node* n)
{
	tree_node *m, *t=*tt;
	if (!t)	/* empty list */
		*tt = n;
	else
	{
		for (m=t; m->next!=NULL; m=m->next);	/* search to end of list */
		m->next = n;
	}
}

/* Generate quadruples for postinc and postdec. */
void postgen(context* ctx, tree_node* t, intermediate** pp, int incordec)
{
	qoperand *o, *o1, *o2;
	if (!t) return;
	o = genexpr(ctx, t, pp, NULL, 0);
	o1 = newconstanti(ctx, 1);
	incordec?qbuild(ctx, pp,opAdd,o,o1,o):qbuild(ctx ,pp,opMinus,o,o1,o);
	if (t->op == INDEX)
	{
		o1 = genexpr(ctx, t->left, pp, NULL, 0);
		o2 = genexpr(ctx, t->right, pp, NULL, 0);
		qbuild(ctx, pp, opIA,  o1, o2, o);	/* write back to array element */
	}
}

/* Recursively generate all the post expressions for a node's children. */
void rpostgen(context* ctx, tree_node* t, intermediate** pp, int incordec)
{
	if (!t) return;
	rpostgen(ctx, t->next, pp, incordec);
	rpostgen(ctx, t->sibling, pp, incordec);
	rpostgen(ctx, t->left, pp, incordec);
	rpostgen(ctx, t->right, pp, incordec);
	postgen(ctx, t->postinc, pp, 1);
	postgen(ctx, t->postdec, pp, 0);
}

void gencells(context* ctx, intermediate** pp, tree_node* t, qoperand* o)
{
	tree_node* n = t->left;
	int numele = 0;
	for (; n!=NULL; n=n->next)
		if (n->op == BEGIN)
			qbuild(ctx, pp, opIA, o, genexpr(ctx, n->left, pp, NULL, 0), genexpr(ctx, n->right, pp, NULL, 0));
		else
			qbuild(ctx, pp, opIA, o, newconstanti(ctx, numele++),
										genexpr(ctx, n, pp, NULL, 0));
}

/* Return the integer value for a hex character. */
int getcharint(char c)
{
	int i;
	char hexchars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f'};
	for (i=0; i<sizeof(hexchars)/sizeof(char); i++)
		if (hexchars[i] == (char)tolower(c))
			return i;
	return 0;
}

/* Parse both hex and decimal integers. */
int parseint(const char* numstr)
{
	int ret, i=2;
	if (numstr[0]=='0' && numstr[1]=='x')	/* hex */
	{
		ret = getcharint(numstr[i++]);
		for (; i<(int)strlen(numstr); i++)
			ret = ret*16+getcharint(numstr[i]);
		return ret;
	}
	else
		return atoi(numstr);	/* decimal */
}

/* Generate quadruples from the syntax tree. */
#define isarrassign(t) ((t)->op=='='&&(t)->left!=NULL&&(t)->left->op==INDEX)
qoperand* genexpr(context* ctx, tree_node* t, intermediate** pp,
				  qoperand* genO, int delnode)
{
	qoperand *o,*o1,*o2,*o3;
	int slot1, slot2;
	if (!t) return (qoperand*)0;
	if (!validatecontext(ctx))
	{
		if (t) freenode(t);
		return (qoperand*)0;
	}
	if (t->isleaf)
	{
		if ((o=newoperand(ctx)) == NULL) return NULL;
		if (t->type==IDFUNCTION || (ishostfunction(t->token.dbuf)&&t->token.token!=LITERAL))
		{
			o->scope = (t->type==IDFUNCTION?SFUNCTION:SHFUNCTION);
			if (ishostfunction(t->token.dbuf))
				o->scope = SHFUNCTION;
			/* drop the record to the funcarr */
			addfunc(findcontextroot(ctx), (*pp)->nextslot, 0,
				findcontextfunction(ctx)->funcname->token.dbuf, t->token.dbuf);
		}
		else
		{
			switch (t->token.token)
			{
			case IDENTIFIER:
				settype(o, TNIL);
				o->entity.entity.offset = t->offset;
				break;
			case INTEGER: setivalue(o, parseint(t->token.dbuf)); break;
			case FLOATING: setfvalue(o, (float)atof(t->token.dbuf)); break;
			case LITERAL: setsvalue(o, cpystr(t->token.dbuf)); break;
			case NIL: settype(o, TNIL); break;
			}
			o->scope = t->scope;
		}
	}
	else
	{
		if ((o=newtemporary(ctx)) == NULL) return NULL;
		if (t->right == NULL)	/* unary operator */
		{
			switch (t->op)
			{
			case PREINC:	case PREDEC:
				o = genexpr(ctx, t->left, pp, NULL, 0);
				o1 = newconstanti(ctx, 1);
				if (t->op == PREINC)
					qbuild(ctx, pp, opAdd, o, o1, o);
				else
					qbuild(ctx, pp, opMinus, o, o1, o);
				if (!t->left->isleaf && t->left->op==INDEX)
				{
					o1 = genexpr(ctx, t->left->left, pp, NULL, 0);
					o2 = genexpr(ctx, t->left->right, pp, NULL, 0);
					/* write back to the array element */
					qbuild(ctx, pp, opIA, o1, o2, o);
				}
				break;
			case POSTINC:
				appendnode(&findexprroot(t)->postinc, t->left);
				o = genexpr(ctx, t->left, pp, NULL, 0);
				break;
			case POSTDEC:
				appendnode(&findexprroot(t)->postdec, t->left);
				o = genexpr(ctx, t->left, pp, NULL, 0);
				break;
			case '+': return genexpr(ctx, t->left, pp, NULL, 0);
			case '-':
				qbuild(ctx, pp, opNeg, genexpr(ctx, t->left, pp, NULL, 0), NULL, o);
				break;
			case '!':
				qbuild(ctx, pp, opNot, genexpr(ctx, t->left, pp, NULL, 0), NULL, o);
				break;
			case FACT:
				qbuild(ctx, pp, opFact, genexpr(ctx, t->left, pp, NULL, 0), NULL, o);
				break;
			case '~':
				qbuild(ctx, pp, opBitcom, genexpr(ctx, t->left, pp, NULL, 0), NULL, o);
				break;
			case TCONSTRUCT:
				qbuild(ctx, pp, opNewtable, NULL, NULL, o);
				gencells(ctx, pp, t, o);
				break;
			case FUNC:
				o = genfunc(ctx, t->left, pp);
				break;
			}
		}
		else if (t->sibling == NULL)	/* binary operator */
		{
			o2 = genexpr(ctx, t->right, pp, NULL, 0);
			if (isarrassign(t))
				o1 = genexpr(ctx, t->left, pp, o2, 0);
			else
				o1 = genexpr(ctx, t->left, pp, NULL, 0);
			switch (t->op)
			{
			case '=':
				if (!isarrassign(t))
				{
					qbuild(ctx, pp, opAssign, o2, NULL, o1);
					o=o1;
				}
				break;
			case '+': qbuild(ctx, pp, opAdd, o1, o2, o); break;
			case '-': qbuild(ctx, pp, opMinus, o1, o2, o); break;
			case '*': qbuild(ctx, pp, opMultiply, o1, o2, o); break;
			case POWER: qbuild(ctx, pp, opPower, o1, o2, o); break;
			case CLASS: qbuild(ctx, pp, opClass, o1, o2, o); break;
			case '/': qbuild(ctx, pp, opDivide, o1, o2, o); break;
			case '%': qbuild(ctx, pp, opMod, o1, o2, o); break;
			case PASS:	case MASS:	case MULASS:	case DASS:	case MODASS:
			case LSASS:	case RSASS:	case BAASS:		case BOASS:	case BXASS:
				switch (t->op)
				{
				case PASS: qbuild(ctx, pp, opAdd, o1, o2, o1); break;
				case MASS: qbuild(ctx, pp, opMinus, o1, o2, o1); break;
				case MULASS: qbuild(ctx, pp, opMultiply, o1, o2, o1); break;
				case DASS: qbuild(ctx, pp, opDivide, o1, o2, o1); break;
				case MODASS: qbuild(ctx, pp, opMod, o1, o2, o1); break;
				case LSASS:	qbuild(ctx, pp, opLshift, o1, o2, o1); break;
				case RSASS:	qbuild(ctx, pp, opRshift, o1, o2, o1); break;
				case BAASS:	qbuild(ctx, pp, opBitand, o1, o2, o1); break;
				case BOASS: qbuild(ctx, pp, opBitor, o1, o2, o1); break;
				case BXASS: qbuild(ctx, pp, opBitxor, o1, o2, o1); break;
				}
				if (!t->left->isleaf && t->left->op==INDEX)
				{
					o2 =genexpr(ctx, t->left->left, pp, NULL, 0);
					o3 =genexpr(ctx, t->left->right, pp, NULL, 0);
					qbuild(ctx, pp, opIA, o2, o3, o1);
				}
				o = o1;
				break;
			case LOGOR: qbuild(ctx, pp, opLogor, o1, o2, o); break;
			case LOGAND: qbuild(ctx, pp, opLogand, o1, o2, o); break;
			case '&': qbuild(ctx, pp, opBitand, o1, o2, o); break;
			case '|': qbuild(ctx, pp, opBitor, o1, o2, o); break;
			case '^': qbuild(ctx, pp, opBitxor, o1, o2, o); break;
			case LSHIFT: qbuild(ctx, pp, opLshift, o1, o2, o); break;
			case RSHIFT: qbuild(ctx, pp, opRshift, o1, o2, o); break;
			case EQUAL: qbuild(ctx, pp, opEqual, o1, o2, o); break;
			case NOTEQUAL: qbuild(ctx, pp, opNeq, o1, o2, o); break;
			case TEQUAL: qbuild(ctx, pp, opTEqual, o1, o2, o); break;
			case TNOTEQUAL: qbuild(ctx, pp, opTNeq, o1, o2, o); break;
			case '>': qbuild(ctx, pp, opGreater, o1, o2, o); break;
			case GEQ: qbuild(ctx, pp, opGeq, o1, o2, o); break;
			case '<': qbuild(ctx, pp, opLess, o1, o2, o); break;
			case LEQ: qbuild(ctx, pp, opLeq, o1, o2, o); break;
			case DOUBLEDOT: qbuild(ctx, pp, opConcat, o1, o2, o); break;
			case INDEX:
				if (genO != NULL)	/* generate an array assignment */
					qbuild(ctx, pp, opIA, o1, o2, genO);
				else
					qbuild(ctx, pp, opIR, o1, o2, o);
				break;
			}
		}
		else	/* conditional operator*/
		{
			o = newtemporary(ctx);
			o1 = genexpr(ctx, t->left, pp, NULL, 0);
			slot1 = (*pp)->nextslot;
			qbuild(ctx, pp, opJz, o1, 0, 0);
			o2 = genexpr(ctx, t->right, pp, NULL, 0);
			qbuild(ctx, pp, opAssign, o2, 0, o);
			slot2 = (*pp)->nextslot;
			qbuildbranch(ctx, pp, opJmp, 0, 0);
			patch(ctx, *pp, slot1, (*pp)->nextslot);
			o3 = genexpr(ctx, t->sibling, pp, NULL, 0);
			qbuild(ctx, pp, opAssign, o3, 0, o);
			patch(ctx, *pp, slot2, (*pp)->nextslot);
		}
		if (t->op != TCONSTRUCT)
			genexpr(ctx, t->next, pp, NULL, 0);
	}
	if (delnode)	/* the roort expressoion node */
	{
		rpostgen(ctx, t, pp, 1);
		freenode(t);
	}
	return o;
}

/* Generate quadruples for a function call. */
qoperand* genfunc(context* ctx, tree_node* t, intermediate** pp)
{
	tree_node *arg, *args = t->parent->sibling;
	qoperand o={0};
	qoperand* q = newtemporary(ctx);
	qoperand r;	/* r.entity.entity.ival is # of arguments */
	qoperand opdest;
	if (t->op == FUNC)	/* function function */
		q = genfunc(ctx, t->left, pp);
	else if (!t->isleaf)
		q = genexpr(ctx, t, pp, 0, 0);	/* expression function */
	setivalue(&r, 0);
	r.scope = SCONSTANT;
	for (arg=args; arg!=0 && arg->next!=0; arg=arg->next);
	for (; arg!=0; arg=arg->prev, r.entity.entity.ival++)
		qbuild(ctx, pp, opPush, 0, 0, genexpr(ctx, arg, pp, NULL, 0));
	if (t->type == IDFUNCTION)	/* direct call */
	{
		addcall(findcontextroot(ctx), (*pp)->nextslot, findname(ctx), t->token.dbuf);
		if (ishostfunction(t->token.dbuf))
		{
			qbuildbranch(ctx, pp, opCallhost, NULL, (int)zen_getfunc(t->token.dbuf));
			qbuild(ctx, pp, opPop, 0, &r, q);
		}
		else
		{
			o.scope = SCONSTANT;
			settype(&o, TINTEGER);
			qbuild(ctx, pp, opCall, 0, &r, &o);
			qbuild(ctx, pp, opPop, 0, &r, q);
		}
	}
	else	/* indirect call */
	{
		if (t->isleaf)
		{
			opdest.scope = t->scope;
			opdest.entity.entity.ival = t->offset;
		}
		else
		{
			opdest.scope = q->scope;
			opdest.entity.entity.ival = q->entity.entity.ival;
		}
		qbuild(ctx, pp, opCallin, 0, 0, &opdest);
		qbuild(ctx, pp, opPop, 0, &r, q);
	}
	return q;
}

/*
	Make a list for identifiers, expression or variables.
	Take a function pointer f to make different nodes.
	seperate: the seperate symbol between list elements.
*/
tree_node* makelist(LexInfo* li, context* ctx, tree_node* (*f)(LexInfo* li, context* ctx), char seperate)
{
	tree_node *t, *t1;
	if (li->next_token.token == TRIPLEDOT)
	{
		clStep(li);
		t = makeleaf(ctx, li->token);
		return t;
	}
	t = t1 = (*f)(li, ctx);
	while (li->next_token.token == seperate)
	{
		if (t1 == NULL)
			return NULL;
		clStep(li);
		/* TRIPLEDOT token here for function declaration */
		if (li->next_token.token == TRIPLEDOT)
		{
			clStep(li);
			t1->next = makeleaf(ctx, li->token);
			return t;
		}
		else
			t1->next = (*f)(li, ctx);
		if (t1->next)
		{
			t1->next->prev = t1;
			t1 = t1->next;
		}
	}
	return t;
}


/* POINTCUT funcname "(" paramlist ")" ":" predexpr ";" */
void dopointcut(LexInfo* li, context* ctx)
{
	/*
	match(li, ctx, POINTCUT);
	funcname(ctx, li);
	match(li, ctx, '(');
	paramlist(li, ctx);
	match(li, ctx, ')');
	match(li, ctx, ':');
	match(li, ctx, ';');
	*/
}

/*
	ADVICE env funcname "(" paramlist ")" compound
	ADVICE env funcname "(" paramlist ")" ":" predexpr compound
*/
void doadvice(LexInfo* li)
{
}

void dofunction(LexInfo* li, context* pctx, intermediate** pp)
{
	intermediate* p;
	tree_node* fn;
	char paramsvariable=0;
	symboltable sbtp = {0};
	statementinfo si = {0};
	if (pctx->compoundlevel != 0)
		setlasterror(pctx, ZEN_PARSER_NO_LOCAL_FUNCTION,
		"No local function allowed, line %d.\n", li->line_num);
	match(li, pctx, DEF);
	fn = funcname(pctx, li);
	match(li, pctx, '(');
	if (li->next_token.token != ')')
		paramsvariable = paramlist(li, pctx, &sbtp);
	match(li, pctx, ')');
	insertfunc(pctx, &pctx->sbt, fn->token.dbuf, 0, 0, 0, si.localsize, si.tempsize,sbtp.offset,paramsvariable);
	if (li->next_token.token == BEGIN)		/* a definition */
	{
		docompound(li, pctx, &p, &sbtp, 1, fn, &si, 0, 0);
		insertfunc(pctx, &pctx->sbt, fn->token.dbuf, p, 0, 0, si.localsize, si.tempsize,sbtp.offset,paramsvariable);
	}
	else if (li->next_token.token != ';')
		setlasterror(pctx, ZEN_PARSER_SYNTAX_ERROR,
		"Syntax error, line %d.\n", li->line_num);
	freenode(fn);
}

/* BREAK */
void dobreak(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	match(li, ctx, BREAK);
	//match(li, ctx, ';');
	addtag(ctx, si, TBREAK, (*pp)->nextslot, NULL);
	qbuildbranch(ctx, pp, opJmp, 0, 0);
}

/* CONTINUE */
void docontinue(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	match(li, ctx, CONTINUE);
	//match(li, ctx, ';');
	addtag(ctx, si, TCONTINUE, (*pp)->nextslot, NULL);
	qbuildbranch(ctx, pp, opJmp, 0, 0);
}

/*
	RETURN
	RETURN expression
*/
void doreturn(LexInfo* li, context* ctx, intermediate** pp)
{
	clStep(li);
	if (li->line_size == li->pos)
		qbuild(ctx, pp, opRet, 0, 0, getlastoperand(*pp));
	else
		qbuild(ctx, pp, opRet, 0, 0, genexpr(ctx, expression(li, ctx), pp, NULL, 1));
	//match(li, ctx, ';');
}

void doyield(LexInfo* li, context* ctx, intermediate** pp)
{
	clStep(li);
	if (li->next_token.token == ';')
		qbuild(ctx, pp, opYield, 0, 0, getlastoperand(*pp));
	else
		qbuild(ctx, pp, opYield, 0, 0, genexpr(ctx, expression(li, ctx), pp, NULL, 1));
	//match(li, ctx, ';');
}

/*
	IF "(" expr ")" statement
	IF expr statement
	IF "(" expr ")" statement ELSE statement
	IF expr statement ELSE statement
*/
void doif(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	int slot1, slot2;
	qoperand* o;
	clStep(li);
	//match(li, ctx, '(');
	o = genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	slot1 = (*pp)->nextslot;
	qbuild(ctx, pp, opJz, o, NULL, NULL);
	//match(li, ctx, ')');
	match(li, ctx, THEN);
	dostatement(li, ctx, pp, si);
	if (li->next_token.token != ELSE)
		patch(ctx, *pp, slot1, (*pp)->nextslot);
	else
	{
		clStep(li);
		slot2 = (*pp)->nextslot;
		qbuild(ctx, pp, opJmp, NULL, NULL, NULL);
		patch(ctx, *pp, slot1, (*pp)->nextslot);
		dostatement(li, ctx, pp, si);
		patch(ctx, *pp, slot2, (*pp)->nextslot);
	}
}

/* SWITCH (expression) compound statement */
void doswitch(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	qoperand *q;
	int slot0, slot1;
	statementinfo lsi = {0};
	ctx->looplevel++;
	match(li, ctx, SWITCH);
	//match(li, ctx, '(');
	q = genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	slot0 = (*pp)->nextslot;
	qbuildbranch(ctx, pp, opJmp, 0, 0);
	match(li, ctx, BEGIN);
	docompound(li, ctx, pp, ctx->sbtp, 0, NULL, &lsi, ctx->sbt.offset, ctx->temp);
	slot1 = (*pp)->nextslot;
	qbuildbranch(ctx, pp, opJmp, 0, 0);
	patch(ctx, *pp, slot0, (*pp)->nextslot);
	patchlabels(ctx, pp, &lsi, q);		/* case and default statements */
	patchtransfers(ctx, pp, &lsi, (*pp)->nextslot, 0, q);/* break statements */
	patch(ctx, *pp, slot1, (*pp)->nextslot);
	ctx->looplevel--;
}

/*
	CASE expr: statement;
	DEFAULT: statement;
*/
void dolabel(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	tree_node* expr;
	float slot;
again:
	switch (li->next_token.token)
	{
	case CASE:
		match(li, ctx, CASE);
		slot = (*pp)->nextslot;
		expr = expression(li, ctx);
		match(li, ctx, DO);
		addtag(ctx, si, LCASE, slot, expr);
		goto again;
	case DEFAULT:
		match(li, ctx, DEFAULT);
		slot = (*pp)->nextslot;
		match(li, ctx, DO);
		addtag(ctx, si, LDEFAULT, slot, NULL);
		goto again;
	default:
		dostatement(li, ctx, pp, si);
		return;
	}
}

/*
	FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
*/
void dofor(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	tree_node *t = NULL;
	qoperand *q;
	int slot1, slot2;
	match(li, ctx, FOR);
	//match(li, ctx, '(');
	if (li->next_token.token != WHILE)	/* 1 */
		genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	match(li, ctx, WHILE);
	slot1 = (*pp)->nextslot;
	if (li->next_token.token != STEP)	/* 2 */
		q = genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	slot2 = (*pp)->nextslot;
	if (slot1 != slot2)	/* 2 is not empty */
		qbuild(ctx, pp, opJz, q, 0, 0);
	match(li, ctx, STEP);
	if (li->next_token.token != LOOP)	/* 3 */
		t = expression(li, ctx);
	match(li, ctx, LOOP);
	ctx->looplevel++;
	dostatement(li, ctx, pp, si);
	ctx->looplevel--;
        genexpr(ctx, t, pp, NULL, 1);
	qbuildbranch(ctx, pp, opJmp, 0, slot1);
	if (slot1 != slot2)	/* 2 is not empty */
		patch(ctx, *pp, slot2, (*pp)->nextslot);
	patchtransfers(ctx, pp, si, (*pp)->nextslot, slot1, NULL);
	clearlabel(si);
}

/*
	FOREACH "(" idlist _IN exprlist ")" statement
	Implementation:
	The FOREACH statement generates the follozen quadruples:
	Start: set vm->slot and vm->node to 0;
	Save: save the values of slot and node to temporary variables;
	Restore: restore the values of slot and node from temporary variables;
	Update: proceed to the next element in the array;
	Copy: copy key/value pair to corresponding variables;
	Save;
	Jump out if reach the end;
	the statement code here...
	Jump to Restore again.
*/
void doforeach(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	tree_node *t, *t1, *t2;
	int slot1, slot2;
	qoperand* o = newtemporary(ctx), *o1, *o2=0;
	qoperand *oslot, *onode;
	qbuild(ctx, pp, opStart, 0, 0, 0);
	clStep(li);
	match(li, ctx, FOREACH);
	t = primary(li, ctx);
	if (li->next_token.token == ',')		/* is it key/value? */
	{
		match(li, ctx, ',');
		t1 = primary(li, ctx);
	}
	else if (li->next_token.token == _IN)	/* no, just value */
		t1 = t;
	else setlasterror(findcontextroot(ctx), ZEN_PARSER_SYNTAX_ERROR,
		"%s: syntax error, line %d, pos %d\n", ctx->source, li->line_num, li->pos - strlen(li->token.dbuf));
	match(li, ctx, _IN);
	t2 = exprlist(li, ctx);
	match(li, ctx, LOOP);
	newtemporary(ctx);	/* skip some temporary slot, messy:( */
	oslot = newtemporary(ctx);
	onode = newtemporary(ctx);
	qbuild(ctx, pp, opSave, oslot, onode, 0);
	slot1 = (*pp)->nextslot;
	qbuild(ctx, pp, opRestore, oslot, onode, 0);
	qbuild(ctx, pp, opUpdate, genexpr(ctx, t2, pp, 0, 1), 0, o);
	o1 = genexpr(ctx, t, pp, 0, 0);		/* do not free t but free t1*/
	if (t==t1&&validatecontext(ctx)||t!=t1)
		o2 = genexpr(ctx, t1, pp, 0, 1);
	if (t!=t1) free(t);
	qbuild(ctx, pp, opCopy, o1, o2, 0);
	qbuild(ctx, pp, opSave, oslot, onode, 0);
	slot2 = (*pp)->nextslot;
	qbuildbranch(ctx, pp, opJz, o, 0);
	ctx->looplevel++;
	dostatement(li, ctx, pp, si);
	ctx->looplevel--;
	qbuildbranch(ctx, pp, opJmp, 0, slot1);
	patch(ctx, *pp, slot2, (*pp)->nextslot);
	patchtransfers(ctx, pp, si, (*pp)->nextslot, slot1, NULL);
	clearlabel(si);
}

/* WHILE "(" expr ")" statement */
void dowhile(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	qoperand* o;
	int slot1, slot2;
	clStep(li);
	//match(li, ctx, '(');
	slot1 = (*pp)->nextslot;
	o = genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	slot2 = (*pp)->nextslot;
	qbuild(ctx, pp, opJz, o, NULL, NULL);
	//match(li, ctx, ')');
	match(li, ctx, LOOP);
	ctx->looplevel++;
	dostatement(li, ctx, pp, si);
	ctx->looplevel--;
	qbuildbranch(ctx, pp, opJmp, 0, slot1);
	patch(ctx, *pp, slot2, (*pp)->nextslot);
	patchtransfers(ctx, pp, si, (*pp)->nextslot, slot1, NULL);
	clearlabel(si);
}

/* DO statement WHILE "(" expr ")" */
void doloop(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	qoperand* o;
	int slot;
	clStep(li);
	slot = (*pp)->nextslot;
	ctx->looplevel++;
	dostatement(li, ctx, pp, si);
	ctx->looplevel--;
	match(li, ctx, WHILE);
	o = genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	qbuildbranch(ctx, pp, opJnz, o, slot);
	patchtransfers(ctx, pp, si, (*pp)->nextslot, slot, NULL);
	clearlabel(si);
}

/* VAR id=assign expression, ... */
void dovar(LexInfo* li, context* pctx, intermediate** pp)
{
	tree_node* t;
	symbol* s;
	qoperand *o1, *o2;
	match(li, pctx, VAR);
	while (1)
	{
		if (lookup(pctx, &pctx->sbt, li->next_token.dbuf))
			setlasterror(pctx, ZEN_PARSER_REDEFINED,
			"Identifier redefinition in line %d, pos %d.\n ", li->line_num, li->pos);
		insertid(pctx, &pctx->sbt, li->next_token.dbuf);
		s = lookupall(li, pctx, li->next_token.dbuf);
		t = makeleaf(pctx, li->next_token);
		if (s) sym2node(s, t);
		t->iscall = 0;
		o1 = genexpr(pctx, t, pp, NULL, 1);
		clStep(li);
		if (li->next_token.token == '=')
		{
			clStep(li);
			o2 = genexpr(pctx, assign(li, pctx), pp, NULL, 1);
			qbuild(pctx, pp, opAssign, o2, NULL, o1);
		}
		if (li->next_token.token == "\n")
			break;
        //must add that it ends in the end of the file too
		clStep(li);
	}
	match(li, pctx, ';');
}

void doenum(LexInfo* li, context* pctx, intermediate** pp)
{
	tree_node* t;
	symbol* s;
	qoperand *o1, *o2;
	match(li, pctx, ENUM);
	match(li, pctx, '{');
	while (1)
	{
		if (lookup(pctx, &pctx->sbt, li->next_token.dbuf))
			setlasterror(pctx, ZEN_PARSER_REDEFINED,
			"Identifier redefinition in line %d, pos %d.\n ", li->line_num, li->pos);
		insertid(pctx, &pctx->sbt, li->next_token.dbuf);
		s = lookupall(li, pctx, li->next_token.dbuf);
		t = makeleaf(pctx, li->next_token);
		if (s) sym2node(s, t);
		t->iscall = 0;
		o1 = genexpr(pctx, t, pp, NULL, 1);
		clStep(li);
		if (li->next_token.token == '}')
		{
			clStep(li);
			o2 = genexpr(pctx, assign(li, pctx), pp, NULL, 1);
			qbuild(pctx, pp, opAssign, o2, NULL, o1);
		}
		if (li->next_token.token == '}')
			break;
        //must add that it ends in the end of the file too
		clStep(li);
	}
	match(li, pctx, '}');
}

/*
	":" block "end"
	Returns the local memory size of the compound.
	If isfunction is 1, this is a function body compound statement;
	otherwise it is a local compound statement.
	pls: parent compound statement local size.
	pts: parent compound statement temporary size.
*/
void docompound(LexInfo* li, context* pctx, intermediate** pp, symboltable* sbtp, int isfuncbody, tree_node* fn, statementinfo* si, int pls, int pts)
{
	int maxlocal=0, maxtemp=0;
	context ctx = {0};
	intermediate* p;
	statementinfo lsi = {0};
	ctx.parent = pctx;
	ctx.sbtp = sbtp;
	ctx.sbt.nextslot = ctx.sbt.offset = pls;
	ctx.temp = pts;
	ctx.compoundlevel = pctx->compoundlevel+1;
	ctx.looplevel = pctx->looplevel;
	/* Starting a new function. */
	if (isfuncbody)
	{
		if ((p=(intermediate*)calloc(1,sizeof(intermediate))) == NULL)
		{
			setlasterror(pctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
			return;
		}
		p->size = 1024;
		if ((p->quadruples=(quadruple*)calloc(p->size, sizeof(quadruple))) == NULL)
		{
			setlasterror(pctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
			return;
		}
		ctx.funcname = fn;
	}
	else
		p = *pp;
	match(li, &ctx, BEGIN);
	if (li->next_token.token == END)	/* empty compound statement */
		clStep(li);
	else
	{
		while (li->next_token.token != ENDOFFILE && li->next_token.token != END && validatecontext(&ctx))
		{
			memset(&lsi, 0, sizeof(lsi));
			dostatement(li, &ctx, &p, &lsi);
			mergetags(&ctx, si, &lsi);
		}
		match(li, &ctx, END);
	}
	if (isfuncbody)
	{
		*pp = p;
		qbuild(pctx, &p, opRet, 0, 0, getlastoperand(*pp));
	}
	else
		pctx->prog = p;
	si->tempsize = ctx.maxtemp;
	si->localsize = ctx.maxlocal;
}

void doexpression(LexInfo* li, context* ctx, intermediate** pp)
{
	genexpr(ctx, expression(li, ctx), pp, NULL, 1);
	//match(li, ctx, ';');
}

/* Adds a new try record to a function represented by s. */
void addtry(symbol* s, uint first, uint last, uint entry)
{
	if (s->ta == 0)
	{
		s->ta = calloc(1, sizeof(tryarr));	/* validity here */
		s->ta->size = 10;
		s->ta->tr = calloc(s->ta->size, sizeof(tryrec));
	}
	s->ta->tr[s->ta->nextslot].first = first;
	s->ta->tr[s->ta->nextslot].last = last;
	s->ta->tr[s->ta->nextslot].catchentry = entry;
	s->ta->nextslot++;
}

/* try statement catch (id) statement */
void dotry(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	uint first=(*pp)->nextslot, last, entry, slot;
	symbol* s = lookupall(li, ctx, findname(ctx));
	qoperand* o;
	match(li, ctx, TRY);
	dostatement(li, ctx, pp, si);
	last = (*pp)->nextslot-1;
	slot = (*pp)->nextslot;
	qbuild(ctx, pp, opJmp, 0, 0, 0);
	match(li, ctx, CATCH);
	match(li, ctx, '(');
	o = genexpr(ctx, expression(li, ctx), pp, 0, 1);
	match(li, ctx, ')');
	entry = (*pp)->nextslot;
	qbuild(ctx, pp, opSaveexc, 0, 0, o);
	dostatement(li, ctx, pp, si);
	patch(ctx, *pp, slot, (*pp)->nextslot);
	addtry(s, first, last, entry);
}

/* throw expression */
void dothrow(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	qoperand* o;
	match(li, ctx, THROW);
	o = genexpr(ctx, expression(li, ctx), pp, 0, 1);
	//match(li, ctx, ';');
	qbuild(ctx, pp, opThrow, 0, 0, o);
}

/* include statement */
void doinc(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	char *buf, path[ZEN_MAX_PATH_LEN*2]={0};
	clStep(li);
	getdir(li->source, path);
	String_Concat(path, li->next_token.dbuf);
	if (!(buf=readfile(path)))
	{
		setlasterror(ctx, ZEN_PARSER_INCLUDE_NOT_FOUND, "Compile error: include file  %s  not found, line %d, pos %d.\n",path, li->line_num, li->pos - strlen(li->token.dbuf));
	    return;
	}
	parse(path, buf, ctx);
	clStep(li);
	free(buf);
}

/* import statement */
void doimport(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	char *path = directory_getcurrent();
	char *buf;
    clStep(li);
	strcat(path, "/lib/");
	strcat(path, li->next_token.dbuf);
	if (strcmp(path, ".zen") != 0)
	{
		strcat(path, ".zen");
	}

	if (!(buf=readfile(path)))
	{
	    setlasterror(ctx, ZEN_PARSER_INCLUDE_NOT_FOUND, "Compiler error: include file  %s  not found, line %d, pos %d.\n", path, li->line_num, li->pos - strlen(li->token.dbuf));
	    free(buf);
	    return;
	}
	parse(path, buf, ctx);
	clStep(li);
	printf("done");
	free(buf);
}

/* import statement */
void dofrom(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
    match(li, ctx, FROM);
	char *path = directory_getcurrent();
	char *buf;
    clStep(li);
	strcat(path, "/lib/");
	strcat(path, li->next_token.dbuf);
	if (strcmp(path, ".zen") != 0)
	{
		strcat(path, ".zen");
	}

	if (!(buf=readfile(path)))
	{
	    // file not found.
	    setlasterror(ctx, ZEN_PARSER_INCLUDE_NOT_FOUND, "Compiler error: include file  %s  not found, line %d, pos %d.\n", path, li->line_num, li->pos - strlen(li->token.dbuf));
	    free(buf);
	    return;
	}
	// file exist. inspite of parsing the whole file, we parse only the requested ID.
	match(li, ctx, IMPORT);
	parse(path, buf, ctx);
	clStep(li);
	printf("done");
	free(buf);
}

void dostatement(LexInfo* li, context* ctx, intermediate** pp, statementinfo* si)
{
	context* croot = findcontextfunction(ctx);
	switch (li->next_token.token)
	{
	case ';': clStep(li); break;
	case VAR: dovar(li, ctx, pp); break;
	case ENUM: doenum(li, ctx, pp); break;
	//case CLASS: break;
	case GLOBAL: doexpression(li, ctx, pp); break;
	case BEGIN: docompound(li, ctx, pp, ctx->sbtp, 0, NULL, si, ctx->sbt.offset, ctx->temp); break;
	case WHILE: dowhile(li, ctx, pp, si); break;
	case LOOP: doloop(li, ctx, pp, si); break;
	case FOR: dofor(li, ctx, pp, si); break;
	case FOREACH: doforeach(li, ctx, pp, si); break;
	case IF: doif(li, ctx, pp, si); break;
	case SWITCH: doswitch(li, ctx, pp, si); break;
	case CASE:	case DEFAULT: dolabel(li, ctx, pp, si); break;
	case RETURN: doreturn(li, ctx, pp); break;
	case YIELD: doyield(li, ctx, pp); break;
	case BREAK: dobreak(li, ctx, pp, si); break;
	case CONTINUE: docontinue(li, ctx, pp, si); break;
	case DEF: dofunction(li, ctx, pp); break;
	case POINTCUT: dopointcut(li, ctx); break;
	case ADVICE: doadvice(li); break;
	case TRY: dotry(li, ctx, pp, si); break;
	case THROW: dothrow(li, ctx, pp, si); break;
	case INCLUDE: doinc(li, ctx, pp, si); break;
	case IMPORT: doimport(li, ctx, pp, si); break;
	case FROM: dofrom(li, ctx, pp, si); break;
	default: doexpression(li, ctx, pp); break;
	}
	croot->maxlocal = MAX(croot->maxlocal, ctx->sbt.offset);
	croot->maxtemp = MAX(croot->maxtemp, ctx->temp);
	ctx->temp = 0;
}

/* Assemble the code to get the final quadruple program. ctx is the root context. */
void assemble(context* ctx)
{
	int i,j,lastquadg;
	intermediate* p = ctx->prog;
	if (!validatecontext(ctx)) return;
	qbuild(ctx, &p, opHalt, 0, 0, 0);
	lastquadg = ctx->prog->nextslot-1;
	for (i=0; i<ctx->sbt.nextslot; i++)
	{
		if (ctx->sbt.table[i].isfunction && (p=ctx->sbt.table[i].prog)!=NULL)
		{
			ctx->sbt.table[i].firstquad = ctx->prog->nextslot;
			for (j=0; j<p->nextslot; j++)
			{
				if (isbranch(&p->quadruples[j]))
					p->quadruples[j].opdest.entity.entity.ival += ctx->sbt.table[i].firstquad;
				qbuild(ctx, &ctx->prog, p->quadruples[j].op, &p->quadruples[j].op1, &p->quadruples[j].op2, &p->quadruples[j].opdest);
			}
			ctx->sbt.table[i].lastquad = ctx->prog->nextslot-1;
		}
	}
	insertfunc(ctx, &ctx->sbt, GLOBAL_NAME, ctx->prog, 0, lastquadg, ctx->maxlocal, ctx->maxtemp, 0, 0);
}

void doblock(LexInfo* li, context* ctx, intermediate** pp)
{
	statementinfo si = {0};
	/* Do statements until end of file or something bad happened. */
	while (li->next_token.token != ENDOFFILE)
		dostatement(li, ctx, pp, &si);
}

/* Initialize the context. */
void initcontext(context* ctx)
{
	if ((ctx->prog=(intermediate*)calloc(1, sizeof(intermediate))) == NULL)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	ctx->calls.size = 1024;
	if ((ctx->calls.calls=(callrec*)calloc(ctx->calls.size, sizeof(callrec))) == NULL)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	ctx->calls.nextslot = 0;
	if ((ctx->funcname=(tree_node*)calloc(1, sizeof(tree_node))) == NULL)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	ctx->funcname->token.dbuf = ctx->funcname->token.buf;
	strcpy(ctx->funcname->token.dbuf, GLOBAL_NAME);
	ctx->funcs.size = 1024;
	if ((ctx->funcs.funcs=(funcrec*)calloc(ctx->funcs.size, sizeof(funcrec))) == NULL)
		setlasterror(ctx, ZEN_OUT_OF_MEMORY, "Out of memory.\n");
	ctx->funcs.nextslot = 0;
	ctx->temp = 0;
}

void deinitcontext(context* ctx)
{
	int i;
	if (!ctx) return;
	if (ctx->prog && ctx->prog->quadruples) free(ctx->prog->quadruples);
	if (ctx->prog) free(ctx->prog);
	for (i=0; i<ctx->sbt.nextslot; i++)
		if (ctx->sbt.table[i].isfunction && ctx->sbt.table[i].prog)
		{
			/* The global context holds the program, must not delete it.*/
			if (strcmp(ctx->sbt.table[i].name, GLOBAL_NAME)!=0)
			{
				free(ctx->sbt.table[i].prog->quadruples);
				free(ctx->sbt.table[i].prog);
			}
			if (ctx->sbt.table[i].ta)
			{
				free(ctx->sbt.table[i].ta->tr);
				free(ctx->sbt.table[i].ta);
			}
		}
	free(ctx->calls.calls);
	free(ctx->funcs.funcs);
	if (ctx->funcname) freenode(ctx->funcname);
	freeoperands(ctx);
}

/* Detect all the tail recursion calls. */
void detectTRC(LexInfo* li, context* ctx)
{
	int i;
	symbol* s;
	for (i=0; i<ctx->prog->nextslot; i++)
		if (ctx->prog->quadruples[i].op == opCall)
		{
			s = getquadfunc(ctx, i);
			if (ctx->prog->quadruples[i+2].op == opRet &&
			s->firstquad == ctx->prog->quadruples[i].opdest.entity.entity.ival)
			ctx->prog->quadruples[i].op = opTRCall;
		}
}

/* Do the parse job. */
int parse(const char* source, char* buf, context* ctx)
{
	LexInfo li = {0};
	if (!clLexOpen(&li, source, buf)) return 0;
	strcpy(ctx->source, source);
	doblock(&li, ctx, &ctx->prog);
	clLexClose(&li);
	return 1;
}

/* Parse the topmost file. */
int parseroot(const char* source, char* buf, context* ctx)
{
	initcontext(ctx);
	insertfunc(ctx, &ctx->sbt, GLOBAL_NAME, 0, 0, 0, ctx->maxlocal, ctx->maxtemp, 0, 0);
	strcpy(ctx->source, source);
	parse(source, buf, ctx);
	assemble(ctx);
	patchcalls(0, ctx);
	detectTRC(0, ctx);
	return 1;
}
