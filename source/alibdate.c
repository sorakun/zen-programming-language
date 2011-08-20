/*
   Date and time controls for Zen.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "zen.h"
#include "alib.h"
#include "aerror.h"
#include "atable.h"

#define MAX_BUF 50

void Date_GetFull(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word DATE, index, source;
	long ind = 0;

    char buf[MAX_BUF];
    time_t seconds = time(NULL);
    struct tm *now = localtime(&seconds);

    // Date
    sets(&index, newstring(vm, "Year"));
    seti(&source,  now->tm_year + 1900);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Month"));
    seti(&source,  now->tm_mon + 1);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Day"));
    seti(&source,  now->tm_mday + 1);
	IA(vm, tbl, tindex, &index, &source);

    // Time
    sets(&index, newstring(vm, "Hour"));
    seti(&source,  now->tm_hour);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Min"));
    seti(&source,  now->tm_min);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Sec"));
    seti(&source,  now->tm_sec);
	IA(vm, tbl, tindex, &index, &source);

    sett(&DATE, tindex);
    returnv(vm, &DATE);
}


static fptrname libdate[] =
{
	{Date_GetFull, "Date_GetFull"},
};

void zen_openlibdate()
{
	int i;
	for (i=0; i<sizeof(libdate)/sizeof(fptrname); i++)
		zen_regfunc(libdate[i].ptr, libdate[i].name);
}
