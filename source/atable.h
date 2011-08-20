/* See Copyright Notice in azure.h. */

#ifndef _ATABLE_H_
#define _ATABLE_H_

#include "alib.h"

#define ZEN_INITIALTABLESIZE 20		/* initial table size */
#define isnodeempty(n) (gettypew(&n->key)==0 && n->value.entity.ival==0)
#define ind2off(ind) (vm->tot.table[ind].offset)

long hash(const char* key, long range);
long newnode(avm* vm, char nodetype, char* key, word value);
long newtable(avm* vm, long tsize);
/* Table assignment and reference. */
void IR(avm* vm, ctable* table, word* index, word* dest);
void IA(avm* vm, ctable* table, long tindex, word* index, word* source);
void clearTOT(avm* vm);
long TOTinsert(avm* vm, long offset);
int update(avm* vm, ctable* table);
int* packi(avm* vm, ctable* table);
float* packf(avm* vm, ctable* table);

#endif
