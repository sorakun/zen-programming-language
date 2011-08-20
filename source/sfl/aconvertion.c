//***************************************************************************************************
//   Name: aconvertion.c
//   About: functions which provide simple data-types convertions.
//   Author: XerXes911. <darklinkcompany@gmail.com>
//   License: please view COPYRIGHT file in the previous folder.
//   Notes: This product uses parts of the iMatix SFL,
//   Copyright © 1991-2000 iMatix Corporation <http://www.imatix.com>
//***************************************************************************************************

#include <string.h>
#include <stdlib.h>
#ifdef  __WIN32__ || _WIN64 || _WIN32 || __TOS_WIN__ || __WINDOWS__ || _WIN32_WCE || WIN32_PLATFORM_'P'
    #include <conio.h>
#endif
#include "../zen.h"
#include "../atable.h"
#include "../aexception.h"
#include "../autil.h"
#include "../agc.h"

#ifdef ZEN_ENABLE_SFL

//***************************************************************************************************
//   Conversations functions
//***************************************************************************************************

void BoolToStr(avm *vm)
{
    /*
       Converts a Bool value to a string according to the specified format: 0 = Yes|No; 1 = Y|N,
       2 = True|False, 3 = T|F, 4 = 1|0. Returns a pointer to a static string that is overwritten
       by each call.
    */
    word w;
    sets(&w, newstring(vm, conv_bool_str(getint(vm, 0),getint(vm,1))));
    returnv(vm, &w);
}

void Date_FromFormat(avm *vm)
{
    /*
        Returns the formatted result. This is a static string, of at most 80 characters, that is
        overwritten by each call. If date is zero, returns an empty string. The 'm' and 'd' formats
        output a leading space when used at the start of the picture. This is to improve alignment
        of columns of dates. The 'm' and 'd' formats also output a space when the previous
        character was a digit; otherwise the date components stick together and are illegible.
    */
    word w;
    sets(&w, newstring(vm, conv_date_pict(getint(vm, 0),getstring(vm,1))));
    returnv(vm, &w);
}

void DateToString(avm *vm)
{
    /*
       Converts a date to a string.  The format argument defines how
       the date is shown:
    */
    word w;
    sets(&w, newstring(vm, conv_date_str(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3), getstring(vm, 4),
                                         getint(vm, 5))));
    returnv(vm, &w);
}

void NumberToStr(avm *vm)
{
    /*
       Converts a number to a string. The number format is defined largely
       by the flags argument, which can specify various values
    */
    word w;
    sets(&w, newstring(vm, conv_number_str(getstring(vm, 0), getint(vm, 1), getstring(vm, 2), getint(vm, 3), getint(vm, 4), getint(vm, 6), getint(vm, 6))));
    returnv(vm, &w);
}

void StrToBool(avm *vm)
{
    /*
       Converts a string to a Bool. Accepts T/Y/1 as TRUE, F/N/0 as FALSE,
       ignoring case. Looks only at the first letter of the string. Returns 1
       for TRUE, 0 for FALSE, -1 if the string was not valid.
    */
    word w;
    seti(&w, conv_str_bool(getstring(vm, 0)));
    returnv(vm, &w);
}

void StrToDate(avm *vm)
{
    /*
       Converts a string to a date.
    */
    word w;
    sets(&w, newstring(vm, conv_str_date(getstring(vm, 0), getint(vm, 1),getint(vm, 2),getint(vm, 3))));
    returnv(vm, &w);
}

void StrToDay(avm *vm)
{
    /*
       Converts a string to a day
    */
    word w;
    seti(&w, conv_str_day(getstring(vm, 0)));
    returnv(vm, &w);
}

void StrToNumber(avm *vm)
{
    /*
       Converts a string to a Number.
    */
    word w;
    sets(&w, newstring(vm, conv_number_str(getstring(vm, 0), getint(vm, 1), getstring(vm, 2), getint(vm, 3),
                                           getint(vm, 4), getint(vm, 6))));

    returnv(vm, &w);
}

void StrToTime(avm *vm)
{
    /*
       convert a string to time
    */
    word w;
    sets(&w, newstring(vm, conv_str_time(getstring(vm, 0))));
    returnv(vm, &w);
}

void TimeFromFormat(avm *vm)
{
    /*
       Converts a time to a string using a picture.
    */

    word w;
    sets(&w, newstring(vm, conv_time_pict(getint(vm, 0), getstring(vm, 1))));
    returnv(vm, &w);
}

void TimeToStr(avm *vm)
{
    /*
       Converts a time to a string.
    */
    word w;
    sets(&w, newstring(vm, conv_time_str(getint(vm,0), getint(vm, 1), getstring(vm, 2), getint(vm, 3))));
    returnv(vm, &w);

}

static fptrname libconv[] =
{

    //   conversations functions:
	{BoolToStr, "BoolToStr"},
	{Date_FromFormat, "Date_FromFormat"},
	{DateToString, "DateToString"},
	{NumberToStr, "NumberToStr"},
	{StrToBool, "StrToBool"},
	{StrToDate, "StrToDate"},
	{StrToDay, "StrToDay"},
	{StrToNumber, "StrToNumber"},
	{StrToTime, "StrToTime"},
	{TimeFromFormat, "TimeFromFormat"},
	{TimeToStr, "TimeToStr"}
};

void zen_opensfllibconv()
{
	int i;
	for (i=0; i<sizeof(libconv)/sizeof(fptrname); i++)
		zen_regfunc(libconv[i].ptr, libconv[i].name);
}
#endif
