/*
 * String and regular expression library.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "zen.h"
#include "alib.h"
#include "aregex.h"
#include "atable.h"
#include "aexception.h"
#include "autil.h"
#include "aerror.h"

/*
	Matches str against the pattern from start. If some match is found,
	a non-zero value is returned which is the length of the matched substring.
*/
int trymatch(const uchar* str, const uchar* pattern)
{
	int pos0=0, pos1=0;
	regnode* t = regexp(pattern, &pos0, 0);
	int res = regmatch(str, &pos1, t)?pos1:0;
	freeregnode(t);
	return res;
}

/*
	Find all the matched substrings in str for the pattern. The results are
	in a table, and its offset is returned.
*/
int refind(avm* vm, const char* str, const char* pattern)
{
	uint pos = 0, lastpos = 0;
	uint length = strlen(str);
	long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word index, source;
	long ind = 0;
	int match;
	regnode* t = regexp(pattern, &pos, 0);
	char *wordbuf = calloc(length, sizeof(char));
	if (!wordbuf)
	{
		setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
		return 0;
	}
	while (lastpos < length)
	{
		pos = 0;
		match = regmatch(str+lastpos, &pos, t);
		if (pos==0 || !match)
			lastpos++;
		else
		{
			memcpy(wordbuf, str+lastpos, sizeof(char)*pos);
			wordbuf[pos] = 0;
			seti(&index, ind++);
			sets(&source, newstring(vm, wordbuf));
			IA(vm, tbl, tindex, &index, &source);
			lastpos += pos;
		}
	}
	free(wordbuf);
	freeregnode(t);
	return tindex;
}

/*
	Split a string using specified seperators seps. All the substrings are
	stored in a table and its index is returned.
*/
int wstrsep(avm* vm, word* strw, word* sepsw)
{
	long off = newtable(vm, ZEN_INITIALTABLESIZE);
	char* str = (char*)getdata(vm->hp.heap, strw->entity.ival);
	uint length = strlen(str);
	char* seps = (char*)getdata(vm->hp.heap, sepsw->entity.ival);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word index, source;
	long ind = 0;
	char *tok;
	tok = strtok((char*)str, seps);
	while (tok)
	{
		seti(&index, ind++);
		sets(&source, newstring(vm, tok));
		IA(vm, tbl, tindex, &index, &source);
		tok = strtok(0, seps);
	}
	return tindex;
}

/* Replace all the substrings by a pattern. */
char* rerepl(const char* str, const char* pattern, const char* sub)
{
	uint pos = 0, lastpos = 0, replpos = 0;
	uint length = (strlen(sub)/strlen(pattern)+1)*strlen(str);
	char* repl = calloc(length+1, sizeof(char));
	regnode* t = regexp(pattern, &pos, 0);
	int match;
	while (lastpos < strlen(str))
	{
		pos = 0;
		match = regmatch(str+lastpos, &pos, t);
		if (pos==0 || !match)
			repl[replpos++] = str[lastpos++];
		else
		{
			memcpy(repl+replpos, sub, sizeof(char)*strlen(sub));
			replpos += strlen(sub);
			lastpos += pos;
		}
	}
	freeregnode(t);
	return repl;
}

/* Match a string to a pattern. */
void cstrmat(avm* vm)
{
	char* str = getstring(vm, 0);
	char* pattern = getstring(vm, 1);
	char* newstr;
	int matchlen = trymatch(str, pattern);
	word wr = {0};
	if (matchlen)
	{
		newstr = calloc(matchlen+1, sizeof(char));
		memcpy(newstr, str, matchlen*sizeof(char));
		sets(&wr, newstring(vm, newstr));
		free(newstr);
	}
	returnv(vm, &wr);
}

/* Find all the matched substrings by a pattern. */
void cstrfnd(avm* vm)
{
	char* str = getstring(vm, 0);
	char* pattern = getstring(vm, 1);
	word wr = {0};
	sett(&wr, refind(vm, str, pattern));
	returnv(vm, &wr);
}

/* Replace all the matched substrings by a pattern with a new substring. */
void cstrrepl(avm* vm)
{
	char* str = getstring(vm, 0);
	char* pattern = getstring(vm, 1);
	char* sub = getstring(vm, 2);
	char* repl = rerepl(str, pattern, sub);
	word wr;
	sets(&wr, newstring(vm, repl));
	free(repl);
	returnv(vm, &wr);
}

/* Return a string length. */
void cstrlen(avm* vm)
{
	char* str = getstring(vm, 0);
	word wr = {0};
	seti(&wr, strlen(str));
	returnv(vm, &wr);
}

/* Convert a string to lower case. */
void cstrlwr(avm* vm)
{
	char* str = getstring(vm, 0);
	word wr = *(getarg(vm, 0));
	int i;
	for (i=0; i<(int)strlen(str); i++)
		str[i] = tolower(str[i]);
	returnv(vm, &wr);
}

/* Convert a string to upper case. */
void cstrupr(avm* vm)
{
	char* str = getstring(vm, 0);
	word wr = *(getarg(vm, 0));
	int i;
	for (i=0; i<(int)strlen(str); i++)
		str[i] = toupper(str[i]);
	returnv(vm, &wr);
}

/* Return a string copy. */
void cstrcpy(avm* vm)
{
	char* str = getstring(vm, 0);
	word wr = {0};
	sets(&wr, newstring(vm, str));
	returnv(vm, &wr);
}

/* Repeat a string n times. */
void cstrrep(avm* vm)
{
	word wr = {0};
	char *str = getstring(vm, 0), *repstr;
	int times = getint(vm, 1), i;
	if (times <= 0)
		setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
	else
	if (!(repstr=calloc(times*strlen(str)+1, sizeof(char))))
		setvmerror(vm, ZEN_OUT_OF_MEMORY);
	else
	{
		for (i=0; i<times; i++)
			strcat(repstr, str);
		sets(&wr, newstring(vm, repstr));
		free(repstr);
		returnv(vm, &wr);
	}
}

/* Get a part of a string between two indices. The indices can be negative. */
void cstrsub(avm* vm)
{
	char* str = getstring(vm, 0);
	char* newstr = calloc(strlen(str)+1, sizeof(char));
	int pos1 = getint(vm, 1);
	int pos2 = getint(vm, 2);
	word wr;
	if (abs(pos1)>=(int)strlen(str) || abs(pos2)>=(int)strlen(str))
	{
		throwoor(vm);
		free(newstr);
		return;
	}
	if (pos1<0) pos1 = strlen(str)+pos1;
	if (pos2<0) pos2 = strlen(str)+pos2;
	memcpy(newstr, str+pos1, pos2-pos1+1);
	sets(&wr, newstring(vm, newstr));
	free(newstr);
	returnv(vm, &wr);
}

/* Reverse a string. */
void cstrrev(avm* vm)
{
	char* str = getstring(vm, 0);
	char* revstr = calloc(strlen(str)+1, sizeof(char));
	int i, len = strlen(str);
	word w;
	for (i=0; i<len; i++)
		revstr[i] = str[len-i-1];
	sets(&w, newstring(vm, revstr));
	free(revstr);
	returnv(vm, &w);
}

/* Seperate a string, stored them in a table, and return the table. */
void cstrsep(avm* vm)
{
	word w;
	word* strw = getarg(vm, 0);
	word* sepsw = getarg(vm, 1);
	sett(&w, wstrsep(vm, strw, sepsw));
	returnv(vm, &w);
}

/* Cut the spaces of a string's head and tail. */
void cstrcut(avm* vm)
{
	word w;
	char* str = getstring(vm, 0);
	char* spaces = getstring(vm, 1);
	strcut(str, spaces);
	sets(&w, newstring(vm, str));
	returnv(vm, &w);
}

/* Return a one-character string given the integer value: int->char. */
void cchr(avm* vm)
{
	int val = getint(vm, 0);
	char buf[2] = {0};
	word wr = {0};
	buf[0] = (char)val;
	sets(&wr, newstring(vm, buf));
	returnv(vm, &wr);
}
#ifdef ZEN_ENABLE_SFL
void String_Remove(avm *vm)
{
    word w;
    sets(&w, newstring(vm, replacechrswith(getstring(vm, 0), getstring(vm, 1), "")));
    returnv(vm, &w);
}

void String_Replace(avm *vm)
{
    word w;
    sets(&w, newstring(vm, replacechrswith(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void String_Insert(avm *vm)
{
    word w;
    sets(&w, newstring(vm, insertstring(getstring(vm, 0), getstring(vm, 1), getint(vm, 2))));
    returnv(vm, &w);
}

void String_SearchReplace(avm *vm)
{
    word w;
    sets(&w, newstring(vm, searchreplace(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void String_SearchDelete(avm *vm)
{
    word w;
    sets(&w, newstring(vm, deletestring(getstring(vm, 0), getstring(vm, 1), getint(vm, 2))));
    returnv(vm, &w);
}
#endif

static fptrname libstr[] =
{
	{cstrmat, "strmat"},
	{cstrfnd, "strfnd"},
	{cstrrepl, "strrpl"},
	{cstrlen, "String_GetLength"},
	{cstrcpy, "String_Copy"},
	{cstrlwr, "String_toLower"},
	{cstrupr, "String_toUpper"},
	{cstrsub, "String_GetSub"},
	{cstrrep, "strrep"},
	{cstrrev, "strrev"},
	{cstrsep, "strsep"},
	{cstrcut, "String_Cut"},
	{cchr, "chr"},
	#ifdef ZEN_ENABLE_SFL
    {String_Remove, "String_Remove"},
    {String_Replace, "String_Replace"},
    {String_SearchReplace, "String_SearchReplace"},
    #endif
};

void zen_openlibstr()
{
	int i;
	for (i=0; i<sizeof(libstr)/sizeof(fptrname); i++)
		zen_regfunc(libstr[i].ptr, libstr[i].name);
}
