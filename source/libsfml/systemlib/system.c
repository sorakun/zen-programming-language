
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

#include <SFML/Config.h>
#include <SFML/System.h>

/********************************************************************
*                       Random        Functions                     *
********************************************************************/

void Random_SetSeed(avm *vm)
{
    sfRandom_SetSeed(getint(vm, 0));
}

void Random_GetSeed(avm *vm)
{
    word w;
    seti(&w, sfRandom_GetSeed());
    returnv(vm, &w);
}

void Random_Float(avm *vm)
{
    word w;
    setf(&w, sfRandom_Float(getfloat(vm, 0), getfloat(vm, 1)));
    returnv(vm, &w);
}

void Random_Int(avm *vm)
{
    word w;
    seti(&w, sfRandom_Int(getint(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

/********************************************************************
*                      Clock          Functions                     *
********************************************************************/

void Clock_Create(avm *vm)
{
    word w;
    seti(&w, sfClock_Create());
    returnv(vm, &w);
}

void Clock_Destroy(avm *vm)
{
    sfClock_Destroy(getint(vm, 0));
}

void Clock_GetTime(avm *vm)
{
    word w;
    setf(&w, sfClock_GetTime(getint(vm, 0)));
    returnv(vm, &w);
}

void Clock_Reset(avm *vm)
{
    sfClock_Reset(getint(vm, 0));
}

/********************************************************************
*                       OS            Functions                     *
********************************************************************/

void Get_OS(avm *vm)
{
    word w;
    #ifdef CSFML_SYSTEM_WINDOWS
       sets(&w, newstring(vm, "Windows"));
    #elifdef CSFML_SYSTEM_LINUX
       sets(&w, newstring(vm, "Linux"));
    #elifdef CSFML_SYSTEM_MACOS
       sets(&w, newstring(vm, "MacOS"));
    #elifdef CSFML_SYSTEM_FREEBSD
       sets(&w, newstring(vm, "FreeBSD"));
    #endif
    returnv(vm, &w);
}

void Application_Sleep(avm *vm)
{
    sfSleep(getfloat(vm, 0));
}

static fptrname systemlib[] =
{
    {Get_OS, "Get_OS"},
    {Application_Sleep, "Sleep"},

    {Clock_Create, "Clock_Create"},
    {Clock_Destroy, "Clock_Destroy"},
    {Clock_GetTime, "Clock_GetTime"},
    {Clock_Reset, "Clock_Reset"},

    {Random_SetSeed, "Random_SetSeed"},
    {Random_GetSeed, "Random_GetSeed"},
    {Random_Float, "Random_Float"},
    {Random_Int, "Random_Int"},
};

void opensystemlib()
{
	int i;
	for (i=0; i<sizeof(systemlib)/sizeof(fptrname); i++)
		zen_regfunc(systemlib[i].ptr, systemlib[i].name);
}

