/*
 * Tables are indirectly referenced by their "table of table (TOT)" indices,
 * which allows for easy auto-expanding.
 * See Copyright Notice in azure.h.
*/

#include <string.h>
#include "zen.h"
#include "agc.h"
#include "atable.h"
#include "aerror.h"
#include "autil.h"

/* Calculate a hash value for a key beteween [0, range-1]. */
long hash(const char* key, long range)
{
	long i, sum;
	for (i=0, sum=0; i<(long)strlen(key); i++)
		sum += key[i];
	return (long)(sum%range);
}

/* Create a node with key and value, for hash tables. */
long newnode(avm* vm, char nodetype, char* key, word value)
{
	objheader* obj;
	cnode* node;
	heap* hp = &vm->hp;
	long size = sizeof(cnode);
	long offset = allocblock(vm, size);
	obj = (objheader*)getobj(hp->heap, offset);
	obj->type = TNODE;
	obj->size = size;
	obj->offset = (long)sizeof(mblock)+offset;
	node = (cnode*)getdata(hp->heap, offset);
	settypew(&node->key, nodetype);
	node->key.entity.ival = newstring(vm, key);
	node->value = value;
	node->nextnode = 0;
	return offset;
}

void clearTOT(avm* vm)
{
	uint i;
	for (i=0; i<vm->tot.size; i++)
		if (!vm->tot.table[i].marked)
			vm->tot.table[i].offset = 0;
}

/* Expand the TOT. */
void expandTOT(avm* vm)
{
	if (!(vm->tot.table = memexp(vm->tot.table, vm->tot.size*sizeof(pair),
		vm->tot.size*2*sizeof(pair))))
		setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
	vm->tot.size *= 2;
}

/* Get the next available TOT slot. The 0th slot is reserved as an error code. */
long TOTnextslot(avm* vm)
{
	uint i, oldsize = vm->tot.size;
	for (i=1; i<vm->tot.size; i++)
		if (vm->tot.table[i].offset == 0)
			return i;
	expandTOT(vm);
	return oldsize;
}

/* Insert a table with offset into TOT and return the position slot. */
long TOTinsert(avm* vm, long offset)
{
	long slot = TOTnextslot(vm);
	vm->tot.table[slot].offset = offset;
	return slot;
}

/* Create an empty table in the heap. */
long newtable(avm* vm, long tsize)
{
	objheader* obj;
	ctable* table;
	heap* hp = &vm->hp;
	long size = ZEN_TABLESIZE(tsize);
	long offset = allocblock(vm, size);
	if (offset)
	{
		obj = (objheader*)getobj(hp->heap, offset);
		obj->type = TTABLE;
		obj->size = size;
		obj->offset = (long)sizeof(mblock)+offset;
		table = (ctable*)getdata(hp->heap, offset);
		table->tablesize = tsize;
		table->nextslot = 0;
	}
	return offset;
}

/* Given an index, return the corresponding slot in the table. */
long getslot(avm* vm, ctable* table, word* index)
{
	return (gettypew(index)==TINTEGER?index->entity.ival:
		hash((char*)getdata(vm->hp.heap,index->entity.ival),table->tablesize));
}

/* Table reference. *dest = table[index]. */
void IR(avm* vm, ctable* table, word* index, word* dest)
{
	cnode *node, *nodes = (cnode*)&(table->nodes);
	int indextype, nodetype;
	char *indexkey, *nodekey;
	long slot = getslot(vm, table, index);
	if (slot >= table->tablesize)
	{
		setnil(dest);
		return;
	}
	node = &nodes[slot];
	memset(dest, 0, sizeof(word));
	while (1)
	{
		indextype = gettypew(index);
		nodetype = gettypew(&node->key);
		indexkey = (char*)getdata(vm->hp.heap, index->entity.ival);
		nodekey = (char*)getdata(vm->hp.heap, node->key.entity.ival);
		if (indextype==TINTEGER&&nodetype==TINTEGER	/* ok, found*/
			|| indextype==TSTRING&&nodetype==TSTRING&&strcmp(indexkey, nodekey)==0)
		{
			*dest = node->value;
			break;
		}
		if (node->nextnode == 0)	/* reach the end of the list */
			break;
		node = (cnode*)getdata(vm->hp.heap, node->nextnode);
	}
}

/* Copy stable's content to dtable. They can have different sizes. */
void copytable(avm* vm, ctable* dtable, long dtindex, ctable* stable)
{
	long i;
	cnode *dnodes = (cnode*)&(dtable->nodes);
	cnode *snodes = (cnode*)&(stable->nodes);
	cnode* node;
	for (i=0; i<stable->tablesize; i++)	/* go through every slot of stable */
	{
		node = &snodes[i];
		if (isnodeempty(node))
			continue;
		while (1)  /* add all the nodes of that slot to dtable */
		{
			IA(vm, dtable, dtindex, &node->key, &node->value);
			if (node->nextnode == 0)
				break;
			node = (cnode*)getdata(vm->hp.heap, node->nextnode);
		}
	}
}

/* Expand a table with 2*maxslot size. otindex: old table TOT slot. */
void expandtable(avm* vm, long otindex, long maxslot)
{
	ctable *table, *oldtable;
	long off = newtable(vm, 2*maxslot);
	table = (ctable*)getdata(vm->hp.heap, off);
	oldtable = (ctable*)getdata(vm->hp.heap, ind2off(otindex));
	copytable(vm, table, otindex, oldtable);
	vm->tot.table[otindex].offset = off;
}

/*
	Table assignment. table[index] = *source.
	tindex is the TOT index of the table.
*/
void IA(avm* vm, ctable* table, long tindex, word* index, word* source)
{
	ctable* newtable;
	cnode *nodes = (cnode*)&(table->nodes), *node, *node1;
	int indextype, nodetype;
	char *indexkey, *nodekey;
	char found = 0;
	long nodeoff;
	long oldnextslot = table->nextslot;
	long slot = getslot(vm, table, index);
	table->nextslot = slot>=oldnextslot?slot+1:oldnextslot;
	if (slot>= table->tablesize)	/* index exceeds, so expand the table */
	{
		expandtable(vm, tindex, slot);
		newtable = (ctable*)getdata(vm->hp.heap, ind2off(tindex));
		newtable->nextslot = table->nextslot;
		nodes = (cnode*)&(newtable->nodes);
	}
	node = &nodes[slot];
	if (isnodeempty(node))
	{
		node->key = *index;
		node->value = *source;
	}
	else	/* search through the node list at position slot */
	{
		while (1)
		{
			indextype = gettypew(index);
			nodetype = gettypew(&node->key);
			indexkey = (char*)getdata(vm->hp.heap, index->entity.ival);
			nodekey = (char*)getdata(vm->hp.heap, node->key.entity.ival);
			if (indextype==TINTEGER&&nodetype==TINTEGER	/* ok, found*/
				|| indextype==TSTRING&&nodetype==TSTRING&&strcmp(indexkey, nodekey)==0)
			{
				node->value = *source;
				found = 1;
				break;
			}
			if (node->nextnode==0)	/* reach the end of the list */
				break;
			node = (cnode*)getdata(vm->hp.heap, node->nextnode);
		}
		if (!found)		/* not found, append a new node at the list's end */
		{
			nodeoff = newnode(vm, gettypew(index),
				(char*)getdata(vm->hp.heap, index->entity.ival), *source);
			node1 = (cnode*)getdata(vm->hp.heap, nodeoff);
			node->nextnode = nodeoff;
		}
	}
}

/*
	Update the next (k,v) pair for a table and put them into wkey and wval.
	Used for the foreach loop. When reaching the end of the table, return 0,
	otherwise 1.
*/
int update(avm* vm, ctable* table)
{
	cnode *nodes = (cnode*)&(table->nodes);
	cnode *node;
	int hasmorenodes;
	char* keystr;
	while (nodes[vm->slot].value.tag==0 && vm->slot<table->tablesize)
		vm->slot++;
	if (vm->slot>=table->tablesize)
		return 0;
	if (vm->node==0)
	{
		vm->wkey = nodes[vm->slot].key;
		vm->wval = nodes[vm->slot].value;
	}
	else
	{
		node = (cnode*)getdata(vm->hp.heap, vm->node);
		vm->wkey = node->key;
		vm->wval = node->value;
	}
	keystr = (char*)getdata(vm->hp.heap, vm->wkey.entity.ival);
	if (vm->node == 0)
		hasmorenodes = (nodes[vm->slot].nextnode!=0);
	else
	{
		node = (cnode*)getdata(vm->hp.heap, vm->node);
		hasmorenodes = (node->nextnode!=0);
	}
	if (!hasmorenodes)
	{
		vm->slot++;
		vm->node = 0;
	}
	else
		if (vm->node == 0)
			vm->node = nodes[vm->slot].nextnode;
		else
		{
			node = (cnode*)getdata(vm->hp.heap, vm->node);
			vm->node = node->nextnode;
		}
	return 1;
}

int* packi(avm* vm, ctable* table)
{
	return 0;
}

float* packf(avm* vm, ctable* table)
{
	int i;
	float* arr = calloc(table->tablesize, sizeof(float));
	cnode* nodes = (cnode*)&(table->nodes);
	for (i=0; i<table->tablesize; i++)
		arr[i] = nodes[i].value.entity.fval;
	return arr;
}
