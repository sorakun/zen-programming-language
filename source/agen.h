/* See Copyright Notice in azure.h. */

#ifndef _AGEN_H_
#define _AGEN_H_

#include "aparse.h"
#include "avm.h"

typedef struct /* address descriptor */
{
	qoperand o;		/* the operand */
	char regno;		/* in which register? valid if not 0 */
} addrdesc;

void translate(context* ctx, assembly* casm);
symbol* getquadfunc(context* ctx, int quad);

#endif
