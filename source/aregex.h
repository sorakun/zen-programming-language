/* See Copyright Notice in azure.h. */

#ifndef _AREGEX_H_
#define _AREGEX_H_

#define ZEN_MAX_TOKEN_LEN 32
#define ZEN_MAX_LITERAL_LENGTH 64*1024

typedef struct _charrange	/* character range */
{
	uchar start, end;
	struct _charrange* next;	/* for chaining */
} charrange;

typedef struct _charclass	/* character class */
{
	char classid;
	charrange* cr;			/* a group of ranges */
	char isnegate;			/* a negated class */
} charclass;

typedef struct _regnode	/* regex tree node */
{
	char op;				/* if op is NULL, it is a leaf */
	int length;				/* the "real" string length of the node */
	charclass cl;
	uint minrep, maxrep;	/* min and max repeat for greedy closure */
	struct _regnode *left, *right;
} regnode;

int regmatch(const uchar* str, int* pos, regnode* t);
regnode* regexp(const uchar* pattern, int* pos, int precedence);
void freeregnode(regnode* t);

#endif
