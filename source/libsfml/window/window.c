/*
  Window functions.
*/

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
#include <SFML/Graphics.h>

/********************************************************************
*                        Window       Functions                     *
********************************************************************/

void window_create(avm* vm)
{
     sfVideoMode Mode = {getint(vm,0), getint(vm,1), getint(vm,2)};
     sfWindowSettings Settings = {getint(vm,5), getint(vm,6), getint(vm,7)};
     /* Create the main window */
  	 word wr = {0};
	 seti(&wr, sfWindow_Create(Mode, getstring(vm, 3), getint(vm,4), Settings));
     returnv(vm, &wr);
}

void window_getinput(avm *vm)
{
    word w;
    seti(&w, sfWindow_GetInput(getint(vm, 0)));
    returnv(vm, &w);
}

void window_createfromhandle(avm *vm)
{
    word wr;
    sfWindowSettings Settings = {getint(vm,1), getint(vm,2), getint(vm,3)};
    seti(&wr, sfWindow_CreateFromHandle(gethandle(vm, 0), Settings));
    returnv(vm ,&wr);
}
void window_destroy(avm* vm)
{
    sfWindow_Destroy(getint(vm, 0));
}

void window_close(avm* vm)
{
    sfWindow_Close(getint(vm, 0));
}

void window_is_opened(avm* vm)
{
    word wr = {0};
    seti(&wr, sfWindow_IsOpened(getint(vm, 0)));
    returnv(vm, &wr);
}

void window_getwidth(avm* vm)
{
    word wr = {0};
    seti(&wr, sfWindow_GetWidth(getint(vm, 0)));
    returnv(vm, &wr);
}

void window_getheight(avm* vm)
{
    word wr = {0};
    seti(&wr, sfWindow_GetHeight(getint(vm, 0)));
    returnv(vm, &wr);
}

void window_getsettings(avm* vm)
{
     sfWindowSettings Settings = sfWindow_GetSettings(getint(vm, 0));
     word w;
     long off = newtable(vm, ZEN_INITIALTABLESIZE);
     long tindex = TOTinsert(vm, off);
     ctable* tbl = (ctable*)getdata(vm->hp.heap, off);
     word index, source;
     sfColor Col = sfImage_GetPixel(getint(vm, 0), getint(vm, 1), getint(vm, 2));
     sets(&index, 0);
     seti(&source, Settings.DepthBits);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, 1);
     seti(&source, Settings.StencilBits);
     IA(vm, tbl, tindex, &index, &source);
     sets(&index, 2);
     seti(&source, Settings.AntialiasingLevel);
     IA(vm, tbl, tindex, &index, &source);
     sett(&w, tindex);
     returnv(vm, &w);
}

void window_geteventtype(avm *vm)
{
    word wr;
    sfEvent Event;
    sfWindow_GetEvent(getint(vm, 0), &Event);
    int b = 0;

    if (Event.Type == sfEvtClosed)
       b = 1;
    else if (Event.Type == sfEvtResized)
       b = 2;
    else if (Event.Type == sfEvtLostFocus)
       b = 3;
    else if (Event.Type == sfEvtGainedFocus)
       b = 4;
    else if (Event.Type == sfEvtTextEntered)
       b = 5;
    else if (Event.Type == sfEvtKeyPressed)
       b = 6;
    else if (Event.Type == sfEvtKeyReleased)
       b = 7;
    else if (Event.Type == sfEvtMouseWheelMoved)
       b = 8;
    else if (Event.Type == sfEvtMouseButtonPressed)
       b = 9;
    else if (Event.Type == sfEvtMouseButtonReleased)
       b = 10;
    else if (Event.Type == sfEvtMouseMoved)
       b = 11;
    else if (Event.Type == sfEvtMouseEntered)
       b = 12;
    else if (Event.Type == sfEvtMouseLeft)
       b = 13;
    else if (Event.Type == sfEvtJoyButtonPressed)
       b = 14;
    else if (Event.Type == sfEvtJoyButtonReleased)
       b = 15;
    else if (Event.Type == sfEvtJoyMoved)
       b = 16;

    seti(&wr, b);
    returnv(vm, &wr);
}

void window_show(avm *vm)
{
    sfWindow_Show(getint(vm, 0), getint(vm, 1));
}

void window_use_vertical_sync(avm *vm)
{
    sfWindow_UseVerticalSync(getint(vm, 0), getint(vm, 1));
}

void window_show_mouse_cursor(avm *vm)
{
     sfWindow_ShowMouseCursor(getint(vm, 0), getint(vm, 1));
}

void window_set_cursor_pos(avm *vm)
{
    sfWindow_SetCursorPosition(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void window_setpos(avm *vm)
{
    sfWindow_SetPosition(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void window_setsize(avm *vm)
{
    sfWindow_SetSize(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void window_display(avm *vm)
{
    sfWindow_Display(getint(vm, 0));
}

void window_enablekeyrepeat(avm *vm)
{
    sfWindow_EnableKeyRepeat(getint(vm, 0), getint(vm, 1));
}

void Window_SetIcon(avm *vm)
{
    sfWindow_SetIcon(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3));
}

void window_setactive(avm *vm)
{
    sfWindow_SetActive(getint(vm, 0), getint(vm, 1));
}

void window_setframeratelimit(avm *vm)
{
    sfWindow_SetFramerateLimit(getint(vm, 0), getint(vm, 1));
}

void window_getframetime(avm *vm)
{
    word wr;
    setf(&wr,  sfWindow_GetFrameTime(getint(vm, 0)));
    returnv(vm, &wr);
}

void window_setjoystickthreshold(avm *vm)
{
    sfWindow_SetJoystickThreshold(getint(vm, 0), getfloat(vm, 1));
}

// Saving functions:
static fptrname windowlib[] =
{
	{window_create, "Win_create"},
	{window_getinput, "Window_getinput"},
	{window_createfromhandle, "Win_createfromhandle"},
	{window_destroy, "Window_Destroy"},
	{window_close, "Window_Close"},
	{window_is_opened, "Window_IsOpened"},
	{window_getwidth, "Window_GetWidth"},
	{window_getheight, "Window_GetHeight"},
	{window_getsettings, "Window_getSettings"},
	{window_geteventtype, "Window_GetEventType"},
	{window_show, "Window_Show"},
	{window_display, "Window_Display"},
	{window_use_vertical_sync, "Window_UseVerticalSync"},
	{window_show_mouse_cursor, "Window_ShowMouseCursor"},
	{window_set_cursor_pos, "Window_SetCursorPos"},
	{window_setpos, "Window_SetPos"},
	{window_setsize, "Window_SetSize"},
	{window_show, "Window_Show"},
	{window_enablekeyrepeat, "Window_EnableKeyRepeat"},
	{window_setactive, "Window_SetActive"},
	{window_setframeratelimit, "Window_SetFrameRateLimit"},
	{window_getframetime, "Window_GetFrameTime"},
	{window_setjoystickthreshold, "Window_SetJoystickThresHold"},
};

void openwindowlib()
{
	int i;
	for (i=0; i<sizeof(windowlib)/sizeof(fptrname); i++)
		zen_regfunc(windowlib[i].ptr, windowlib[i].name);
}
