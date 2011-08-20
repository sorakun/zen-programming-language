/*
  Image functions.
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
*                       Image         Functions                     *
********************************************************************/

void image_createfromfile(avm *vm)
{
    word wr;
    seti(&wr, sfImage_CreateFromFile(getstring(vm, 0)));
    returnv(vm, &wr);
}

void image_create(avm *vm)
{
    word wr;
    seti(&wr, sfImage_Create());
    returnv(vm, &wr);
}

void Image_CreateFromPixels(avm *vm)
{
    word wr;
    seti(&wr, sfImage_CreateFromPixels(getint(vm, 0), getint(vm, 1), getint(vm,2)));
    returnv(vm, &wr);
}

void Image_CreateFromMemory(avm *vm)
{
    word wr;
    seti(&wr, sfImage_CreateFromMemory(getstring(vm, 0), getint(vm, 1)));
    returnv(vm, &wr);
}

void image_save(avm *vm)
{
    word wr;
    seti(&wr, sfImage_SaveToFile(getint(vm, 0), getstring(vm, 1)));
    returnv(vm, &wr);
}

void Img_CreateMastFromColor(avm *vm)
{
    sfColor color = {getint(vm, 1), getint(vm, 2), getint(vm, 3), getint(vm, 4)};
    sfImage_CreateMaskFromColor(getint(vm, 0), color, getint(vm, 5));


}

void Img_Copy(avm *vm)
{
    sfIntRect Rect;
    Rect.Bottom=getint(vm, 4);
    Rect.Left=getint(vm, 5);
    Rect.Right=getint(vm, 6);
    Rect.Top=getint(vm, 7);
    sfImage_Copy(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3), Rect);
}

void Img_CopyScreen(avm *vm)
{
    sfIntRect Rect;
    Rect.Bottom=getint(vm, 2);
    Rect.Left=getint(vm, 3);
    Rect.Right=getint(vm, 4);
    Rect.Top=getint(vm, 5);
    sfImage_CopyScreen(getint(vm, 0), getint(vm, 1), Rect);
}

// ------------------------------

void Image_GetPixel(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfImage_GetPixel(getint(vm, 0), getint(vm, 1), getint(vm, 2));
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

void Image_GetPixelsPtr(avm *vm)
{
    word wr;
    seti(&wr, sfImage_GetPixelsPtr(getint(vm, 0)));
	returnv(vm, &wr);
}

void img_createfromcolor(avm *vm)
{
    word wr;
    sfColor Color = {getint(vm, 2), getint(vm, 3), getint(vm, 4), getint(vm, 5)};
    seti(&wr, sfImage_CreateFromColor(getint(vm, 0), getint(vm, 1), Color));
    returnv(vm, &wr);
}

void image_destroy(avm *vm)
{
    sfImage_Destroy(getint(vm, 0));
}

void img_setpixel(avm *vm)
{
    sfColor colors = {getint(vm, 3), getint(vm, 4), getint(vm, 5), getint(vm, 6)};
    sfImage_SetPixel(getint(vm, 0), getint(vm, 1), getint(vm, 2), colors);
}

void image_getheight(avm *vm)
{
    word w;
    seti(&w,sfImage_GetHeight(getint(vm, 0)));
    returnv(vm, &w);
}

void image_getwidth(avm *vm)
{
    word w;
    seti(&w,sfImage_GetWidth(getint(vm, 0)));
    returnv(vm, &w);
}

void image_setsmooth(avm *vm)
{
    sfImage_SetSmooth(getint(vm, 0), getint(vm, 1));
}


void image_issmooth(avm *vm)
{
    word w;
    seti(&w, sfImage_IsSmooth(getint(vm, 0)));
    returnv(vm, &w);
}

void image_bind(avm *vm)
{
    sfImage_Bind(getint(vm, 0));
}

static fptrname imagelib[] =
{
    {image_createfromfile, "$Image_FromFile"},
	{image_create, "$Image"},
	{img_createfromcolor, "Img_CreateFromColor"},
	{Image_CreateFromPixels, "Image_CreateFromPixels"},
	{Image_CreateFromMemory, "Image_CreateFromMemory"},
	{image_destroy, "Image_Destroy"},
	{img_setpixel, "Img_SetPixel"},
	{image_save, "Image_SaveToFile"},
	{Img_CreateMastFromColor, "Img_CreateMastFromColor"},
	{Img_Copy, "Img_Copy"},
	{Img_CopyScreen, "Img_CopyScreen"},
	{Image_GetPixel, "image_GetPixel"},
	{Image_GetPixelsPtr, "Image_GetPixelsPtr"},
	{image_getheight, "Image_GetHeight"},
	{image_getwidth, "Image_GetWidth"},
	{image_setsmooth, "Image_SetSmooth"},
	{image_issmooth, "Image_IsSmooth"},
	{image_bind, "Image_Bind"},
};

void openimagelib()
{
	int i;
	for (i=0; i<sizeof(imagelib)/sizeof(fptrname); i++)
		zen_regfunc(imagelib[i].ptr, imagelib[i].name);
}
