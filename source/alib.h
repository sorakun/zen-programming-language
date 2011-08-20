/* See Copyright Notice in azure.h. */

#ifndef _ALIB_H_
#define _ALIB_H_

#include "avm.h"

typedef struct _fptrname
{
	userfunc ptr;
	char* name;
} fptrname;

typedef struct _userlib
{
	fptrname* lib;
	int size, nextslot;
} userlib;

#endif
