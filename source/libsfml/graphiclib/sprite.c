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
*                       Sprites       Functions                     *
********************************************************************/

void sprite_create(avm *vm)
{
    word w;
    seti(&w, sfSprite_Create());
    returnv(vm, &w);
}

void sprite_destroy(avm *vm)
{
    sfSprite_Destroy(getint(vm, 0));
}

void sprite_setx(avm *vm)
{
     sfSprite_SetX(getint(vm, 0), getfloat(vm, 1));
}

void sprite_sety(avm *vm)
{
     sfSprite_SetY(getint(vm, 0), getfloat(vm, 1));
}

void sprite_setpos(avm *vm)
{
     sfSprite_SetPosition(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_setscalex(avm *vm)
{
     sfSprite_SetScaleX(getint(vm, 0), getfloat(vm, 1));
}

void sprite_setscaley(avm *vm)
{
     sfSprite_SetScaleY(getint(vm, 0), getfloat(vm, 1));
}

void sprite_setscale(avm *vm)
{
     sfSprite_SetScale(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_setrotation(avm *vm)
{
     sfSprite_SetRotation(getint(vm, 0), getfloat(vm, 1));
}

void sprite_setcenter(avm *vm)
{
     sfSprite_SetCenter(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_setcolor(avm *vm)
{
    sfColor col = {getint(vm, 1),getint(vm, 2),getint(vm, 3),getint(vm, 4)};
    sfSprite_SetColor(getint(vm, 0), col);
}

void sprite_setblendmode(avm *vm)
{
     sfSprite_SetBlendMode(getint(vm, 0), getint(vm, 1));
}

void sprite_getx(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetX(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_gety(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetY(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getscalex(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetScaleX(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getscaley(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetScaleY(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getrotation(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetRotation(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getcenterx(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetCenterX(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getcentery(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetCenterY(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getcolor(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfSprite_GetColor(getint(vm, 0));
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

void sprite_getblendmode(avm *vm)
{
     word w;
     seti(&w, sfSprite_GetBlendMode(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_move(avm *vm)
{
     sfSprite_Move(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_scale(avm *vm)
{
     sfSprite_Scale(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_rotate(avm *vm)
{
     sfSprite_Rotate(getint(vm, 0), getfloat(vm, 1));
}

void sprite_transformtolocal(avm *vm)
{
    float x = getfloat(vm, 3), y = getfloat(vm, 4);
    sfSprite_TransformToLocal(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), &x, &y);
}

void sprite_transformtogolbal(avm *vm)
{
    float x = getfloat(vm, 3), y = getfloat(vm, 4);
    sfSprite_TransformToGlobal(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), &x, &y);
}

void sprite_setimage(avm *vm)
{
     sfSprite_SetImage(getint(vm, 0), getint(vm, 1));
}
// not set !
void sprite_setsubrect(avm *vm)
{
     //sfSprite_SetSubRect(sfSprite* Sprite, sfIntRect SubRect);
}

void sprite_resize(avm *vm)
{
     sfSprite_Resize(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void sprite_flipx(avm *vm)
{
     sfSprite_FlipX(getint(vm, 0), getint(vm, 1));
}

void sprite_flipy(avm *vm)
{
     sfSprite_FlipY(getint(vm, 0), getint(vm, 1));
}

void sprite_getimage(avm *vm)
{
     word w;
     seti(&w, sfSprite_GetImage(getint(vm, 0)));
     returnv(vm, &w);
}
// not set !
void sprite_getsubrect(avm *vm)
{
     //sfIntRect sfSprite_GetSubRect(sfSprite* Sprite);
}

void sprite_getwidht(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetWidth(getint(vm, 0)));
     returnv(vm, &w);
}

void sprite_getheight(avm *vm)
{
     word w;
     setf(&w, sfSprite_GetHeight(getint(vm, 0)));
     returnv(vm, &w);
}
//not set !
void sprite_getpixel(avm *vm)
{
     //sfColor sfSprite_GetPixel(sfSprite* Sprite, unsigned int X, unsigned int Y);
}

static fptrname spritelib[] =
{
    {sprite_create, "$Sprite"},
    {sprite_destroy, "Sprite_Destroy"},
    {sprite_setx, "Sprite_SetX"},
    {sprite_sety, "Sprite_SetY"},
    {sprite_setpos, "Sprite_SetPos"},
    {sprite_setscalex, "Sprite_ScaleX"},
    {sprite_setscaley, "Sprite_ScaleY"},
    {sprite_setscale, "Sprite_SetScale"},
    {sprite_setrotation, "Sprite_SetRotation"},
    {sprite_setcenter, "Sprite_SetCenter"},
    {sprite_setcolor, "Sprite_SetColor"},
    {sprite_setblendmode, "Sprite_SetBlendMode"},
    {sprite_getx, "Sprite_GetX"},
    {sprite_gety, "Sprite_GetY"},
    {sprite_getscalex, "Sprite_GetScaleX"},
    {sprite_getscaley, "Sprite_GetScaleY"},
    {sprite_getrotation, "Sprite_GetRotation"},
    {sprite_getcenterx, "Sprite_GetCenterX"},
    {sprite_getcentery,"Sprite_GetCenterY"},
    {sprite_getcolor, "Sprite_GetColor"},
    {sprite_getblendmode, "Sprite_GetBlendMode"},
    {sprite_move, "Sprite_Move"},
    {sprite_scale, "Sprite_Scale"},
    {sprite_rotate, "Sprite_Rotate"},
    {sprite_transformtogolbal, "Sprite_TransformToGolbal"},
    {sprite_transformtolocal, "Sprite_TransformToLocal"},
    {sprite_setimage, "Sprite_SetImage"},
    {sprite_setsubrect, "Sprite_SetSubrect"},
    {sprite_resize, "Sprite_Resize"},
    {sprite_flipx, "Sprite_FlipX"},
    {sprite_flipy, "Sprite_FlipY"},
    {sprite_getimage, "Sprite_GetImage"},
    {sprite_getsubrect, "Sprite_GetSubrect"},
    {sprite_getwidht, "Sprite_GetWidht"},
    {sprite_getheight, "Sprite_GetHeight"},
};

void openspritelib()
{
	int i;
	for (i=0; i<sizeof(spritelib)/sizeof(fptrname); i++)
		zen_regfunc(spritelib[i].ptr, spritelib[i].name);
}
