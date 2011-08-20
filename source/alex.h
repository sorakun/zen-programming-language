/* See Copyright Notice in Zen.h. */

#ifndef _ALEX_H_
#define _ALEX_H_

#include "zen.h"
#include "aregex.h"

#define ZEN_MAX_PATH_LEN 10240

typedef enum
{
	/* Reserved words. */
	ADVICE=256, BREAK, CASE, CATCH, CONTINUE, DEF, DEFAULT, DO, ELSE, FOR,
	FOREACH, IF, _IN, INCLUDE, NIL, POINTCUT, REQUIRE, IMPORT, RETURN, SWITCH, THROW,
	TRY, VAR, WHILE, YIELD, END, BEGIN, THEN, LOOP, CELL, FACT, SQUAREROOT,
    WITH, STEP, CLASS, TEQUAL /* Type equality */, TNOTEQUAL, ENUM, FROM,
    GLOBAL, DEFINE, IFDEF, ELIFDEF, ENDIF,
	/* Others. */
	IDENTIFIER, INTEGER, FLOATING, LITERAL, COMMENT, SPACE, DOUBLEDOT,
	TRIPLEDOT, INC, DEC, PREINC, POSTINC, PREDEC, POSTDEC, PASS, MASS, MULASS,
	DASS, MODASS, LSASS, RSASS, BAASS, BOASS, BXASS, LEQ, GEQ, EQUAL, NOTEQUAL,
	LOGAND, LOGOR, TCONSTRUCT, LSHIFT, RSHIFT, ENDOFFILE, UNBOUNDEDCOMMENT, POWER,
	UNBOUNDEDLITERAL, SCOPE, FUNC, INDEX, UNDEFINED, UKERROR
} eToken;

typedef struct
{
	int token;
	char buf[ZEN_MAX_TOKEN_LEN], *dbuf;
	uint dbuflen;
	uint seencr;	/* # of CR before this token? */
} _token;

typedef struct
{
	int token;
	char* str;
	char* pattern;
} token_name;

/* Tokens, scanned by greedy matching. Change of ordering may induce bugs. */
static const token_name tokens[] =
{
	{IDENTIFIER, "", "(_|\\$|\\a)(_|\\a|\\d)*"},	/* must precede keywords */
	/* keywords */
	{ADVICE, "advice", ""},
	{BREAK, "break", ""},
	{CASE, "case", ""},
	{CATCH, "catch", ""},
	{CONTINUE, "continue", ""},
	{DEF, "func", ""},
	{DEFAULT, "default", ""},
	{DO, "do", ""},
	{ELSE, "else", ""},
	{FOR, "for", ""},
	{FOREACH, "foreach", ""},
	{GLOBAL, "global", ""},
	{IF, "if", ""},
	{_IN, "in", ""},
	{INCLUDE, "use", ""},
	{IMPORT, "import", ""},
	{NIL, "void", ""},
	//{POINTCUT, "pointcut", ""},
	//{REQUIRE, "require", ""},
	{RETURN, "return", ""},
	{SWITCH, "switch", ""},
	{THROW, "throw", ""},
	{TRY, "try", ""},
	{VAR, "local", ""},
	{WHILE, "while", ""},
	{YIELD, "yield", ""},
    {THEN, "then", ""},
	{WITH, "with", ""},
	{END, "end", ""},
	{LOOP, "loop", ""},
	{POWER, "**", "\\*\\*"},
    {FACT, "!!", "\\!\\!"},
    {STEP, "step", ""},
    {ENUM, "enum", ""},
    {FROM, "from", ""},
	{SPACE, " ", ""},
    {CELL, "=>", ""},
    {IFDEF, "ifdef", ""},
    {ELIFDEF, "elifdef", ""},
    {DEFINE, "def", ""},
    {ENDIF, "endif", ""},
	/* multi-character tokens */
	{FLOATING, "", "(\\d*\\.\\d+([eE]?[-+]?\\d+)?)\
					|(\\d+\\.\\d*([eE]?[-+]?\\d+)?)|(\\d+[eE][-+]?\\d+)"},
	{INTEGER, "", "(0x[a-fA-F0-9]+)|(\\d+)"},
	{TRIPLEDOT, "...", "\\.\\.\\."},
	{DOUBLEDOT, "..", "\\.\\."},
	{INC, "++", "\\+\\+"},
	{DEC, "--", "--"},
	{PASS, "+=", "\\+="},
	{MASS, "-=", "-="},
	{MULASS, "*=", "\\*="},
	{DASS, "/=", "/="},
	{MODASS, "%=", "%="},
	{LSASS, "<<=", "<<="},
	{RSASS, ">>=", ">>="},
	{BAASS, "&=", "&="},
	{BOASS, "|=", "|="},
	{BXASS, "^=", "\\^="},
	{LEQ, "<=", "<="},
	{GEQ, ">=", ">="},
    {TEQUAL, "===", "\\=\\=\\="},
	{TNOTEQUAL, "!==", "\\!\\=\\="},
	{EQUAL, "==", "=="},
	{NOTEQUAL, "!=", "!="},
	{LOGAND, "&&", "&&"},
	{LOGAND, "and", ""},
	{LOGOR, "||", "\\|\\|"},
	{LOGOR, "or", ""},
	{LSHIFT, "<<", "<<"},
	{RSHIFT, ">>", ">>"},
	{'(', "::", "\\:\\:"},
	{BEGIN, ":", ":"},

	/* single-character tokens */
	{'(', "(", "\\("}, {')', ")", "\\)"}, {'[', "[", "\\["}, {']', "]", "\\]"},
	{',', "{", "\\{"}, {')', "}", "\\}"}, {'.', ".", "\\."}, {',', ",", ","},
	{';', ";", ";"}, {'\\', "\\", "\\"}, {'+', "+", "\\+"},
	{'-', "-", "-"}, {'*', "*", "\\*"}, {'/', "/", "/"}, {'%', "%", "%"},
	{'=', "=", "="}, {'<', "<", "<"}, {'>', ">", ">"}, {'?', "?", "\\?"},
	{'!',"!","!"}, {'&',"&","&"}, {'|',"|","|"}, {'^',"^","\\^"},
	{'~', "~", "~"}, {'°',"°","\\°"}, {'@',"@","\\@"}, {'#',"#","\\#"},
};

typedef struct
{
	regnode* reg;
	char first;
} REGNODE;

typedef struct
{
	char source[ZEN_MAX_PATH_LEN];	/* source path */
	char* buf;						/* buffer for the source */
	long pos;						/* character position in the stream */
	_token token, next_token;		/* current and next tokens */
	int expected;					/* expected next token */
	REGNODE regtrees[sizeof(tokens)/sizeof(token_name)];
	uint line_num;
	char* lineerrors;	/* tracks the line error */
	uint line_size;		/* size of lineerrors */
} LexInfo;

int clLexOpen(LexInfo* li, const char* source, char* buf);
void clLexClose(LexInfo* li);
int clStep(LexInfo* li);
char* clGetTokenName(int token);
void freetoken(_token* tok);
int copytoken(_token* dest, _token* source);

#endif
