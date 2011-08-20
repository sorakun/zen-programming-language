/*
  Sprite functions.
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
*                       String        Functions                     *
********************************************************************/

void string_create(avm *vm)
{
    word w;
    seti(&w, sfString_Create());
    returnv(vm, &w);
}

void string_destroy(avm *vm)
{
    sfString_Destroy(getint(vm, 0));
}

void string_setx(avm *vm)
{
    sfString_SetX (getint(vm, 0), getfloat(vm, 1));
}

void string_sety(avm *vm)
{
    sfString_SetY(getint(vm, 0), getfloat(vm, 1));
}

void string_setpos(avm *vm)
{
    sfString_SetPosition (getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void string_setscalex(avm *vm)
{
    sfString_SetScaleX (getint(vm, 0), getfloat(vm, 1));
}

void string_setscaley(avm *vm)
{
    sfString_SetScaleY (getint(vm, 0), getfloat(vm, 1));
}

void string_setscale(avm *vm)
{
    sfString_SetScale (getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void string_setrotation(avm *vm)
{
    sfString_SetRotation (getint(vm, 0), getfloat(vm, 1));
}

void string_setcenter(avm *vm)
{
    sfString_SetCenter(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void string_setcolor(avm *vm)
{
    sfColor col;
    col.r = getint(vm, 1);
    col.g = getint(vm, 2);
    col.b = getint(vm, 3);
    col.a = getint(vm, 4);
    sfString_SetColor(getint(vm, 0), col);
}

void string_setblendmode(avm *vm)
{
    sfString_SetBlendMode(getint(vm, 0), getint(vm, 1));

}

void string_getx(avm *vm)
{
    word w;
    setf(&w,  sfString_GetX(getint(vm, 0)));
    returnv(vm, &w);
}

void string_gety(avm *vm)
{
    word w;
    setf(&w,  sfString_GetY(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getscalex(avm *vm)
{
    word w;
    setf(&w,  sfString_GetScaleX(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getscaley(avm *vm)
{
    word w;
    setf(&w,  sfString_GetScaleY(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getrotation(avm *vm)
{
    word w;
    setf(&w,  sfString_GetRotation(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getscreenx(avm *vm)
{
    word w;
    setf(&w,  sfString_GetCenterX(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getscreeny(avm *vm)
{
    word w;
    setf(&w,  sfString_GetCenterY(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getcolor(avm *vm)
{

     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfString_GetColor(getint(vm, 0));
     sets(&index, newstring(vm, "R"));
     sets(&source, newstring(vm, col.r));
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "G"));
     sets(&source, newstring(vm, col.g));
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "B"));
     sets(&source, newstring(vm, col.b));
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "A"));
     sets(&source, newstring(vm, col.a));
     IA(vm, tbl, tindex, &index, &source);
     sett(&w, tindex);
     returnv(vm, &w);
}

void string_move(avm *vm)
{
    sfString_Move(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void string_rotate(avm *vm)
{
    sfString_Rotate(getint(vm, 0), getfloat(vm, 1));
}

void string_transformtolocal(avm *vm)
{
    float x = getfloat(vm, 3), y = getfloat(vm, 4);
    sfString_TransformToLocal(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), &x, &y);
}

void string_transformtoglobal(avm *vm)
{
    float x = getfloat(vm, 3), y = getfloat(vm, 4);
    sfString_TransformToGlobal(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), &x, &y);
}

void string_settext(avm *vm)
{
    sfString_SetText(getint(vm, 0), getstring(vm, 1));
}

void string_setunicodetext(avm *vm)
{
    sfString_SetUnicodeText(getint(vm, 0), getstring(vm, 1));
}

void string_setfont(avm *vm)
{
    sfString_SetFont(getint(vm, 0), getint(vm, 1));
}

void string_setsize(avm *vm)
{
    sfString_SetSize(getint(vm, 0), getfloat(vm, 1));
}

void string_setstyle(avm *vm)
{
    sfString_SetStyle(getint(vm, 0), getint(vm, 1));
}

void string_getunicodetext(avm *vm)
{
    word w;
    sets(&w,  sfString_GetUnicodeText(getint(vm, 0)));
    returnv(vm, &w);
}

void string_gettext(avm *vm)
{
    word w;
    sets(&w,  sfString_GetText(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getfont(avm *vm)
{
    word w;
    seti(&w,  sfString_GetFont(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getsize(avm *vm)
{
    word w;
    setf(&w,  sfString_GetSize(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getstyle(avm *vm)
{
    word w;
    seti(&w,  sfString_GetStyle(getint(vm, 0)));
    returnv(vm, &w);
}

void string_getcharpos(avm *vm)
{
    float x= getfloat(vm, 2), y= getfloat(vm, 3);
    sfString_GetCharacterPos(getint(vm, 0), getint(vm, 1), &x, &y);
}

void string_getrect(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfFloatRect col = sfString_GetRect(getstring(vm, 0));
     sets(&index, newstring(vm, "Left"));
     seti(&source, col.Left);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "Top"));
     seti(&source, col.Top);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "Right"));
     seti(&source, col.Right);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, newstring(vm, "Bottom"));
     seti(&source, col.Bottom);
     IA(vm, tbl, tindex, &index, &source);
     sett(&w, tindex);
     returnv(vm, &w);
}

/********************************************************************
*                       Font          Functions                     *
********************************************************************/

void font_create(avm *vm)
{
    word w;
    seti(&w,  sfFont_Create());
    returnv(vm, &w);
}

void font_createfromfile(avm *vm)
{
    word w;
    seti(&w,  sfFont_CreateFromFile(getstring(vm, 0), getint(vm, 1), NULL));
    returnv(vm, &w);
}

void font_destroy(avm *vm)
{
     sfSprite_Destroy (getint(vm, 0));
}

void font_createfrommemory(avm *vm)
{
    //sfFont_CreateFromMemory(const char* Data, size_t SizeInBytes, unsigned int CharSize, const sfUint32* Charset);
}

static fptrname stringlib[] =
{
	{string_create, "$String"},
	{string_destroy, "String_Destroy"},
	{string_setx, "String_SetX"},
	{string_sety, "String_SetY"},
	{string_setpos, "String_SetPos"},
	{string_setscalex, "String_SetScaleX"},
	{string_setscaley, "String_SetScaleY"},
	{string_setscale, "String_SetScale"},
	{string_setrotation, "String_SetRotation"},
    {string_setcenter, "String_SetCenter"},
	{string_setcolor, "str_setcolor"},
	{string_setblendmode, "String_SetBlendMode"},
	{string_getx, "String_GetX"},
	{string_gety, "String_gety"},
	{string_getscalex, "String_GetscaleX"},
	{string_getscaley, "String_GetscaleY"},
	{string_getrotation, "String_GetRotation"},
	{string_getscreenx, "String_GetScreenX"},
	{string_getscreeny, "String_GetscreenY"},
	{string_move, "String_Move"},
	{string_rotate, "String_Rotate"},
	{string_transformtolocal, "String_TransformToLocal"},
	{string_transformtoglobal, "string_TransformToGlobal"},
	{string_settext, "String_SetText"},
	{string_setunicodetext, "String_SetUnicodeText"},
	{string_setfont, "String_SetFont"},
	{string_setsize, "String_SetSize"},
	{string_setstyle, "String_SetStyle"},
	{string_getunicodetext, "String_GetUnicodeText"},
	{string_gettext, "String_GetText"},
	{string_getfont, "String_GetFont"},
	{string_getsize, "String_GetSize"},
	{string_getstyle, "String_Getstyle"},


	{font_create, "$Font"},
	{font_createfromfile, "$Font_FromFile"}
};

void openstringlib()
{
	int i;
	for (i=0; i<sizeof(stringlib)/sizeof(fptrname); i++)
		zen_regfunc(stringlib[i].ptr, stringlib[i].name);
}

