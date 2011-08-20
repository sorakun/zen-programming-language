/*
 * Regular expression functions.
 * See Copyright Notice in azure.h.
*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "zen.h"
#include "aregex.h"
#include "aerror.h"
#include "alex.h"

#define CONCAT 1
#define CLOSURE 2

#define CHARACTERCLASS "acdglprsuwxACDGLPRSUWX"

regnode* regexp(const uchar* pattern, int* pos, int precedence);

void expect(const uchar* pattern, int* pos, uchar c)
{
	if (pattern[(*pos)++] != c)
		error("Invalid regular expression!\n");
}

int addrange(charclass* cc, uchar start, uchar end)
{
	charrange *cr0, *cr = (charrange*)calloc(1, sizeof(charrange));
	if (cr == NULL)
	{
		error("Out of memory.\n");
		return 0;
	}
	cr->start = start;
	cr->end = end;
	if (cc->cr == NULL)
	{
		cc->cr = cr;
		return 1;
	}
	for (cr0=cc->cr; cr0->next!=NULL; cr0=cr0->next);
	cr0->next = cr;
	return 1;
}

regnode* newregnode()
{
	regnode* t = (regnode*)calloc(1, sizeof(regnode));
	if (t == NULL) error("Out of memory.\n");
	return t;
}

regnode* makeregnode(char op, regnode* left, regnode* right)
{
	regnode* t = newregnode();
	if (t == NULL) return NULL;
	t->op = op;
	t->left = left;
	t->right = right;
	return t;
}

void freecharrange(charrange* cr)
{
	if (cr == NULL) return;
	if (cr->next) freecharrange(cr->next);
	free(cr);
}

void freeregnode(regnode* t)
{
	if (t == NULL) return;
	if (t->left) freeregnode(t->left);
	if (t->right) freeregnode(t->right);
	freecharrange(t->cl.cr);
	free(t);
}

int parsenumber(const uchar* str, int* pos)
{
	int ret = str[(*pos)++]-'0';
	while (isdigit(str[*pos]))
		ret = ret*10+str[(*pos)++]-'0';
	return ret;
}

/* Return a character class in the string from current position. */
regnode* regprimary(const uchar* pattern, int* pos)
{
	regnode *t, *t1;
	int isopen = 0;		/* if true, we have seen a '-' */
	uchar start;		/* the starting character */
	if ((t=newregnode()) == NULL) return NULL;
	switch (pattern[*pos])
	{
	case '(':
		(*pos)++;
		t1 = regexp(pattern, pos, 0);
		freeregnode(t);
		expect(pattern, pos, ')');
		return t1;
	case '[':	/* character class*/
		if (pattern[++(*pos)] == '^')	/* a negated character class */
		{
			(*pos)++;
			t->cl.isnegate = 1;
		}
		while (pattern[*pos] != ']')
		{
			start = pattern[(*pos)++];
			if (pattern[*pos] != '-')
				addrange(&t->cl, start, start);
			else
			{
				addrange(&t->cl, start, pattern[++(*pos)]);
				(*pos)++;
			}
		}
		(*pos)++;
		break;
	case '.':	/* '.' matches any character except newline */
		addrange(&t->cl, 0, '\n');
		addrange(&t->cl, '\n', 255);
		(*pos)++;
		break;
	case '\\':	/* escape characters */
		(*pos)++;
		if (strchr(CHARACTERCLASS, (int)pattern[*pos]))	/* predefined class */
			t->cl.classid = pattern[*pos];
		else/* system */
			switch(pattern[*pos])
			{
			case 'f': addrange(&t->cl, '\f', '\f'); break;
			case 'n': addrange(&t->cl, '\n', '\n'); break;
			case 'r': addrange(&t->cl, '\r', '\r'); break;
			case 't': addrange(&t->cl, '\t', '\t'); break;
			default: addrange(&t->cl, pattern[*pos], pattern[*pos]); break;
			}
		(*pos)++;
		break;
	case '\"':	/* literal strings */
		(*pos)++;
		addrange(&t->cl, pattern[*pos], pattern[*pos]);
		(*pos)++;
		while (pattern[*pos]!='\"' && pattern[*pos]!=0)
		{
			t1 = newregnode();
			addrange(&t1->cl, pattern[*pos], pattern[*pos]);
			t = makeregnode(CONCAT, t, t1);
			(*pos)++;
		}
		expect(pattern, pos, '\"');
		break;
	case '^':	case '$': addrange(&t->cl, '\n', '\n'); (*pos)++; break;
	default:	/* must be a normal letter */
		addrange(&t->cl, pattern[*pos], pattern[*pos]);
		(*pos)++;
		break;
	}
	return t;
}

regnode* regpostfix(const uchar* pattern, int* pos)
{
	regnode* t = regprimary(pattern, pos);
	if (pattern[(*pos)]=='*' || pattern[(*pos)]=='+' || pattern[(*pos)]=='?' ||
		pattern[(*pos)]==BEGIN)
	{
		t = makeregnode(CLOSURE, t, NULL);
		t->maxrep = 0xFFFF;
		switch (pattern[(*pos)++])
		{
		case '*': t->maxrep = 0xFFFF; break;
		case '+': t->minrep = 1; break;
		case '?': t->maxrep = 1; break;
		case BEGIN:
			t->minrep = parsenumber(pattern, pos);
			if (pattern[*pos] == END)
			{
				t->maxrep = t->minrep;
				(*pos)++;
				break;
			}
			if (pattern[*pos] == ',')
			{
				(*pos)++;
				if (pattern[*pos] == END)
				{
					(*pos)++;
					break;
				}
				t->maxrep = parsenumber(pattern, pos);
				expect(pattern, pos, END);
			}
			break;
		}
	}
	return t;
}

#define PRECEDENCE(op) ((op)==CONCAT?2:(op)=='|'?1:0)
regnode* regexp(const uchar* pattern, int* pos, int precedence)
{
	regnode* t = regpostfix(pattern, pos);
	char op = (pattern[*pos]==0?0:pattern[*pos]=='|'?'|':CONCAT);
	while (pattern[*pos]!=0 && pattern[*pos]!=')' && PRECEDENCE(op)>=precedence)
	{
		if (op == '|') (*pos)++;
		t = makeregnode(op, t, regexp(pattern, pos, PRECEDENCE(op)+1));
		op = (pattern[*pos]=='|'?'|':CONCAT);
	}
	return t;
}

int matchclass(uchar c, charclass* cl)
{
	charrange* cr;
	int ret = 0;
	if (c == 0) return 0;	/* the end of the string, can not match */
	if (cl->classid != 0)	/* a predefined class */
		switch (cl->classid)
		{
		case 'a': ret = isalpha((int)c); break;
		case 'c': ret = iscntrl((int)c); break;
		case 'd': ret = isdigit((int)c); break;
		case 'g': ret = isgraph((int)c); break;
		case 'l': ret = islower((int)c); break;
		case 'p': ret = ispunct((int)c); break;
		case 'r': ret = isprint((int)c); break;
		case 's': ret = isspace((int)c); break;
		case 'u': ret = isupper((int)c); break;
		case 'w': ret = isalnum((int)c); break;
		case 'x': ret = isxdigit((int)c); break;
		case 'A': ret = !isalpha((int)c); break;
		case 'C': ret = !iscntrl((int)c); break;
		case 'D': ret = !isdigit((int)c); break;
		case 'G': ret = !isgraph((int)c); break;
		case 'L': ret = !islower((int)c); break;
		case 'P': ret = !ispunct((int)c); break;
		case 'R': ret = !isprint((int)c); break;
		case 'S': ret = !isspace((int)c); break;
		case 'U': ret = !isupper((int)c); break;
		case 'W': ret = !isalnum((int)c); break;
		case 'X': ret = !isxdigit((int)c); break;
		}
	else	/* otherwise, a customized class */
		for (cr=cl->cr; cr!=NULL; cr=cr->next)
			if ((c>=cr->start&&c<=cr->end))
				ret = 1;
	if (cl->isnegate) ret = !ret;
	return ret;
}

/* Match a string against a RE tree. *pos is the offset from the start. */
int regmatch(const uchar* str, int* pos, regnode* t)
{
	int opos = *pos;
	uint matchcount = 0;
	switch (t->op)
	{
	case 0:
		if (matchclass(str[(*pos)], &t->cl))	/* leaf */
		{
			(*pos)++;
			return 1;
		}
		break;
	case CONCAT:
		if (regmatch(str, pos, t->left) && regmatch(str, pos, t->right))
			return 1;
		break;
	case '|':
		if (regmatch(str, pos, t->left)) return 1;
		*pos = opos;	/* restore the position */
		if (regmatch(str, pos, t->right)) return 1;
		break;
	case CLOSURE:
		for (opos=*pos;regmatch(str,pos,t->left)&&*pos<ZEN_MAX_LITERAL_LENGTH;
			opos=*pos)
			matchcount++;
		*pos = opos;	/* restore the position */
		if (matchcount>=t->minrep && matchcount<=t->maxrep)
			return 1;
	}
	return 0;
}
