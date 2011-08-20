/*
 * Error functions.
 * See Copyright Notice in azure.h.
*/

#include <stdio.h>
#include "zen.h"
#include "aerror.h"

/* This error message can be redirected. */
int error(const char* msg)
{
	char fmsg[1024];
	sprintf(fmsg, "%s", msg);
	fprintf(stderr, fmsg);
	return -1;
}

int warning(const char* msg)
{
#ifdef ZEN_ENABLE_WARNING
	char fmsg[1024];
	sprintf(fmsg, "%s", msg);
	fprintf(stderr, fmsg);
#endif
	return -1;
}
