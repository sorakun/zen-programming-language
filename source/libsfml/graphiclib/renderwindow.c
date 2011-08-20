/*
  RenderWindow functions.
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
*                       Render Window Functions                     *
********************************************************************/

void renderwindow_create(avm* vm)
{
     sfVideoMode Mode = {getint(vm,0), getint(vm,1), getint(vm,2)};
     sfWindowSettings Settings = {getint(vm,5), getint(vm,6), getint(vm,7)};
     /* Create the main renderwindow */
  	 word wr = {0};
	 seti(&wr, sfRenderWindow_Create(Mode, getstring(vm, 3), getint(vm,4), Settings));
     returnv(vm, &wr);
}

void renderwindow_getinput(avm *vm)
{
    word w;
    seti(&w, sfRenderWindow_GetInput(getint(vm, 0)));
    returnv(vm, &w);
}

void renderwindow_createfromhandle(avm *vm)
{
    word wr;
    sfWindowSettings Settings = {getint(vm,1), getint(vm,2), getint(vm,3)};
    seti(&wr, sfRenderWindow_CreateFromHandle(gethandle(vm, 0), Settings));
    returnv(vm ,&wr);
}
void renderwindow_destroy(avm* vm)
{
    sfRenderWindow_Destroy(getint(vm, 0));
}

void renderwindow_close(avm* vm)
{
    sfRenderWindow_Close(getint(vm, 0));
}

void renderwindow_is_opened(avm* vm)
{
    word wr = {0};
    seti(&wr, sfRenderWindow_IsOpened(getint(vm, 0)));
    returnv(vm, &wr);
}

void renderwindow_getwidth(avm* vm)
{
    word wr = {0};
    seti(&wr, sfRenderWindow_GetWidth(getint(vm, 0)));
    returnv(vm, &wr);
}

void renderwindow_getheight(avm* vm)
{
    word wr = {0};
    seti(&wr, sfRenderWindow_GetHeight(getint(vm, 0)));
    returnv(vm, &wr);
}

void renderwindow_getsettings(avm* vm)
{
     sfWindowSettings Settings = sfRenderWindow_GetSettings(getint(vm, 0));
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
void renderwindow_geteventtype(avm *vm)
{
    word wr;
    sfEvent Event;
    sfRenderWindow_GetEvent(getint(vm, 0), &Event);
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

void renderwindow_display(avm *vm)
{
    sfRenderWindow_Display(getint(vm, 0));
}

void renderwindow_clr(avm *vm)
{
    sfColor col = {getint(vm, 1), getint(vm, 2), getint(vm, 3), getint(vm, 4)};
    sfRenderWindow_Clear(getint(vm, 0),col );
}

void renderwindow_use_vertical_sync(avm *vm)
{
    sfRenderWindow_UseVerticalSync(getint(vm, 0), getint(vm, 1));
}

void renderwindow_show_mouse_cursor(avm *vm)
{
     sfRenderWindow_ShowMouseCursor(getint(vm, 0), getint(vm, 1));
}

void renderwindow_set_cursor_pos(avm *vm)
{
    sfRenderWindow_SetCursorPosition(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void renderwindow_setpos(avm *vm)
{
    sfRenderWindow_SetPosition(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void renderwindow_setsize(avm *vm)
{
    sfRenderWindow_SetSize(getint(vm, 0), getint(vm, 1),getint(vm, 2));
}

void renderwindow_show(avm *vm)
{
    sfRenderWindow_Show(getint(vm, 0), getint(vm, 1));
}

void renderwindow_enablekeyrepeat(avm *vm)
{
    sfRenderWindow_EnableKeyRepeat(getint(vm, 0), getint(vm, 1));
}

void RenderWindow_SetIcon(avm *vm)
{
    sfRenderWindow_SetIcon(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3));
}

void renderwindow_setactive(avm *vm)
{
    sfRenderWindow_SetActive(getint(vm, 0), getint(vm, 1));
}

void renderwindow_setframeratelimit(avm *vm)
{
    sfRenderWindow_SetFramerateLimit(getint(vm, 0), getint(vm, 1));
}

void renderwindow_getframetime(avm *vm)
{
    word wr;
    setf(&wr,  sfRenderWindow_GetFrameTime(getint(vm, 0)));
    returnv(vm, &wr);
}

void renderwindow_setjoystickthreshold(avm *vm)
{
    sfRenderWindow_SetJoystickThreshold(getint(vm, 0), getfloat(vm, 1));
}

void renderwindow_preserveopenglstates(avm *vm)
{
    sfRenderWindow_PreserveOpenGLStates(getint(vm, 0), getint(vm, 1));
}

void renderwindow_capture(avm *vm)
{
    word w;
    seti(&w, sfRenderWindow_Capture(getint(vm, 0)));
    returnv(vm, &w);
}


void renderwindow_drawpostfx(avm *vm)
{
    sfRenderWindow_DrawPostFX(getint(vm, 0), getint(vm, 1));
}

void renderwindow_drawsprite(avm *vm)
{
    sfRenderWindow_DrawSprite(getint(vm, 0), getint(vm, 1));
}

void renderwindow_drawshape(avm *vm)
{
    sfRenderWindow_DrawShape(getint(vm, 0), getint(vm, 1));
}

void renderwindow_drawstring(avm *vm)
{
    sfRenderWindow_DrawString(getint(vm, 0), getint(vm, 1));
}

void renderwindow_setview(avm *vm)
{
    sfRenderWindow_SetView(getint(vm, 0), getint(vm, 1));
}

void renderwindow_getview(avm *vm)
{
    word w;
    seti(&w, sfRenderWindow_GetView(getint(vm, 0)));
    returnv(vm, &w);
}

void renderwindow_getdefaultview(avm *vm)
{
    word w;
    seti(&w, sfRenderWindow_GetDefaultView(getint(vm, 0)));
    returnv(vm, &w);
}

void renderwindow_convertcoords(avm *vm)
{
    sfRenderWindow_ConvertCoords(getint(vm, 0), getint(vm, 1), getint(vm, 2), getint(vm, 3), getint(vm, 4), getint(vm, 5));
}

// Saving functions:
static fptrname renderwindowlib[] =
{
	{renderwindow_create, "RenderWin_create"},
	{renderwindow_getinput, "RenderWindow_getinput"},
	{renderwindow_createfromhandle, "RenderWin_createfromhandle"},
	{renderwindow_destroy, "RenderWindow_Destroy"},
	{renderwindow_close, "RenderWindow_Close"},
	{renderwindow_is_opened, "RenderWindow_IsOpened"},
	{renderwindow_getwidth, "RenderWindow_GetWidth"},
	{renderwindow_getheight, "RenderWindow_GetHeight"},
	{renderwindow_getsettings, "RenderWindow_getSettings"},
	{renderwindow_geteventtype, "RenderWindow_GetEventType"},
	{renderwindow_display, "RenderWindow_Display"},
	{renderwindow_use_vertical_sync, "RenderWindow_UseVerticalSync"},
	{renderwindow_show_mouse_cursor, "RenderWindow_ShowMouseCursor"},
	{renderwindow_set_cursor_pos, "RenderWindow_SetCursorPos"},
	{renderwindow_setpos, "RenderWindow_SetPos"},
	{renderwindow_setsize, "RenderWindow_SetSize"},
	{renderwindow_show, "RenderWindow_Show"},
	{renderwindow_enablekeyrepeat, "RenderWindow_EnableKeyRepeat"},
	{renderwindow_setactive, "RenderWindow_SetActive"},
	{renderwindow_setframeratelimit, "RenderWindow_SetFrameRateLimit"},
	{renderwindow_getframetime, "RenderWindow_GetFrameTime"},
	{renderwindow_setjoystickthreshold, "RenderWindow_SetJoystickThresHold"},
	{renderwindow_preserveopenglstates, "RenderWindow_PreserveOpenglStates"},
    {renderwindow_clr, "RenderWindow_clr"},
    {renderwindow_capture, "RenderWindow_Capture"},
    {renderwindow_setview,"RenderWindow_SetView"},
    {renderwindow_getview,"RenderWindow_Getview"},
    {renderwindow_getdefaultview,"RenderWindow_GetDefaultView"},
    {renderwindow_convertcoords, "RenderWindow_ConvertCoords"},
	{renderwindow_drawpostfx, "RenderWindow_DrawPostFX"},
	{renderwindow_drawshape, "RenderWindow_DrawShape"},
	{renderwindow_drawsprite, "RenderWindow_DrawSprite"},
	{renderwindow_drawstring, "RenderWindow_DrawString"},
};

void openrenderwindowlib()
{
	int i;
	for (i=0; i<sizeof(renderwindowlib)/sizeof(fptrname); i++)
		zen_regfunc(renderwindowlib[i].ptr, renderwindowlib[i].name);
}
