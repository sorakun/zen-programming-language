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
*                       Shape         Functions                     *
********************************************************************/

void Shape_Create(avm *vm)
{
    word wr;
    seti(&wr, sfShape_Create());
    returnv(vm, &wr);
}

void Shape_CreateLine(avm *vm)
{
    word wr;
    sfColor Col; Col.r = getint(vm, 5); Col.g = getint(vm, 6); Col.b = getint(vm, 7); Col.a = getint(vm, 8);
    sfColor OutlineCol; OutlineCol.r = getint(vm, 10); OutlineCol.g = getint(vm, 11); OutlineCol.b = getint(vm, 12); OutlineCol.a = getint(vm, 13);
    seti(&wr, sfShape_CreateLine(getfloat(vm, 0), getfloat(vm, 1), getfloat(vm, 2), getfloat(vm, 3), getfloat(vm, 4), Col, getfloat(vm, 9), OutlineCol));
    returnv(vm, &wr);
}

void Shape_CreateRectangle(avm *vm)
{
    word wr;
    sfColor Col; Col.r = getint(vm, 4); Col.g = getint(vm, 5); Col.b = getint(vm, 6); Col.a = getint(vm, 7);
    sfColor OutlineCol; OutlineCol.r = getint(vm, 9); OutlineCol.g = getint(vm, 10); OutlineCol.b = getint(vm, 11); OutlineCol.a = getint(vm, 12);
    seti(&wr, sfShape_CreateRectangle(getfloat(vm, 0), getfloat(vm, 1), getfloat(vm, 2), getfloat(vm, 3), Col, getfloat(vm, 8), OutlineCol));
    returnv(vm, &wr);
}

void Shape_CreateCircle(avm *vm)
{
    word wr;
    sfColor Col; Col.r = getint(vm, 3); Col.g = getint(vm, 4); Col.b = getint(vm, 5); Col.a = getint(vm, 6);
    sfColor OutlineCol; OutlineCol.r = getint(vm, 8); OutlineCol.g = getint(vm, 9); OutlineCol.b = getint(vm, 10); OutlineCol.a = getint(vm, 11);
    seti(&wr, sfShape_CreateCircle(getfloat(vm, 0), getfloat(vm, 1), getfloat(vm, 2), Col, getfloat(vm, 7), OutlineCol));
    returnv(vm, &wr);
}

void Shape_SetX(avm *vm)
{
    sfShape_SetX(getint(vm, 0), getfloat(vm, 1));
}

void Shape_SetY(avm *vm)
{
    sfShape_SetY(getint(vm, 0), getfloat(vm, 1));
}

void Shape_SetPosition(avm *vm)
{
    sfShape_SetPosition(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void Shape_SetScaleX(avm *vm)
{
    sfShape_SetScaleX(getint(vm, 0), getfloat(vm, 1));
}

void Shape_SetScaleY(avm *vm)
{
    sfShape_SetScaleY(getint(vm, 0), getfloat(vm, 1));
}

void Shape_SetScale(avm *vm)
{
    sfShape_SetScale(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void Shape_SetRotation(avm *vm)
{
    sfShape_SetRotation(getint(vm, 0), getfloat(vm, 1));
}

void Shape_SetCenter(avm *vm)
{
    sfShape_SetCenter(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void Shape_SetColor(avm *vm)
{
    sfColor Color = {getint(vm, 1), getint(vm, 2), getint(vm, 3), getint(vm, 4)};
    sfShape_SetColor(getint(vm, 0), Color);
}

void Shape_SetBlendMode(avm *vm)
{
    int x = getint(vm, 1);
    sfBlendMode mode;
    if (x == 0)
       mode = sfBlendAlpha;
    else if(x == 1)
       mode = sfBlendAdd;
    else if(x == 2)
       mode = sfBlendMultiply;
    else
       mode = sfBlendNone;
    sfShape_SetBlendMode(getint(vm, 0), mode);
}

void Shape_GetPosition(avm *vm)
{
    word w;
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, sfShape_GetX(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, sfShape_GetY(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void Shape_GetScale(avm *vm)
{
    word w;
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, sfShape_GetScaleX(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, sfShape_GetScaleY(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void Shape_GetRotation(avm *vm)
{
    word w;
    setf(&w, sfShape_GetRotation(getint(vm, 0)));
    returnv(vm, &w);
}

void Shape_GetCenter(avm *vm)
{
    word w;
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, sfShape_GetCenterX(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, sfShape_GetCenterY(getint(vm, 0)));
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void Shape_GetColor(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfShape_GetColor(getint(vm, 0));
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

void Shape_GetBlendMode(avm *vm)
{
    sfBlendMode mode = sfShape_GetBlendMode(getint(vm, 0));
    int x;
    if(mode == sfBlendAlpha)
       x = 0;
    else if(mode == sfBlendAdd)
       x = 1;
    else if(mode == sfBlendMultiply)
       x = 2;
    else
       x = 3;
    word w;
    seti(&w, x);
    returnv(vm, &w);
}

void Shape_Move(avm *vm)
{
    sfShape_Move(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void Shape_Scale(avm *vm)
{
    sfShape_Scale(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2));
}

void Shape_Rotate(avm *vm)
{
    sfShape_Rotate(getint(vm, 0), getfloat(vm, 1));
}

//sfShape_TransformToLocal(sfShape* Shape, float PointX, float PointY, float* X, float* Y);
//sfShape_TransformToGlobal(sfShape* Shape, float PointX, float PointY, float* X, float* Y);

void Shape_AddPoint(avm *vm)
{
    sfColor Col = {getint(vm, 3), getint(vm, 4), getint(vm, 5), getint(vm, 6)};
    sfColor OutlineCol = {getint(vm, 7), getint(vm, 8), getint(vm, 9), getint(vm, 10)};
    sfShape_AddPoint(getint(vm, 0), getfloat(vm, 1), getfloat(vm, 2), Col, OutlineCol);
}

void Shape_EnableFill(avm *vm)
{
    sfShape_EnableFill(getint(vm, 0), getint(vm, 1));
}

void Shape_EnableOutline(avm *vm)
{
    sfShape_EnableOutline(getint(vm, 0), getint(vm, 1));
}

void Shape_SetOutlineWidth(avm *vm)
{
    sfShape_SetOutlineWidth(getint(vm, 0), getfloat(vm, 1));
}

void Shape_GetOutlineWidth(avm *vm)
{
    word w;
    setf(&w, sfShape_GetOutlineWidth(getint(vm, 0)));
    returnv(vm, &w);
}

void Shape_GetNbPoints(avm *vm)
{
    word w;
    seti(&w, sfShape_GetNbPoints(getint(vm, 0)));
    returnv(vm, &w);
}

void Shape_GetPointPosition(avm *vm)
{
    float X, Y;
    sfShape_GetPointPosition(getint(vm, 0), getint(vm, 1), &X, &Y);
    word w;
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
    long tindex = TOTinsert(vm, off);
    ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
    word index, source;
    sets(&index, newstring(vm, "X"));
    setf(&source, X);
    IA(vm, tbl, tindex, &index, &source);
    sets(&index, newstring(vm, "Y"));
    setf(&source, Y);
    IA(vm, tbl, tindex, &index, &source);
    sett(&w, tindex);
    returnv(vm, &w);
}

void Shape_GetPointColor(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfShape_GetPointColor(getint(vm, 0), getint(vm, 1));
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

void Shape_GetPointOutlineColor(avm *vm)
{
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor col = sfShape_GetPointOutlineColor(getint(vm, 0), getint(vm, 1));
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

Shape_SetPointPosition(avm *vm)
{
    sfShape_SetPointPosition(getint(vm, 0), getint(vm, 1), getfloat(vm, 2), getfloat(vm, 3));
}

void Shape_SetPointColor(avm *vm)
{
    sfColor Color = {getint(vm, 2), getint(vm, 3), getint(vm, 4), getint(vm, 5)};
    sfShape_SetPointColor(getint(vm, 0), getint(vm, 1), Color);
}

void Shape_SetPointOutlineColor(avm *vm)
{
    sfColor Color = {getint(vm, 2), getint(vm, 3), getint(vm, 4), getint(vm, 5)};
    sfShape_SetPointOutlineColor(getint(vm, 0), getint(vm, 1), Color);
}

// Saving functions:
static fptrname shapelib[] =
{
	{Shape_Create, "$Shape_Create"},
	{Shape_CreateLine, "shape_createline"},
	{Shape_CreateRectangle, "shape_createrectangle"},
	{Shape_CreateCircle, "shape_createcircle"},
	{Shape_SetX, "shape_setx"},
	{Shape_SetY, "shape_sety"},
	{Shape_SetPosition, "shape_setposition"},
	{Shape_SetScaleX, "shape_setscalex"},
	{Shape_SetScaleY, "shape_setscaley"},
	{Shape_SetScale, "shape_setscale"},
	{Shape_SetRotation, "shape_setrotation"},
	{Shape_SetCenter, "shape_setcenter"},
	{Shape_SetColor, "shape_setcolor"},
	{Shape_SetBlendMode, "Shape_SetBlendMode"},
	{Shape_GetPosition, "shape_getposition"},
	{Shape_GetScale, "shape_getscale"},
	{Shape_GetRotation, "Shape_GetRotation"},
	{Shape_GetCenter, "Shape_GetCenter"},
	{Shape_GetColor, "Shape_GetColor"},
	{Shape_GetBlendMode, "Shape_GetBlendMode"},
	{Shape_Move, "shape_move"},
	{Shape_Scale, "shape_scale"},
	{Shape_Rotate, "shape_rotate"},
	{Shape_AddPoint, "shape_addpoint"},
	{Shape_EnableFill, "Shape_EnableFill"},
	{Shape_EnableOutline, "Shape_EnableOutline"},
	{Shape_SetOutlineWidth, "shape_setoutlinewidth"},
	{Shape_GetOutlineWidth, "Shape_GetOutlineWidth"},
	{Shape_GetNbPoints, "Shape_GetNbPoints"},
	{Shape_GetPointPosition, "shape_getpointposition"},
	{Shape_GetPointColor, "shape_getpointcolor"},
	{Shape_GetPointOutlineColor, "shape_getpointoutlinecolor"},
	{Shape_SetPointPosition, "shape_setpointposition"},
	{Shape_SetPointColor, "shape_setpointcolor"},
	{Shape_SetPointOutlineColor, "shape_setpointoutlinecolor"}
};


void openshapelib()
{
	int i;
	for (i=0; i<sizeof(shapelib)/sizeof(fptrname); i++)
		zen_regfunc(shapelib[i].ptr, shapelib[i].name);
}
