/* See Copyright Notice in azure.h. */

#ifndef _AGC_H_
#define _AGC_H_

#include "avm.h"

/* Get the size of a memory block. */
#define sizeb(mb) ((mb).eoff-(mb).boff+1)

void bookblock(avm* vm, mbtable* t, long boff, long eoff, char merge);
void unbookblock(avm* vm, mbtable* t, long boff, long eoff);
long allocblock(avm* vm, long size);
long freeblock(avm* vm, heap* hp, long boff);
void addextobj(avm* vm, word* w);
void freeextobjs(avm* vm);
void collect(avm* vm);

#endif
