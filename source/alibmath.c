/*
 * Math library.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include <math.h>
#include "zen.h"
#include "alib.h"
#include "aerror.h"

/* Convert a number to float if it's an integer. Also check argument type. */
float convfloat(avm* vm, word* w)
{
	char type = gettypew(w);
	if (type!=TINTEGER && type!=TFLOAT) setvmerror(vm, ZEN_VM_INVALID_ARGUMENT);
	return type==TFLOAT?(w)->entity.fval:(float)(w)->entity.ival;
}

void cmsin(avm* vm)
{
	word wr;
	setf(&wr, (float)sin(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmasin(avm* vm)
{
	word wr;
	setf(&wr, (float)asin(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmcos(avm* vm)
{
	word wr;
	setf(&wr, (float)cos(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmacos(avm* vm)
{
	word wr;
	setf(&wr, (float)acos(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmtan(avm* vm)
{
	word wr;
	setf(&wr, (float)tan(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmatan(avm* vm)
{
	word wr;
	setf(&wr, (float)atan(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmceil(avm* vm)
{
	word wr;
	setf(&wr, (float)ceil(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmfloor(avm* vm)
{
	word wr;
	setf(&wr, (float)floor(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmexp(avm* vm)
{
	word wr;
	setf(&wr, (float)exp(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmpow(avm* vm)
{
	word wr;
	setf(&wr, (float)pow(convfloat(vm, getarg(vm, 0)),
		convfloat(vm, getarg(vm, 1))));
	returnv(vm, &wr);
}

void cmsqrt(avm* vm)
{
	word wr;
	setf(&wr, (float)sqrt(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmlog(avm* vm)
{
	word wr;
	setf(&wr, (float)log(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmlog10(avm* vm)
{
	word wr;
	setf(&wr, (float)log10(convfloat(vm, getarg(vm, 0))));
	returnv(vm, &wr);
}

void cmsrand(avm* vm)
{
	srand((uint)getarg(vm, 0)->entity.ival);
}

/* Generate a pseudorandom number in the range 0 to 1. */
void cmrand(avm* vm)
{
	word wr;
	setf(&wr, (float)rand()/RAND_MAX);
	returnv(vm, &wr);
}

/* Refer to floating-point routine library of C for homonymous functions. */
static fptrname libmath[] =
{
	{cmsin, "sin"},
	{cmasin, "asin"},
	{cmcos, "cos"},
	{cmacos, "acos"},
	{cmtan, "tan"},
	{cmatan, "atan"},
	{cmceil, "ceil"},
	{cmfloor, "floor"},
	{cmexp, "exp"},
	{cmpow, "pow"},
	{cmsqrt, "sqrt"},
	{cmlog, "log"},
	{cmlog10, "log10"},
	{cmsrand, "srand"},
	{cmrand, "rand"}
};

/* Register all the math functions. */
void zen_openlibmath()
{
	int i;
	for (i=0; i<sizeof(libmath)/sizeof(fptrname); i++)
		zen_regfunc(libmath[i].ptr, libmath[i].name);
}
