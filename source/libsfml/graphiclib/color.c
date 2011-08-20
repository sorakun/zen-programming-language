/*
  Color functions.
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../zen.h"
#include "../../alib.h"
#include "../../aregex.h"
#include "../../atable.h"
#include "../../aexception.h"
#include "../../autil.h"
#include "../../aerror.h"
#include <SFML/Graphics.h>

/********************************************************************
*                       Color         Functions                     *
********************************************************************/

void Color_Add(avm *vm)
{
     sfColor c1 = {getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)};
     sfColor c2 = {getint(vm, 4), getint(vm, 5), getint(vm, 6), getint(vm, 7)};
     sfColor col = sfColor_Add(c1, c2);
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sets(&index, newstring(vm, "R"));
     seti(&source, col.r);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "G"));
     seti(&source, col.g);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "B"));
     seti(&source, col.b);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "A"));
     seti(&source, col.a);
     IA(vm, tbl, tindex, &index, &source);
     sett(&w, tindex);
     returnv(vm, &w);
}

void Color_ModulateR(avm *vm)
{
    sfColor c1 = {getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)};
    sfColor c2 = {getint(vm, 4), getint(vm, 5), getint(vm, 6), getint(vm, 7)};
    sfColor c3 = sfColor_Modulate(c1, c2);
    word w;
    seti(&w, c3.r);
    returnv(vm, &w);
}

void Color_ModulateG(avm *vm)
{
    sfColor c1 = {getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)};
    sfColor c2 = {getint(vm, 4), getint(vm, 5), getint(vm, 6), getint(vm, 7)};
    sfColor c3 = sfColor_Modulate(c1, c2);
    word w;
    seti(&w, c3.r);
    returnv(vm, &w);
}
void Color_ModulateB(avm *vm)
{
    sfColor c1 = {getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)};
    sfColor c2 = {getint(vm, 4), getint(vm, 5), getint(vm, 6), getint(vm, 7)};
    sfColor c3 = sfColor_Modulate(c1, c2);
    word w;
    seti(&w, c3.r);
    returnv(vm, &w);
}
void Color_ModulateA(avm *vm)
{
    sfColor c1 = {getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3)};
    sfColor c2 = {getint(vm, 4), getint(vm, 5), getint(vm, 6), getint(vm, 7)};
    sfColor c3 = sfColor_Modulate(c1, c2);
    word w;
    seti(&w, c3.r);
    returnv(vm, &w);
}

static fptrname colorlib[] =
{
	{Color_Add, "Color_add"},
	{Color_ModulateR, "Color_ModulateR"},
	{Color_ModulateG, "Color_ModulateG"},
	{Color_ModulateB, "Color_ModulateB"},
	{Color_ModulateA, "Color_ModulateA"},
};

void opencolorlib()
{
	int i;
	for (i=0; i<sizeof(colorlib)/sizeof(fptrname); i++)
		zen_regfunc(colorlib[i].ptr, colorlib[i].name);
}
