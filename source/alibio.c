/*
 * IO library.
 * See Copyright Notice in azure.h.
*/

#include <string.h>
#include <stdlib.h>

#include "zen.h"
#include "atable.h"
#include "aexception.h"
#include "autil.h"
#include "agc.h"

#ifndef ZEN_ENABLE_SFL

/* Open a file. The mode strings are identical to those of C's. */

void cfopen(avm* vm)
{
	word w;
	char* fname = getstring(vm, 0);
	char* fmode = getstring(vm, 1);
	FILE* file;
	if (strlen(fname) == 0)
		throwfnf(vm);
	else
	{
		if (!(file=fopen(fname, fmode)))
			throwfnf(vm);
		seth(&w, (int)file);
		returnv(vm, &w);
	}
}

/* Close a file. */
void cfclose(avm* vm)
{
	FILE* fhandle = gethandle(vm, 0);
	if (fhandle) fclose(fhandle);
}

/* Get the file length. */
void cflength(avm* vm)
{
	word w;
	char* fname = getstring(vm, 0);
	seti(&w, flength(fname));
	returnv(vm, &w);
}

/* Read the file content. */
void cfread(avm* vm)
{
	word w;
	FILE* file = gethandle(vm, 0);
	long flen = flengthh(file);
	char* buf = (char*)calloc(flen, sizeof(char));	/* validity */
	fread(buf, sizeof(char), flen, file);
	/*setb(&w, (int)buf);*/
	sets(&w, newstring(vm, buf));
	/*addextobj(vm, &w);*/
	free(buf);
	returnv(vm, &w);
}

/* Write data to file. */
void cfwrite(avm* vm)
{
	word w;
	FILE* file = gethandle(vm, 0);
	char* data = getstring(vm, 1);
	seti(&w, fwrite(data, sizeof(char), strlen(data), file));
	returnv(vm, &w);
}
#endif
/* Print an integer, float, a string, etc. */
void print(avm* vm)
{
	word* w = getarg(vm, 0);
	printw(vm, w);
	returnv(vm, w);
}

/* Print an integer, float, a string, etc, with a carriage-return. */
void println(avm* vm)
{
	word* w = getarg(vm, 0);
	printw(vm, w);
	printf("\n");
	returnv(vm, w);
}

/* Get an integer from the standard input stream. */
void inputi(avm* vm)
{
	word w;
	settypew(&w, TINTEGER);
	scanf("%d", &w.entity.ival);
	fflush(stdin);
	returnv(vm, &w);
}

/* Get a float from the standard input stream. */
void inputf(avm* vm)
{
	word w;
	settypew(&w, TFLOAT);
	float x;
	scanf("%f", &w.entity.fval);
	fflush(stdin);
	returnv(vm, &w);
}

/* Get a character from the standard input stream. */
void inputc(avm* vm)
{
	word w={0};
	getc(stdin);
	returnv(vm, &w);
}

/* Get a string from the standard input stream. */
void inputs(avm* vm)
{
	word w;
	char ret[10240];
	fgets(ret, 10240, stdin);
	strcut(ret, " \t\n\r");
	sets(&w, newstring(vm, ret));
	returnv(vm, &w);
}

void Console_Clear(avm *vm)
{
    #ifdef __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
        system("cls");
    #else
        system("clear");
    #endif
}

static fptrname libio[] =
{
    #ifndef ZEN_ENABLE_SFL
	{cfopen, "File_Open"},
	{cfclose, "File_Close"},
	{cflength, "File_GetLength"},
	{cfread, "File_GetStr"},
	{cfwrite, "File_Put"},
	#endif
	{print, "put"},
	{println, "puts"},
	{inputi, "getint"},
	{inputf, "getfloat"},
	{inputc, "getchar"},
	{inputs, "getstr"},
	{Console_Clear, "Console_Clear"},
};

/* Open the IO lib. */
void zen_openlibio()
{
	int i;
	for (i=0; i<sizeof(libio)/sizeof(fptrname); i++)
		zen_regfunc(libio[i].ptr, libio[i].name);
}
