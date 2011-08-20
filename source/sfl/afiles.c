//***************************************************************************************************
//   Name: afiles.c
//   About: functions which provide files-access.
//   Author: XerXes911. <darklinkcompany@gmail.com>
//   License: please view COPYRIGHT file in the previous folder.
//   Notes: This product uses parts of the iMatix SFL,
//   Copyright © 1991-2000 iMatix Corporation <http://www.imatix.com>
//   Not all functions requires the SFL-lib here, but they are mixed because
//   most of them requires it!
//***************************************************************************************************

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "../zen.h"
#include "../alib.h"
#include "../aerror.h"
#include "../atable.h"
#include "../aexception.h"
#include "../autil.h"

#ifdef ZEN_ENABLE_SFL
#include "types.h"

void File_Open(avm *vm)
{
	word w;
	char* fname = getstring(vm, 0);
	char* fmode = getstring(vm, 1);
	FILE* file;
	if (strlen(fname) == 0)
		throwfnf(vm);
	else
	{
		if (!(file=fopen(fname, fmode)))
			throwfnf(vm);
		seth(&w, (int)file);
		returnv(vm, &w);
	}
}

void File_Locate(avm *vm)
{
    word w;
	FILE* file;
	file = file_locate(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2));
    seth(&w, (int)file);
    returnv(vm, &w);
}

/* Get the file length. */
void cflength(avm* vm)
{
	word w;
	char* fname = getstring(vm, 0);
	seti(&w, flength(fname));
	returnv(vm, &w);
}

void File_Close(avm *vm)
{
    word w;
    FILE* file = gethandle(vm, 0);
    seti(&w, file_close(file));
    return(vm, &w);
}

void File_Read(avm *vm)
{
    word w;
	FILE* file = gethandle(vm, 0);
	long flen = flengthh(file);
	char* buf = (char*)calloc(flen, sizeof(char));	/* validity */
	fread(buf, sizeof(char), flen, file);
	/*setb(&w, (int)buf);*/
	sets(&w, newstring(vm, buf));
	/*addextobj(vm, &w);*/
	free(buf);
	returnv(vm, &w);
}

void File_Write(avm *vm)
{
    word w;
	FILE* file = gethandle(vm, 0);
	char* data = getstring(vm, 1);
	seti(&w, fwrite(data, sizeof(char), strlen(data), file));
	returnv(vm, &w);
}

void File_Concat(avm *vm)
{
    word w;
    seti(&w, file_concat(getstring(vm, 0), getstring(vm, 1)));
    returnv(vm, &w);
}

void File_Rename(avm *vm)
{
    word w;
    seti(&w, file_rename(getstring(vm, 0), getstring(vm, 1)));
    returnv(vm, &w);
}

void File_Delete(avm *vm)
{
    word w;
    seti(&w, file_delete(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_Exists(avm *vm)
{
    word w;
    seti(&w, file_exists(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_Where(avm *vm)
{
    word w;
    sets(&w, newstring(vm, file_where(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2), getstring(vm, 3))));
    returnv(vm, &w);
}

void File_Where_Ext(avm *vm)
{
    word w;
    sets(&w, newstring(vm, file_where_ext(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2), getstring(vm, 3))));
    returnv(vm, &w);
}

void File_Cycle(avm *vm)
{
    word w;
    seti(&w, file_cycle(getstring(vm, 0), getint(vm, 1)))
    returnv(vm, &w);
}

void File_CycleNeeded(avm *vm)
{
    word w;
    seti(&w, file_cycle_needed(getstring(vm, 0), getint(vm, 1)))
    returnv(vm, &w);
}

void File_HasChanged(avm *vm)
{
    word w;
    seti(&w, file_has_changed(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isSafeToExtend(avm *vm)
{/*
    word w;
    //seti(&w, safe_to_extend(getstring(vm, 0)));
    returnv(vm, &w);
*/
}

void File_DefaultExtension(avm *vm)
{
    word w;
    sets(&w, newstring(vm, default_extension(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void File_FixedExtension(avm *vm)
{
    word w;
    sets(&w, newstring(vm, fixed_extension(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void File_StripExtension(avm *vm)
{
    word w;
    sets(&w, newstring(vm, strip_extension(getstring(vm, 0))));
    returnv(vm, &w);
}

void File_AddExtension(avm *vm)
{
    word w;
    sets(&w, newstring(vm, add_extension(getstring(vm, 0), getstring(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void File_StripFilePath(avm *vm)
{
    word w;
    sets(&w, newstring(vm, strip_file_path(getstring(vm, 0))));
    returnv(vm, &w);
}

void File_StripFileName(avm *vm)
{
    word w;
    sets(&w, newstring(vm, strip_file_name(getstring(vm, 0))));
    returnv(vm, &w);
}

void File_GetNewFileName(avm *vm)
{
    word w;
    sets(&w, newstring(vm, get_new_filename(getstring(vm, 0))));
    returnv(vm, &w);
}

void File_isReadable(avm *vm)
{
    word w;
    seti(&w, file_is_readable(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isWritable(avm *vm)
{
    word w;
    seti(&w, file_is_writeable(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isExecutable(avm *vm)
{
    word w;
    seti(&w, file_is_executable(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isProgram(avm *vm)
{
    word w;
    seti(&w, file_is_program(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isDirectory(avm *vm)
{
    word w;
    seti(&w, file_is_directory(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_isLegal(avm *vm)
{
    word w;
    seti(&w, file_is_legal(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_ExecName(avm *vm)
{
    word w;
    sets(&w, newstring(vm, file_exec_name(getstring(vm, 0))));
    returnv(vm, &w);
}

void File_GetSize(avm *vm)
{
    word w;
    seti(&w, get_file_size(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_GetTime(avm *vm)
{
    word w;
    seti(&w, get_file_time(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_GetLines(avm *vm)
{
    word w;
    seti(&w, get_file_lines(getstring(vm, 0)));
    returnv(vm, &w);
}

void File_Slurp(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word w, index, source;
	long ind = 0;
	DESCR * TDESCR = file_slurp(getstring(vm, 0));

	sets(&index, newstring(vm, "Size"));
    seti(&source,  TDESCR->size);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Data"));
    sets(&source,  newstring(vm, TDESCR->data));
	IA(vm, tbl, tindex, &index, &source);

	sett(&w, tindex);
    returnv(vm, &w);
}

void File_Slurpl(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word w, index, source;
	long ind = 0;
	DESCR * TDESCR = file_slurpl(getstring(vm, 0));

	sets(&index, newstring(vm, "Size"));
    seti(&source,  TDESCR->size);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Data"));
    sets(&source,  newstring(vm, TDESCR->data));
	IA(vm, tbl, tindex, &index, &source);

	sett(&w, tindex);
    returnv(vm, &w);
}

void File_SetEndOfLine(avm *vm)
{
    word w;
    seti(&w, file_set_eoln(getstring(vm, 0), getstring(vm, 1), getint(vm, 2), getint(vm, 3)));
    returnv(vm, &w);
}

void File_GetTempName(avm *vm)
{
    word w;
    sets(&w, newstring(vm, get_tmp_file_name(getstring(vm, 0), getint(vm, 1), getstring(vm, 2))));
    returnv(vm, &w);
}

void File_fhRedirect(avm *vm)
{
    word w;
    seti(&w, file_fhredirect(getint(vm, 0), getint(vm, 1)));
    returnv(vm, &w);
}

void File_fhRestore(avm *vm)
{
    file_fhrestore(getint(vm, 0), getint(vm, 1));
}

void TempFile_Open(avm *vm)
{
    FILE* file=ftmp_open(getstring(vm, 0));
    word w;
    seth(&w, (int)file);
	returnv(vm, &w);
}

void TempFile_Close(avm *vm)
{
    ftmp_close(gethandle(vm, 0));
}

void File_CloseAll(avm *vm)
{
    #ifndef  __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
       word w;
       seti(&w, fcloseall());
	   returnv(vm, &w);
    #endif
}

static fptrname libfile[] =
{
    {File_Open, "File_Open"},
    {File_Locate, "File_Locate"},
    {cflength, "File_GetLength"},
    {File_Close, "File_Close"},
    {File_Read, "File_Get"},
    {File_Write, "File_Put"},
    {File_Concat, "File_Concat"},
    {File_Rename, "File_Rename"},
    {File_Delete, "File_Delete"},
    {File_Exists, "File_Exists"},
    {File_Where, "File_Where"},
    {File_Where_Ext, "File_Where_Ext"},
    {File_Cycle, "File_Cycle"},
    {File_CycleNeeded, "File_CycleNeeded"},
    {File_HasChanged, "File_HasChanged"},
    {File_isSafeToExtend, "File_isSafeToExtend"},
    {File_DefaultExtension, "File_DefaultExtension"},
    {File_FixedExtension, "File_FixedExtension"},
    {File_StripExtension, "File_StripExtension"},
    {File_StripFilePath, "File_StripFilePath"},
    {File_StripFileName, "File_StripFileName"},
    {File_GetNewFileName, "File_GetNewFileName"},
    {File_isReadable, "File_isReadable"},
    {File_isWritable ,"File_isWritable"},
    {File_isExecutable, "File_isExecutable"},
    {File_isProgram, "File_isProgram"},
    {File_isDirectory, "File_isDirectory"},
    {File_isLegal, "File_isLegal"},
    {File_ExecName, "File_ExecName"},
    {File_GetSize, "File_GetSize"},
    {File_GetTime, "File_GetTime"},
    {File_GetLines, "File_GetLines"},
    {File_Slurp, "File_Slurp"},
    {File_Slurpl, "File_Slurpl"},
    {File_GetTempName, "File_GetTempName"},
    {File_fhRedirect, "File_fhRedirect"},
    {File_fhRestore, "File_fhRestore"},
    {File_SetEndOfLine, "File_SetEndOfLine"},
    {TempFile_Open, "TempFile_Open"},
    {TempFile_Close, "TempFile_Close"},
    {File_CloseAll, "File_CloseAll"},

};

void zen_openlibfile()
{
	int i;
	for (i=0; i<sizeof(libfile)/sizeof(fptrname); i++)
		zen_regfunc(libfile[i].ptr, libfile[i].name);
}
#endif
