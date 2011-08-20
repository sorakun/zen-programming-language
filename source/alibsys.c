/*
 * System library.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "zen.h"
#include "alib.h"
#include "atable.h"

/*
	The time function returns the number of seconds elapsed since midnight
	(00:00:00), January 1, 1970, coordinated universal time, according to
	the system clock.
*/
void cctime(avm* vm)
{
	time_t ltime;
	word w;
	time(&ltime);
	seti(&w, (int)ltime);
	returnv(vm, &w);
}

void ___file___(avm* vm)
{
    /*
    word w;
    sets(&w, newstring(vm, f));
    returnv(vm, &w);
    */
}

void csystem(avm* vm)
{
	word w={0};
	char* str = getstring(vm, 0);
	if (strcmp(str, ""))
		seti(&w, system((const char*)str));
	returnv(vm, &w);
}

void cpackf(avm* vm)
{
	word w={0};
	long toff = (long)vm->tot.table[getarg(vm, 0)->entity.ival].offset;
	ctable* table = (ctable*)getdata(vm->hp.heap, toff);
	seti(&w, (int)packf(vm, table));
	returnv(vm, &w);
}

void System_GetSettings(avm* vm)
{
    word w;
    // this one will be ported to Catlib, once we find how to do it in the windows platform!
    #ifndef  __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
       #include <sys/utsname.h>
       long off = newtable(vm, ZEN_INITIALTABLESIZE);
       long tindex = TOTinsert(vm, off);
       ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
       word index, source;
       struct utsname name;
       if(uname(&name)) exit(-1);
       sets(&index, newstring(vm, "name"));
       sets(&source, newstring(vm, name.sysname));
       IA(vm, tbl, tindex, &index, &source);
       sets(&index, newstring(vm, "nodename"));
       sets(&source, newstring(vm, name.nodename));
       IA(vm, tbl, tindex, &index, &source);
       sets(&index, newstring(vm, "release"));
       sets(&source, newstring(vm, name.release));
       IA(vm, tbl, tindex, &index, &source);
       sets(&index, newstring(vm, "version"));
       sets(&source, newstring(vm, name.version));
       IA(vm, tbl, tindex, &index, &source);
       sets(&index, newstring(vm, "machine"));
        sets(&source, newstring(vm, name.machine));
       IA(vm, tbl, tindex, &index, &source);
       sett(&w, tindex);
    #endif

    returnv(vm, &w);
}

void RunString(avm* vm)
{
    word w;
    seti(&w, zen_runstring(getstring(vm, 0), getstring(vm, 1)));
    returnv(vm, &w);
}

// Message-file access functions.
#ifdef ZEN_ENABLE_SFL
void MessageFile_Open(avm *vm)
{
    word w;
    seti(&w, open_message_file(getstring(vm, 0)));
    returnv(vm, &w);
}

void MessageFile_Close(avm *vm)
{
    close_message_file();
}

void Message_Print(avm *vm)
{
    print_message(getint(vm, 0));
}

void Message_GetText(avm *vm)
{
    word w;
    sets(&w, newstring(vm, message_text(getint(vm, 0))));
    returnv(vm, &w);
}
#endif

void Parse(avm *vm)
{
    char* buf;
    word w;
    parse(getstring(vm, 0), buf, vm->ctx);
    sets(&w, newstring(vm, buf));
    returnv(vm, &w);
}

static fptrname libsys[] =
{
	{cctime, "System_GetTime"},
	{csystem, "system"},
	{System_GetSettings, "System_GetSettings"},
	{cpackf, "packf"},
	{RunString, "runstring"},
	#ifdef ZEN_ENABLE_SFL
	{MessageFile_Open, "MessageFile_Open"},
	{MessageFile_Close, "MessageFile_Close"},
	{Message_Print, "Message_Print"},
	{Message_GetText, "Message_GetText"},
	#endif
	{Parse, "parse"},
};

void zen_openlibsys()
{
	int i;
	for (i=0; i<sizeof(libsys)/sizeof(fptrname); i++)
		zen_regfunc(libsys[i].ptr, libsys[i].name);
}
