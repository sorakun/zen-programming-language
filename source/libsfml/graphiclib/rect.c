/*
  Rect functions
*/

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
*                       Rect          Functions                     *
********************************************************************/

void Rect_Offset(avm *vm)
{
    sfFloatRect* Rect = NULL;
    Rect->Left = getfloat(vm, 0);
    Rect->Top = getfloat(vm, 1);
    Rect->Right = getfloat(vm, 2);
    Rect->Bottom = getfloat(vm, 3);
    sfFloatRect_Offset(&Rect, getfloat(vm, 4), getfloat(vm, 5));
}

void FloatRect_Contains(avm *vm)
{
    sfFloatRect* Rect = NULL;
    Rect->Left = getfloat(vm, 0);
    Rect->Top = getfloat(vm, 1);
    Rect->Right = getfloat(vm, 2);
    Rect->Bottom = getfloat(vm, 3);
    word w;
    seti(&w, sfFloatRect_Contains(&Rect, getfloat(vm, 4), getfloat(vm, 5)));
    returnv(vm, &w);
}

void FloatRect_Intersects(avm *vm)
{
    sfFloatRect* Rect1 = NULL;
    sfFloatRect* Rect2 = NULL;
    sfFloatRect* Rect3 = NULL;

    Rect1->Left = getfloat(vm, 0);
    Rect1->Top = getfloat(vm, 1);
    Rect1->Right = getfloat(vm, 2);
    Rect1->Bottom = getfloat(vm, 3);

    Rect2->Left = getfloat(vm, 4);
    Rect2->Top = getfloat(vm, 5);
    Rect2->Right = getfloat(vm, 6);
    Rect2->Bottom = getfloat(vm, 7);

    word wr, w , table;
    seti(&wr, sfFloatRect_Intersects(&Rect1, &Rect2, Rect3));

    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    ctable* tb2 = (ctable*)getdata(vm->hp.heap, off);
    word index, source;

    sets(&index, newstring(vm, "Left"));
    setf(&source, Rect3->Left);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Right"));
    setf(&source, Rect2->Right);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Top"));
    setf(&source, Rect2->Top);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Bottom"));
    setf(&source, Rect2->Bottom);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    sets(&index, newstring(vm, "Intersects"));
    IA(vm, tb2, tindex, &index, &wr);
    sets(&index, newstring(vm, "Rect"));
    IA(vm, tb2, tindex, &index, &w);

    sett(&table, tindex);
    returnv(vm, &table);
}

// Saving functions:
static fptrname rectlib[] =
{
    {Rect_Offset, "rect_offset"},
    {FloatRect_Contains, "floatrect_contains"},
    {FloatRect_Intersects, "floatrect_intersects"}
};


void openrectlib()
{
	int i;
	for (i=0; i<sizeof(rectlib)/sizeof(fptrname); i++)
		zen_regfunc(rectlib[i].ptr, rectlib[i].name);
}
