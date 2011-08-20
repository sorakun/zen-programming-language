//***************************************************************************************************
//   Name: afiles.c
//   About: functions which provide directories manipulation functions.
//   Author: XerXes911. <darklinkcompany@gmail.com>
//   License: please view COPYRIGHT file in the previous folder.
//   Notes: This product uses parts of the iMatix SFL,
//   Copyright © 1991-2000 iMatix Corporation <http://www.imatix.com>
//***************************************************************************************************

#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "../zen.h"
#include "../atable.h"
#include "../aexception.h"
#include "../autil.h"
#include "../agc.h"

#ifdef ZEN_ENABLE_SFL

//***************************************************************************************************
//   Directories Manipulation Functions
//***************************************************************************************************

void Dir_Open(avm* vm)
{
    word w;
	char* dname = getstring(vm, 0);
	DIR* dir;
	if (strlen(dname) == 0)
		throwfnf(vm);
	else
	{
		if (!(dir=opendir(dname)))
			throwfnf(vm);
		setd(&w, (int)dir);
		returnv(vm, &w);
	}
}

void Dir_GetFileDescriptor(avm* vm)
{
    word w;
    //seti(&w, dirfd(getdirectory(vm, 0)))&;
    returnv(vm, &w);
}

void Dir_GetCurrent(avm* vm)
{
    word w;
    sets(&w, newstring(vm, directory_getcurrent()));
    returnv(vm, &w);
}

void Dir_SetCurrent(avm *vm)
{
    word w;
    seti(&w, set_curdir(getstring(vm, 0)));
    returnv(vm, &w);
}

void Dir_Create(avm *vm)
{
    word w;
    seti(&w, make_dir(getstring(vm, 0)));
    returnv(vm, &w);
}

void Dir_Remove(avm *vm)
{
    word w;
    seti(&w, remove_dir(getstring(vm, 0)));
    returnv(vm, &w);
}

void Dir_GetUsage(avm *vm)
{
    word w;
    seti(&w, dir_usage(getstring(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

void Dir_GetFilesNumber(avm *vm)
{
    word w;
    seti(&w, dir_files(getstring(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

//***************************************************************************************************
//   Registering Functions
//***************************************************************************************************

static fptrname libdir[] =
{
    {Dir_Open, "Dir_Open"},
    {Dir_GetFileDescriptor, "Dir_GetFileDescriptor"},
    {Dir_GetCurrent, "Dir_GetCurrent"},
    {Dir_SetCurrent, "Dir_SetCurrent"},
    {Dir_Create, "Dir_Create"},
    {Dir_Remove, "Dir_Remove"},
    {Dir_GetUsage, "Dir_GetUsage"},
    {Dir_GetFilesNumber, "Dir_GetFilesNumber"}
};

void zen_opensfllibdir()
{
	int i;
	for (i=0; i<sizeof(libdir)/sizeof(fptrname); i++)
		zen_regfunc(libdir[i].ptr, libdir[i].name);
}
#endif
