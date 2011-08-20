/* See Copyright Notice in azure.h. */

#ifndef _AVM_H_
#define _AVM_H_

#include <stdio.h>
#include "zen.h"

/* Regs.  1  2   3   4   5   6   7   8 */
enum { ra=1, rb, rc, re, rd, rs, rf, NUM_REGS};

/* Data types. */
enum
{	/* 0    1         2       3          4           5        6      7 */
	TNIL=0, TINTEGER, TFLOAT, TFUNCTION, THFUNCTION, TSTRING, TNODE, TTABLE,
	TSTACK, TTHREAD, THANDLE, TBLOB, TDIR
};  /* 8    9        10       11     12*/

/* Set the type of a word. */
#define settypew(w, type) (w)->tag = type
/* Get the type of a word. */
#define gettypew(w) ((w)->tag)
/* Push a word in register r onto stack s, in virtual machine vm. */
#define pushr(vm, s, r) push((vm), s, &(vm)->regs[r])
/* Pop a word from stack s and store it into register r, in virtual machine vm. */
#define popr(vm, s, r) vm->regs[r] = *(pop(vm, s))
#define ZEN_BLOCKHEADERSIZE ((long)sizeof(mblock)+(long)sizeof(objheader))
#define ZEN_TABLESIZE(num) ((long)(num*sizeof(cnode)+sizeof(long)*2))
#define ZEN_INITIALTOTSIZE 100

/* Word type/value setting macros. */
#define seti(w,vi) {(w)->entity.ival=vi;settypew(w,TINTEGER);}	/* integer */
#define setf(w,vf) {(w)->entity.fval=vf;settypew(w,TFLOAT);}	/* float */
#define sets(w,os) {(w)->entity.offset=os;settypew(w,TSTRING);}	/* string */
#define sett(w,os) {(w)->entity.offset=os;settypew(w,TTABLE);}	/* table */
#define seth(w,hv) {(w)->entity.offset=hv;settypew(w,THANDLE);}	/* handle */
#define setd(w,hv) {(w)->entity.offset=hv;settypew(w,TDIR);}	/* handle */
#define setb(w,bv) {(w)->entity.offset=bv;settypew(w,TBLOB);}	/* BLOB */

#define setnil(w) { (w)->tag=0; (w)->entity.ival=0; }
#define isnil(w) (gettypew(w)==TNIL)

#define ishostfunction(name) (zen_getfunc(name)!=0)

/* return the object offset given the block */
#define getobj(heap, offset) (heap+offset+(long)sizeof(mblock))
/* return the data offset given the block */
#define getdata(heap, offset) (heap+offset+ZEN_BLOCKHEADERSIZE)

#define isobject(w) (gettypew(w)>=TSTRING)
#define istable(w) (gettypew(w)==TTABLE)
#define isthread(w) (gettypew(w)==TTHREAD)
#define isstack(w) (gettypew(w)==TSTACK)

/* Get the nth argument from vm. Starting from 0, left to right. */
#define getarg(vm,n) (&(vm)->cs->sbody[(vm)->regs[rf].entity.ival-(n)-2])
/* Return a value to the script engine. */
#define returnv(vm,w) ((vm)->cs->sbody[(vm)->regs[rf].entity.ival+1]=*(w))

/* The opcodes are shared by quadruples and instructions. */
typedef enum
{
	/* RR instructions. */
	opAssign, opLogand, opLogor, opLshift, opBitcom, opBitand, opBitor,
	opBitxor, opRshift, opXor, opNeg, opNot, opAdd, opMinus, opMultiply,
	opPower, opFact, opDivide, opMod, opEqual, opNeq, opTEqual, opTNeq,
	opGreater, opGeq, opLess, opLeq, opClass,
	opConcat, opPush, opPop, opCall, opTRCall, opCallhost, opCallin, opRet,
	opYield, opSaveexc, opThrow, opStart, opUpdate, opCopy, opSave, opRestore,
	/* Heap instructions. */
	opNewstring, opNewtable,
	/* RM instructions. */
	opLD, opST, opLDA, opSTA,
	/* RI instructions. */
	opIR, opIA,
	/* RA instructions. */
	opLCI, opLCF, opLNIL, opLFUNC, opLHF, opJz, opJnz, opJmp, opHalt
} eOpcode;

/* A single word holding different data types. */
typedef struct _word
{
	char tag;			/* tag contains type information */
	union
	{
		int ival;		/* integer constant or jump address */
		float fval;		/* float constant */
		char* sval;		/* string constant */
		int offset;		/* object offset, see operand structure below */
	} entity;
} word;

typedef union
{
    char scope;
    long ival;		/* integer constant or jump address */
    float fval;		/* float constant */
    char* sval;		/* string constant */
    int offset;		/* object offset, see operand structure below */
} operand;

typedef struct
{
	eOpcode op;
	operand op1, op2, opdest;
} instruction;

typedef struct _assembly
{
	instruction* instructions;
	long size, nextslot, entry;	/* entry is the first instruction to execute */
	long stacksize;
} assembly;

typedef struct _stack
{
	word* sbody;	/* stack body */
	long soff;		/* stack offset in the heap */
	long size;		/* stack size */
	long sp,fp;		/* stack pointer and frame pointer */
} stack;

typedef struct _mblock
{
	long boff, eoff;	/* begin and end offsets */
} mblock;

typedef struct _objheader
{
	char type;
	long offset;	/* offset in the heap */
	long size;		/* size of data */
	long refcount;	/* reference counter */
	char marked;	/* mark bit used for garbage collection */
} objheader;

typedef struct _mbtable	/* for managing memory blocks in the heap */
{
	mblock *mbt;
	int size;
	int nextslot;
} mbtable;

typedef struct _heap
{
	char* heap;				/* heap body */
	long size, availsize;	/* total size and available size */
	mbtable avtable;		/* available block table */
	mbtable altable;		/* allocated block table */
} heap;

typedef struct
{
	long offset;
	char marked;
} pair;

typedef struct _TOT		/* table of table */
{
	pair* table;
	uint size;
} TOT;

typedef struct _thread
{
	word regs[NUM_REGS];		/* for thread context saving */
	word ofp;					/* original fp */
	long pc;					/* for thread context saving */
	long entry;					/* entry instruction */
	stack* s;					/* thread-owned stack */
	char dead;					/* is the thread dead? */
} thread;

typedef struct _threadstack
{
	thread* threads[64];
	long nextslot;
} threadstack;

typedef struct _extobj	/* external object (allocated in the host) */
{
	word w;
	struct _extobj* next;
} extobj;

struct _context;
struct _symbol;
struct _avm;

typedef struct _avm
{
	struct _context* ctx;
	word regs[NUM_REGS];		/* general registers */
	word ofp;					/* original fp */
	instruction* instructions;	/* code segment */
	long numins;				/* total # of instructions */
	stack *hs, *cs;				/* head of stack list, current active stack */
	heap hp;					/* the only heap in vm */
	long pc;					/* program counter */
	long globalsize;			/* size of global variables */
	TOT tot;					/* "table of table" */
	threadstack ts;				/* thread stack for saving thread contexts */
	thread* ct;					/* current thread, redundant with thoff~~ */
	extobj* extobjs;			/* external object list */
	long thoff;					/* thread offset in the heap */
	char gcapplied;				/* garbage collection applied */
	char expanded;				/* is the heap expanded? */
	char* oldheap;				/* old heap pointer */
	char loaded;				/* any program loaded on this vm? */
	char execfunc;				/* executing only one function? */
	userfunc traphook;			/* callback function of traps */
	long slot, node;
	word wkey, wval;
	char lasterror;				/* error status for inspection */
	char errorreported;			/* prevent multiple run-time reports */
} avm;

typedef struct _cnode	/* each node is an element in the array */
{
	word key;		/* key's offset */
	word value;		/* value corresponding to the key */
	long nextnode;	/* heap offset of the next node, for chaining */
} cnode;

typedef struct _ctable	/* for arrays and hash tables */
{
	long tablesize, nextslot;
	cnode* nodes;	/* the array, variable length */
} ctable;

void setvmerror(struct _avm* vm, int errorno);
void load(struct _context* ctx, avm* vm, assembly* p);
void run(struct _context* ctx, avm* vm);
int step(struct _context* ctx, avm* vm);
long newstring(avm* vm, char* s);
struct _symbol* getinsfunc(struct _context* ctx, uint ins);
void setglobali(avm* vm, const char* idname, int ival);
void setglobalf(avm* vm, const char* idname, float fval);
void setglobals(avm* vm, const char* idname, const char* sval);
int getint(avm* vm, int ind);
float getfloat(avm* vm, int ind);
char* getstring(avm* vm, int ind);
FILE* gethandle(avm* vm, int ind);
word* getglobal(avm* vm, const char* idname);
int getglobali(avm* vm, const char* idname);
float getglobalf(avm* vm, const char* idname);
char* getglobals(avm* vm, const char* idname);
void pushi(avm* vm, int ival);
void pushf(avm* vm, float fval);
void pushs(avm* vm, const char* str);
void callf(avm* vm, const char* func);
word* getret(avm* vm, int n);
void verifytype(avm* vm, word* w, int type);
void settraphook(avm* vm, userfunc hook);
void printw(avm* vm, word* w);
void releasevm(avm* vm);

#endif
