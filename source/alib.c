/*
 * Library manipulation functions.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include <string.h>
#include "zen.h"
#include "alib.h"

userlib ulib = {0};

/* Register a user-defined function to the library. */
void zen_regfunc(userfunc uf, char* fname)
{
	int newsize;
	fptrname* newlib;
	if (ulib.nextslot>=ulib.size)	/* the array full? */
	{
		newsize = ulib.size*2+1;
		if (!(newlib=calloc(newsize,sizeof(fptrname))))
			return;
		memcpy(newlib, ulib.lib, ulib.size*sizeof(fptrname));
		if (ulib.lib)
			free(ulib.lib);
		ulib.lib = newlib;
		ulib.size = newsize;
	}
	ulib.lib[ulib.nextslot].ptr = uf;
	ulib.lib[ulib.nextslot++].name = fname;
}

/* Get a function's entry by its name. */
userfunc zen_getfunc(char* name)
{
	int i;
	for (i=0; i<ulib.nextslot; i++)
		if (strcmp(ulib.lib[i].name, name) == 0)
			return ulib.lib[i].ptr;
	return 0;
}

void zen_openlibs()
{
	zen_openlibstd();
	zen_openlibio();
	zen_openlibmath();
	zen_openlibstr();
	zen_openlibsys();
	zen_openlibdate();

	// Graphics:
    openrenderwindowlib();
    openimagelib();
    opencolorlib();
    openspritelib();
    openstringlib();
    openshapelib();
    openrectlib();
    // Audio
    openaudiolib();
    // System
    opensystemlib();

#ifdef ZEN_ENABLE_CONCTRL
	zen_openlibconctrl();
#endif

#ifdef ZEN_ENABLE_DYNAMIC_LOADING
	zen_opendllib();
#endif

#ifdef ZEN_ENABLE_SFL
    zen_opensfllibconv();
    zen_opensfllibdate();
    zen_opensfllibdir();
    zen_openlibfile();
#endif
}

void zen_closelibs()
{
	free(ulib.lib);
}
