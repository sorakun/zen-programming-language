//***************************************************************************************************
//   Name: adate.c
//   About: functions which provide simple date & time controls.
//   Author: XerXes911. <darklinkcompany@gmail.com>
//   License: please view COPYRIGHT file in the previous folder.
//   Notes: This product uses parts of the iMatix SFL,
//   Copyright © 1991-2000 iMatix Corporation <http://www.imatix.com>
//***************************************************************************************************

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../zen.h"
#include "../atable.h"
#include "../aexception.h"
#include "../autil.h"
#include "../agc.h"

#ifdef ZEN_ENABLE_SFL
#define MAX_BUF 50
//***************************************************************************************************
//   Date and Time  Functions
//***************************************************************************************************

void Date_Get(avm *vm)
{
    word w;
    seti(&w, date_now());
    returnv(vm, &w);
}

void Time_Get(avm *vm)
{
    word w;
    seti(&w, time_now());
    returnv(vm, &w);
}

void Year_isLeap(avm *vm)
{
    word w;
    seti(&w, leap_year(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_GetJulianDate(avm *vm)
{
    word w;
    seti(&w, julian_date(getint(vm, 0)));
    returnv(vm, &w);
}

void Day_GetDayOfWeek(avm *vm)
{
    word w;
    seti(&w, getint(vm, 0));
    returnv(vm, &w);
}

void Week_GetWeekOfYear(avm *vm)
{
    word w;
    seti(&w, week_of_year(getint(vm, 0)));
    returnv(vm, &w);
}

void Year_GetYearQuarter(avm *vm)
{
    word w;
    seti(&w, year_quarter(getint(vm, 0)));
    returnv(vm, &w);
}

void WeekDay_GetNext(avm *vm)
{
    word w;
    seti(&w, next_weekday(getint(vm, 0)));
    returnv(vm, &w);
}

void WeekDay_GetPrev(avm *vm)
{
    word w;
    seti(&w, prev_weekday(getint(vm, 0)));
    returnv(vm, &w);
}

void Century_GetDefault(avm *vm)
{
    word w;
    seti(&w, default_century(getint(vm, 0)));
    returnv(vm, &w);
}

void DateToDays(avm *vm)
{
    word w;
    seti(&w, date_to_days(getint(vm, 0)));
    returnv(vm, &w);
}

void DaysToDate(avm *vm)
{
    word w;
    seti(&w, days_to_date(getint(vm, 0)));
    returnv(vm, &w);
}

void DateToTimer(avm *vm)
{
    word w;
    seti(&w, date_to_timer(getint(vm, 0)));
    returnv(vm, &w);
}

void TimerToDate(avm *vm)
{
    word w;
    seti(&w, timer_to_date(getint(vm, 0)));
    returnv(vm, &w);
}

void TimerToTime(avm *vm)
{
    word w;
    seti(&w, timer_to_date(getint(vm, 0)));
    returnv(vm, &w);
}

void TimerToGMTDate(avm *vm)
{
    word w;
    seti(&w, timer_to_gmdate(getint(vm, 0)));
    returnv(vm, &w);
}

void TimerToGMTTime(avm *vm)
{
    word w;
    seti(&w, timer_to_gmtime(getint(vm, 0)));
    returnv(vm, &w);
}

void TimeToCSecs(avm *vm)
{
    word w;
    seti(&w, time_to_csecs(getint(vm, 0)));
    returnv(vm, &w);
}

void CSecsToTime(avm *vm)
{
    word w;
    seti(&w, csecs_to_time(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_GetFutureDate(avm *vm)
{
    word w;
    int x = getint(vm, 0);
    int y = getint(vm, 1);
    future_date(&x, &y, getint(vm, 2), getint(vm, 3));
    seti(&w, x);
    returnv(vm, &w);
}

void Time_GetFutureTime(avm *vm)
{
    word w;
    int x = getint(vm, 0);
    int y = getint(vm, 1);
    future_date(&x, &y, getint(vm, 2), getint(vm, 3));
    seti(&w, y);
    returnv(vm, &w);
}

void Date_GetPastDate(avm *vm)
{
    word w;
    int x = getint(vm, 0);
    int y = getint(vm, 1);
    past_date(&x, &y, getint(vm, 2), getint(vm, 3));
    seti(&w,x);
    returnv(vm, &w);
}

void Time_GetPastTime(avm *vm)
{
    word w;
    int x = getint(vm, 0);
    int y = getint(vm, 1);
    past_date(&x, &y, getint(vm, 2), getint(vm, 3));
    seti(&w,y);
    returnv(vm, &w);
}

void Date_GetDiffDate(avm *vm)
{
    word w;
    int x = getint(vm, 4);
    int y = getint(vm, 5);
    past_date(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3), &x, &y);
    seti(&w,x);
    returnv(vm, &w);
}

void Time_GetDiffTime(avm *vm)
{
    word w;
    int x = getint(vm, 4);
    int y = getint(vm, 5);
    past_date(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3), &x, &y);
    seti(&w,y);
    returnv(vm, &w);
}

void Date_isInvalid(avm *vm)
{
    word w;
    seti(&w, valid_date(getint(vm, 0)));
    returnv(vm, &w);
}

void Time_isInvalid(avm *vm)
{
    word w;
    seti(&w, valid_time(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_isFuture(avm *vm)
{
    word w;
    seti(&w, date_is_future(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_isPast(avm *vm)
{
    word w;
    seti(&w, date_is_past(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_Pack(avm *vm)
{
    word w;
    seti(&w, pack_date(getint(vm, 0)));
    returnv(vm, &w);
}

void Time_Pack(avm *vm)
{
    word w;
    seti(&w, pack_time(getint(vm, 0)));
    returnv(vm, &w);
}

void Date_Unpack(avm *vm)
{
    word w;
    seti(&w, unpack_date(getint(vm, 0)));
    returnv(vm, &w);
}

void Time_Unpack(avm *vm)
{
    word w;
    seti(&w, unpack_time(getint(vm, 0)));
    returnv(vm, &w);
}

void Time_GetTimeZone(avm *vm)
{
    word w;
    sets(&w, newstring(vm, timezone_string()));
    returnv(vm, &w);
}

void Time_LocalToGMT(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word w, index, source;
	long ind = 0;
	long* gmt_date, gmt_time;
	local_to_gmt(getint(vm, 0), getint(vm, 1), &gmt_date, &gmt_time);

    // Date
    sets(&index, newstring(vm, "GMTDate"));
    seti(&source,  gmt_date);
	IA(vm, tbl, tindex, &index, &source);

	// Time
    sets(&index, newstring(vm, "GMTTime"));
    seti(&source,  gmt_time);
	IA(vm, tbl, tindex, &index, &source);

	sett(&w, tindex);
    returnv(vm, &w);
}

void Time_GTMToLocal(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word w, index, source;
	long ind = 0;
	long* gmt_date, gmt_time;
	gmt_to_local(getint(vm, 0), getint(vm, 1), &gmt_date, &gmt_time);

    // Date
    sets(&index, newstring(vm, "GMTDate"));
    seti(&source,  gmt_date);
	IA(vm, tbl, tindex, &index, &source);

	// Time
    sets(&index, newstring(vm, "GMTTime"));
    seti(&source,  gmt_time);
	IA(vm, tbl, tindex, &index, &source);

	sett(&w, tindex);
    returnv(vm, &w);
}

void Time_GetLocal(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word DATE, index, source;
	long ind = 0;

    char buf[MAX_BUF];
    time_t seconds = time(NULL);
    struct tm *now = safe_localtime(&seconds);

    // Date
    sets(&index, newstring(vm, "Year"));
    seti(&source,  now->tm_year + 1900);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Month"));
    seti(&source,  now->tm_mon + 1);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Day"));
    seti(&source,  now->tm_mday + 1);
	IA(vm, tbl, tindex, &index, &source);

    // Time
    sets(&index, newstring(vm, "Hour"));
    seti(&source,  now->tm_hour);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Min"));
    seti(&source,  now->tm_min);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Sec"));
    seti(&source,  now->tm_sec);
	IA(vm, tbl, tindex, &index, &source);

    sett(&DATE, tindex);
    returnv(vm, &DATE);
}

void Time_GetGMT(avm *vm)
{
    long off = newtable(vm, ZEN_INITIALTABLESIZE);
	long tindex = TOTinsert(vm, off);
	ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
	word DATE, index, source;
	long ind = 0;

    char buf[MAX_BUF];
    time_t seconds = time(NULL);
    struct tm *now = safe_gmtime(&seconds);

    // Date
    sets(&index, newstring(vm, "Year"));
    seti(&source,  now->tm_year + 1900);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Month"));
    seti(&source,  now->tm_mon + 1);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Day"));
    seti(&source,  now->tm_mday + 1);
	IA(vm, tbl, tindex, &index, &source);

    // Time
    sets(&index, newstring(vm, "Hour"));
    seti(&source,  now->tm_hour);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Min"));
    seti(&source,  now->tm_min);
	IA(vm, tbl, tindex, &index, &source);

	sets(&index, newstring(vm, "Sec"));
    seti(&source,  now->tm_sec);
	IA(vm, tbl, tindex, &index, &source);

    sett(&DATE, tindex);
    returnv(vm, &DATE);
}


static fptrname libdate[] =
{
    //   Date and Time  Functions
    {Date_Get, "Date_Get"},
    {Time_Get, "Time_Get"},
    {Year_isLeap, "Year_isLeap"},
    {Date_GetJulianDate, "Date_GetJulianDate"},
    {Day_GetDayOfWeek, "Day_GetDayOfWeek"},
    {Week_GetWeekOfYear, "Week_GetWeekOfYear"},
    {Year_GetYearQuarter, "Year_GetYearQuarter"},
    {WeekDay_GetNext, "WeekDay_GetNext"},
    {WeekDay_GetPrev, "WeekDay_GetPrev"},
    {Century_GetDefault, "Century_GetDefault"},
    {DateToDays, "DateToDays"},
    {DaysToDate, "DaysToDate"},
    {DateToTimer, "DateToTimer"},
    {TimerToDate, "TimerToDate"},
    {TimerToTime, "TimerToTime"},
    {TimerToGMTDate, "TimerToGMTDate"},
    {TimerToGMTTime, "TimerToGMTTime"},
    {TimeToCSecs, "TimeToCSecs"},
    {CSecsToTime, "CSecsToTime"},
    {Date_GetFutureDate, "Date_GetFutureDate"},
    {Time_GetFutureTime, "Time_GetFutureTime"},
    {Date_GetPastDate, "Date_GetPastDate"},
    {Time_GetPastTime, "Time_GetPastTime"},
    {Date_GetDiffDate, "Date_GetDiffDate"},
    {Time_GetDiffTime, "Time_GetDiffTime"},
    {Date_isInvalid, "Date_isInvalid"},
    {Time_isInvalid, "Time_isInvalid"},
    {Date_isFuture, "Date_isFuture"},
    {Date_isPast, "Date_isPast"},
    {Date_Pack, "Date_Pack"},
    {Time_Pack, "Time_Pack"},
    {Date_Unpack, "Date_Unpack"},
    {Time_Unpack, "Time_Unpack"},
    {Time_GetTimeZone, "Time_GetTimeZone"},
    {Time_LocalToGMT, "Time_LocalToGMT"},
    {Time_GTMToLocal, "Time_GTMToLocal"},
    {Time_GetLocal, "Time_GetLocal"},
    {Time_GetGMT, "Time_GetGMT"},

};

void zen_opensfllibdate()
{
	int i;
	for (i=0; i<sizeof(libdate)/sizeof(fptrname); i++)
		zen_regfunc(libdate[i].ptr, libdate[i].name);
}
#endif
