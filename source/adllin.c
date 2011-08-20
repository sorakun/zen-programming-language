/*
 * Dynamic loading of modules for Linux.
 * See Copyright Notice in zen.h.
*/

#include <dlfcn.h>
#include "zen.h"
#include "avm.h"
#include "alib.h"
#include "aexception.h"
#include "aerror.h"

#ifdef ZEN_ENABLE_DYNAMIC_LOADING

/* Open a dynamic library. */
void ldlopen(avm* vm)
{
	word w;
	char* lib = getstring(vm, 0);
	void* handle = dlopen(lib, RTLD_LAZY);
	if (!handle)
		setvmerror(vm, ZEN_VM_CANNOT_LOAD_LIB);
	seti(&w, (int)handle);
	returnv(vm, &w);
}

/* Find a function in a module by the function's name. */
void ldlfind(avm* vm)
{
	word w;
	void* handle = (void*)getint(vm, 0);
	char* func = getstring(vm, 1);;
	userfunc pfunc = dlsym(handle, func);
	if (!pfunc)
	{
		dlclose(handle);
		setvmerror(vm, ZEN_VM_CANNOT_FIND_FUNC);
	}
	settypew(&w, THFUNCTION);
	w.entity.ival = (int)pfunc;
	returnv(vm, &w);
}

/* Derefer a dynamic library. */
void ldlclose(avm* vm)
{
	void* handle = (void*)getint(vm, 0);
	if (handle) dlclose(handle);
}


/* Get the dll default extention. */
void ldlgetext(avm* vm)
{
    // not finished
    #ifdef  __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
       char ext[4] = "dll";
    #endif
}

void zen_opendllib()
{
	zen_regfunc(ldlopen, "DLL_Open");
	zen_regfunc(ldlfind, "DLL_Find");
	zen_regfunc(ldlclose, "DLL_Close");
}

#endif
