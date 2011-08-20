/* See Copyright Notice in Zen.h. */

#ifndef _APARSE_H_
#define _APARSE_H_

#include "alex.h"
#include "avm.h"

/* Data scopes. */
enum
{
	SUNDEFINED=0, SGLOBAL, SLOCAL, SPARAMETER, STEMP, SCONSTANT, SFUNCTION,
	SHFUNCTION
};

/* Id types. */
enum { IDIDENTIFIER=0, IDFUNCTION };

/* Breaks and labels. */
enum {TBREAK=0, TCONTINUE, LCASE, LDEFAULT};

#define GLOBAL_NAME "_g_"

#define MAX(a,b) ((a)>(b)?(a):(b))

#define isbranch(t) ((((t)->op>=opJz&&(t)->op<=opJmp)||(t)->op==opCall||(t)->op==opTRCall)?1:0)
#define isbbranch(t) ((((t)->op>=opJz&&(t)->op<=opJmp)||((t)->op>=opCall&&(t)->op<=opRet))?1:0)
#define settype(o, type) { (o)->entity.tag = type; }
#define gettype(o) ((o)->entity.tag)

typedef struct _tree_node
{
	char isleaf;	/* is this a leaf node? */
	_token token;	/* identifier or constants for leaf nodes */
	char type;		/* an identifier or a function? */
	char scope;		/* is leaf a local, a parameter, a temporary, or a constant? */
	int offset;
	char iscall;	/* a function call or just referring to its address? */
	int op;			/* operator for non-leaf nodes */
	/* sibling for tertiary operators' 3rd operand, also used for arguments */
	struct _tree_node *parent, *left, *right, *sibling, *next, *prev;
	/* extra increment/decrement expression node lists */
	struct _tree_node *postinc, *postdec;
	long line_num;	/* the line where this node is constructed */
	int index;		/* index in a list */
} tree_node;

typedef struct
{
	uint first, last, catchentry;		/* catch block entry instruction */
} tryrec;

typedef struct
{
	tryrec* tr;
	int size, nextslot;				/* # try records and current position */
} tryarr;

struct _intermediate;
struct _symboltable;
typedef struct _symbol
{
	char name[ZEN_MAX_TOKEN_LEN];
	char isfunction;				/* a function or an identifier? */
	char ishost;					/* is the function a host? */
	struct _symboltable* params;	/* parameter info for a function */
	char numnamedparams;			/* # of named parameters */
	char paramsvariable;			/* # of named parameters variable? */
	int compoundlevel;
	char scope;
	int offset;
	struct _symbol *next;			/* used for a list */
	struct _intermediate* prog;		/* a function's code in quadruples */
	int firstquad, lastquad;		/* first and last quadruples (function) */
	int localsize, tempsize;		/* local and temporary variable sizes */
	tryarr* ta;						/* try array, for exception handling */
} symbol;

typedef struct _symboltable
{
	symbol table[128];	/* replace 1024 */
	int offset;			/* offset of next variable */
	int nextslot;		/* next available slot */
} symboltable;

/* structure for backpatching labels and "break" and "continue"statements */
typedef struct _ttag
{
	char type;
	long slot;
	tree_node* expr;	/* expression associated with case lables */
	struct ttag* next;	/* for chaining */
} ttag_;

typedef struct _statementinfo	/* info of a statement */
{
	int localsize, tempsize;	/* local and temporary variable sizes */
	ttag_ tags[64];				/* label array */
	int nextslot;
} statementinfo;

typedef struct _callrec
{
	long slot;
	char caller[ZEN_MAX_TOKEN_LEN], callee[ZEN_MAX_TOKEN_LEN];
} callrec;

typedef struct _callarr
{
	callrec* calls;
	long size, nextslot;
} callarr;

typedef struct _funcrec
{
	long slot, operand;
	char caller[ZEN_MAX_TOKEN_LEN], callee[ZEN_MAX_TOKEN_LEN];
} funcrec;

typedef struct _funcarr
{
	funcrec* funcs;
	long size, nextslot;
} funcarr;

typedef enum
{
	LEFT, RIGHT
} eAssociativity;

#define NUMOP(ops) (sizeof(ops)/sizeof(_operator))	/* get array size */
typedef struct
{
	int op;
	int precedence;
	eAssociativity associativity;
} _operator;

static const _operator unaoperators[] =
{
	{'+', 14, RIGHT}, {'-', 14, RIGHT},
	{INC, 14, LEFT}, {PREINC, 14, RIGHT}, {POSTINC, 14, LEFT},
	{DEC, 14, LEFT}, {PREDEC, 14, RIGHT}, {POSTDEC, 14, LEFT},
	{'!', 14, RIGHT}, {'~', 14, RIGHT}, {FACT, 13, LEFT},
	{TCONSTRUCT, 15, LEFT},
};

static const _operator binoperators[] =
{
	{LOGOR, 4, LEFT},
	{LOGAND, 5, LEFT},
	{'|', 6, LEFT},
	{'^', 7, LEFT},
	{'&', 8, LEFT},
	{EQUAL, 9, LEFT}, {NOTEQUAL, 9, LEFT},
	{TEQUAL, 9, LEFT}, {TNOTEQUAL, 9, LEFT},
	{'<', 10, LEFT}, {LEQ, 10, LEFT}, {'>', 10, LEFT}, {GEQ, 10, LEFT},
	{LSHIFT, 11, LEFT}, {RSHIFT, 11, LEFT},
	{'+', 12, LEFT}, {'-', 12, LEFT}, {DOUBLEDOT, 12, LEFT},
	{'*', 13, LEFT}, {POWER, 14, LEFT}, {'/', 13, LEFT}, {'%', 13, LEFT},
	{CLASS, 16, LEFT},
};

/* Intermediate code representations. */
typedef struct _qoperand
{
	char scope;
	word entity;
	struct _qoperand* next;
} qoperand;

typedef struct
{
	eOpcode op;
	qoperand op1, op2, opdest;
	uint firstinstruction, lastinstruction;	/* first instruction's index of the quad*/
	char isleader;		/* is this quadruple a leader? for data flow analysis */
} quadruple;

typedef struct _intermediate
{
	quadruple* quadruples;
	long size, nextslot;
} intermediate;

/* the context information when parsing a compound statement */
typedef struct _context
{
	struct _context* parent;	/* enclosing block(parent) information */
	char source[ZEN_MAX_PATH_LEN];
	symboltable sbt, *sbtp;		/* local and parameter symbol tables */
	intermediate* prog;			/* global */
	callarr calls;				/* global */
	funcarr funcs;				/* functional variable array */
	qoperand *qophead, *qcurop;	/* qoperands to free */
	tree_node* funcname;/* function name of block, GLOBAL_NAME if global */
	int compoundlevel;	/* nested compound level */
	int looplevel;		/* nested loop level */
	int maxlocal;		/* total local size for the outermost level (==0) */
	int maxtemp;		/* # of temporary variables used */
	int temp;			/* # of temporary variables currently used */
	char inleader;		/* translating a leader quadruple? */
	char insord;		/* order of the instruction in the quadruple */
	char lasterror;		/* last error, global, for memory or file problems */
} context;

typedef struct _Azure_entity
{
	avm vm;
	context ctx;
	assembly assm;
} Azure_entity;

int parse(const char* source, char* buf, context* ctx);
int parseroot(const char* source, char* buf, context* ctx);
symbol* lookup(context* ctx, symboltable* sbt, const char* name);
int validatecontext(context* ctx);
void setlasterror(context* ctx, int errorno, const char* msg, ...);
symbol* lookupall(LexInfo* li, context* ctx, const char* name);
void deinitcontext(context* ctx);

#endif
