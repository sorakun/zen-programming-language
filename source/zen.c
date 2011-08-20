/*
 * Wrapping of compiling and running functions.
 * See Copyright Notice in Zen.h.
*/
#include <time.h>
#include "zen.h"
#include "aconsole.h"
#include "aparse.h"
#include "agen.h"
#include "avm.h"
#include "aerror.h"
#include "autil.h"
#include "alib.h"

int DebugCnt = 0;
int isDebug = 0;

void ErrorFileINIT()
{
    file = fopen("exectrace.txt", "w");
    time_t t;
    fprintf(file, ZEN_CON_HEADER, ZEN_VERSION, getenv("os"));
    time(&t);
    if (time(&t) != (time_t)-1)
    fprintf(file, "Execution Started: At:%s\n", ctime(&t));
    fprintf(file, "________________________________________________________\n\n\n");
}

void WriteErrorMessage(const char* msg)
{
    String_Concat(msg, "\n\n");
    fprintf(file, "%s", msg);
}

void ErrorFileClose()
{
    time_t t;
    fprintf(file, "________________________________________________________\n\n\n");
    time(&t);
    if (time(&t) != (time_t)-1)
    fprintf(file, "Execution Ended: At:%s\n", ctime(&t));
    fprintf(file, "________________________________________________________\n\n\n");
    fclose(file);
}

/* Compile a script ready in the buffer. */
Azure_entity* compilebuf(const char* source, char* buf)
{
	Azure_entity* entity = (Azure_entity*)calloc(1, sizeof(Azure_entity));
	parseroot(source, buf, &entity->ctx);
	if (validatecontext(&entity->ctx))
		translate(&entity->ctx, &entity->assm);
	load(&entity->ctx, &entity->vm, &entity->assm);
	return entity;
}

/* Compile a script by its name. */
Azure_entity* zen_compile(const char* source)
{
	Azure_entity* entity;
	char* buf = readfile(source);
	if (!buf) return (Azure_entity*)0;
	entity = compilebuf(source, buf);
	free(buf);
	return entity;
}

void zen_callf(Azure_entity* entity, const char* func)
{
	callf(&entity->vm, func);
}

/* Execute a scrippet. */
int zen_runstring(const char* source, const char* str)
{
	Azure_entity* entity = compilebuf(source, (char*)str);
	zen_openlibs();
	run(&entity->ctx, &entity->vm);
	zen_release_entity(entity);
	return 0;
}

/* Execute a script by its name. */
int zen_runfile(const char* filename)
{
	char* buf = readfile(filename);
	if (!buf) return error("File not found!\n");
	zen_runstring(filename, buf);
	free(buf);
	return 0;
}

/* Debug a script by its name. */
int zen_debugfile(const char* filename)
{
    isDebug = 1;
    ErrorFileINIT();
	char* buf = readfile(filename);
	if (!buf) return error("File not found!\n");
	zen_runstring(filename, buf);
	free(buf);
	isDebug = 0;
	ErrorCnt = 0;
	ErrorFileClose();
	return 0;
}


void zen_pushi(Azure_entity* entity, int ival)
{
	pushi(&entity->vm, ival);
}

void zen_pushf(Azure_entity* entity, float fval)
{
	pushf(&entity->vm, fval);
}

void zen_pushs(struct _Azure_entity* entity, const char* str)
{
	pushs(&entity->vm, str);
}

/* Get the nth return value as integer type. */
int zen_getreti(struct _Azure_entity* entity, int n)
{
	word* w = getret(&entity->vm, n);
	verifytype(&entity->vm, w, TINTEGER);
	return w->entity.ival;
}

/* Get the nth return value as float type. */
float zen_getretf(struct _Azure_entity* entity, int n)
{
	word* w = getret(&entity->vm, n);
	verifytype(&entity->vm, w, TFLOAT);
	return w->entity.fval;
}

/* Get the nth return value as string type. */
char* zen_getrets(struct _Azure_entity* entity, int n)
{
	word* w = getret(&entity->vm, n);
	char* str = (char*)getdata(&entity->vm, w->entity.ival);
	verifytype(&entity->vm, w, TSTRING);
	return str;
}

int zen_getglobali(Azure_entity* entity, const char* idname)
{
	return getglobali(&entity->vm, idname);
}

float zen_getglobalf(Azure_entity* entity, const char* idname)
{
	return getglobalf(&entity->vm, idname);
}

char* zen_getglobals(Azure_entity* entity, const char* idname)
{
	return getglobals(&entity->vm, idname);
}

void zen_setglobali(Azure_entity* entity, const char* idname, int ival)
{
	setglobali(&entity->vm, idname, ival);
}

void zen_setglobalf(struct _Azure_entity* entity, const char* idname, float fval)
{
	setglobalf(&entity->vm, idname, fval);
}

void zen_setglobals(struct _Azure_entity* entity, const char* idname, const char* sval)
{
	setglobals(&entity->vm, idname, sval);
}

void zen_settraphook(struct _Azure_entity* entity, userfunc hook)
{
	settraphook(&entity->vm, hook);
}

/* Release the Zen entity. */
void zen_release_entity(Azure_entity* entity)
{
	deinitcontext(&entity->ctx);
	releasevm(&entity->vm);
	free(entity);
}
