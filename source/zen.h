/******************************************************************************
* Copyright (C) 2005 Gangcai Lin
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the follozen conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

#ifndef _AZURE_H_
#define _AZURE_H_

/*
 * To build a suitable profile of the language to meet specific requirements,
 * define/undefine the corresponding compiling directives as needed:
 *
 * ZEN_ENABLE_DYNAMIC_LOADING: enable dynamic module loading functionality;
 * ZEN_ENABLE_WARNING: print warning messages;
 * ZEN_ENABLE_TRAP: enable instruction interrupt handler functionality;
 * ZEN_ENABLE_OPTIMIZATION: enable optimizations;
 * ZEN_MAC_OSX: define this when compiled under Mac OSX;
 * ZEN_CPLUSPLUS: define this when compiled along with a C++ project;
 * ZEN_ENABLE_CONCTRL: Enable newt bindings for Zen.
 * ZEN_ENABLE_SFL: Enable SFL library.
*/

#define ZEN_ENABLE_DYNAMIC_LOADING
#define ZEN_ENABLE_OPTIMIZATION
#define ZEN_ENABLE_SFL
/* #define ZEN_MAC_OSX */

#ifdef ZEN_CPLUSPLUS
extern "C"
{
#endif

#include <stdio.h>
#ifndef ZEN_MAC_OSX
#include <malloc.h>
#endif

#define ZEN_START_RESOURCE_INDEX 9511079

#define ZEN_DEFAULT_STACK_SIZE 1024		/* stack size in #words */
#define ZEN_DEFAULT_HEAP_SIZE 1024*500	/* heap size in #bytes */

/* Forward definitions. */
struct _Azure_entity;
struct _avm;
struct _ctable;
struct _word;

typedef unsigned char uchar;
typedef unsigned int uint;

/* Prototype for native functions and callback functions. */
typedef void (*userfunc)(struct _avm* vm);

int ErrorCnt;
FILE* file;
int isDebug;
void WriteErrorMessage(const char* msg);

/* Compile and execute APIs. */
struct _Azure_entity* zen_compile(const char* source);
void zen_callf(struct _Azure_entity* entity, const char* func);
int zen_runstring(const char* source, const char* str);
int zen_runfile(const char* filename);
int zen_debugfile(const char* filename);

/* APIs for manipulating the Zen virtual machine. */
/* Push operations. */
void zen_pushi(struct _Azure_entity* entity, int ival);
void zen_pushf(struct _Azure_entity* entity, float fval);
void zen_pushs(struct _Azure_entity* entity, const char* str);
/* Get return values. */
int zen_getreti(struct _Azure_entity* entity, int n);
float zen_getretf(struct _Azure_entity* entity, int n);
char* zen_getrets(struct _Azure_entity* entity, int n);
/* Get global variables. */
int zen_getglobali(struct _Azure_entity* entity, const char* idname);
float zen_getglobalf(struct _Azure_entity* entity, const char* idname);
char* zen_getglobals(struct _Azure_entity* entity, const char* idname);
/* Set global variables. */
void zen_setglobali(struct _Azure_entity* entity, const char* idname, int ival);
void zen_setglobalf(struct _Azure_entity* entity, const char* idname, float fval);
void zen_setglobals(struct _Azure_entity* entity, const char* idname, const char* sval);
/* Others. */
void zen_settraphook(struct _Azure_entity* entity, userfunc hook);
void zen_release_entity(struct _Azure_entity* entity);

/* Library extension functions. */
void zen_regfunc(userfunc uf, char* fname);
userfunc zen_getfunc(char* name);
void zen_openlibs();
void zen_closelibs();

/* Library functions. */
void zen_openlibstd();
void zen_openlibio();
void zen_openlibmath();
void zen_openlibstr();
void zen_openlibsys();
#ifdef ZEN_ENABLE_DYNAMIC_LOADING
void zen_opendllib();
#endif

/* Console functions. */
void zen_prompt(int argc, char *argv[]);

#ifdef ZEN_CPLUSPLUS
}
#endif

#endif
