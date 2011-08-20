/* See Copyright Notice in azure.h. */

#ifndef _ACOROUTINE_H_
#define _ACOROUTINE_H_

#include "avm.h"

#define withinmainthread(vm) (vm->ts.nextslot==0)	/* in the main context? */

long newcoroutine(avm* vm, uint entry, uint stacksize);
void switchtocoroutine(avm* vm, thread* th);
void savecurrentthread(avm* vm);
thread* restorelastthread(avm* vm);
void yield(avm* vm);
void pushthread(avm* vm, thread* th, word* w);

#endif
