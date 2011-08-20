/*
 * Dynamic loading of modules for Windows.
 * See Copyright Notice in zen.h.
*/

#include <windows.h>
#include "zen.h"
#include "avm.h"
#include "aexception.h"
#include "aerror.h"

#ifdef ZEN_ENABLE_DYNAMIC_LOADING

/* Open a dynamic linking library. */
void wdlopen(avm* vm)
{
	word w;
	char* lib = getstring(vm, 0);
	HMODULE mod = LoadLibrary((LPCTSTR)lib);
	if (!mod)
		setvmerror(vm, ZEN_VM_CANNOT_LOAD_LIB);
	seti(&w, (int)mod);
	returnv(vm, &w);
}

/* Find a function in a module by the function's name. */
void wdlfind(avm* vm)
{
	word w;
	HMODULE mod = (HMODULE)getint(vm, 0);
	char* func = getstring(vm, 1);
	userfunc pfunc = (userfunc)GetProcAddress(mod, func);
	if (!pfunc)
	{
		FreeLibrary(mod);
		setvmerror(vm, ZEN_VM_CANNOT_FIND_FUNC);
	}
	settypew(&w, THFUNCTION);
	w.entity.ival = (int)pfunc;
	returnv(vm, &w);
}

/* Derefer a dynamic linking library. */
void wdlclose(avm* vm)
{
	HMODULE mod = (HMODULE)getint(vm, 0);
	if (mod) FreeLibrary(mod);
}

void zen_opendllib()
{
	zen_regfunc(wdlopen, "dlopen");
	zen_regfunc(wdlfind, "dlfind");
	zen_regfunc(wdlclose, "dlclose");
}

#endif
