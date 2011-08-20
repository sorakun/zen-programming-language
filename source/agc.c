/*
 * Gargbage collection.
 * See Copyright Notice in azure.h.
*/

#include <memory.h>
#include "zen.h"
#include "atable.h"
#include "aerror.h"
#include "agc.h"
#include "autil.h"

#define GCTHRESHHOLD 0.5f	/* larger means more frequent gc */

/*
	Add a block record to block table t. If merge is true, adjacent blocks are
	merged together to formed a larger block.
*/
void bookblock(avm* vm, mbtable* t, long boff, long eoff, char merge)
{
	int i,j;
	long temp;
	if (t->nextslot >= t->size)	/* try to expand the mem block table */
	{
		if (!(t->mbt =
			memexp(t->mbt,t->size*sizeof(mblock),t->size*sizeof(mblock)*2)))
		{
			vm->lasterror = ZEN_VM_HEAP_ALLOCATION_ERROR;
			return;
		}
		t->size = t->size*2;
	}
	for (i=0; i<t->nextslot; i++)
		if (eoff<t->mbt[i].boff)	/* insert before the ith block record */
		{
			for (j=t->nextslot; j>=i+1; j--)	/* move backward */
				t->mbt[j] = t->mbt[j-1];
			break;
		}
	t->mbt[i].boff = boff;
	t->mbt[i].eoff = eoff;
	t->nextslot++;
	/* merge necessary only if there are more than one blocks */
	if (merge && t->nextslot>1)
	{
		if (i==0)	/* at the beginning of the table */
		{
			if (t->mbt[i].eoff == t->mbt[i+1].boff-1)	/* adjacent */
			{
				t->mbt[i].eoff = t->mbt[i+1].eoff;
				for (j=i+1; j<t->nextslot-2; j++)	/* move backward */
					t->mbt[j] = t->mbt[j+1];
				t->nextslot--;
			}
		}
		else if (i==t->nextslot-1)	/* at the end of the table */
		{
			if (t->mbt[i].boff == t->mbt[i-1].eoff+1)	/* adjacent */
			{
				t->mbt[i-1].eoff = t->mbt[i].eoff;
				t->nextslot--;
			}
		}
		else	/* in the middle, the general case */
		{
			if (t->mbt[i].eoff+1==t->mbt[i+1].boff)		/* adjacent at back */
			{
				temp = t->mbt[i+1].eoff;
				unbookblock(vm, t, t->mbt[i+1].boff, t->mbt[i+1].eoff);
				t->mbt[i].eoff = temp;
			}
			if (t->mbt[i].boff == t->mbt[i-1].eoff+1)	/* adjacent at front */
			{
				temp = t->mbt[i].eoff;
				unbookblock(vm, t, t->mbt[i].boff, t->mbt[i].eoff);
				t->mbt[i-1].eoff = temp;
			}
		}
	}
}

/* Remove a block record to block table t. */
void unbookblock(avm* vm, mbtable* t, long boff, long eoff)
{
	int i,j;
	if (t->nextslot<=0)
		vm->lasterror = ZEN_VM_HEAP_ALLOCATION_ERROR;
	else
		for (i=0; i<t->nextslot; i++)
			if (t->mbt[i].boff == boff)
			{
				for (j=i; j<=t->nextslot-1; j++)
					t->mbt[j] = t->mbt[j+1];
				t->nextslot--;
				break;
			}
}

/*
	Scan through the heap's available block list. If there is such a block
	that satisfies, return its offset; otherwise return 0.
*/
long scanforblock(avm* vm, long size)
{
	int i;
	heap* hp = &vm->hp;
	long offset = 0;
	long blocksize = ZEN_BLOCKHEADERSIZE+size;
	for (i=0; i<hp->avtable.nextslot; i++)
	{
		/* if the block is pretty large, divide it into two subblocks */
		if (sizeb(hp->avtable.mbt[i]) > size+ZEN_BLOCKHEADERSIZE+3)
		{
			offset = hp->avtable.mbt[i].boff;
			hp->avtable.mbt[i].boff += blocksize;
			bookblock(vm, &hp->altable, offset, offset+ZEN_BLOCKHEADERSIZE+size-1, 0);
			hp->availsize -= blocksize;
			break;
		}
	}
	return offset;
}

/* Is the memory stressed? */
int memstressed(avm* vm)
{
	if ((float)vm->hp.availsize/vm->hp.size<GCTHRESHHOLD ||
		(float)vm->hp.altable.nextslot/vm->hp.altable.size>1-GCTHRESHHOLD)
		return 1;
	return 0;
}

/* Expand the heap. The heap's new size must be not smaller than minimalsize. */
int expandheap(avm* vm, heap* hp, long minimalsize)
{
	long oldsize = hp->size;
	hp->size = hp->size*2+minimalsize+ZEN_BLOCKHEADERSIZE;
	hp->availsize += hp->size-oldsize;
	if (vm->oldheap)
		free(vm->oldheap);
	vm->oldheap = hp->heap;
	if (hp->heap = (char*)calloc(hp->size, sizeof(char)))
	{
		memcpy(hp->heap, vm->oldheap, oldsize);
		hp->avtable.mbt[hp->avtable.nextslot-1].eoff = hp->size-1;
	}
	return (hp->heap != 0);
}

/*
	Allocate a new memory block in vm's heap. If return value == 0, failed;
	otherwise return value has the offset of the block. size is data size in
	bytes.
	The whole process goes through two steps:
	1. Scan through the available list for the block;
	2. If still not found, expand the heap, then scan again.
	The block is zero-initialized.
*/
long allocblock(avm* vm, long size)
{
	long offset;
	vm->gcapplied = memstressed(vm)?1:vm->gcapplied;
	if (!(offset=scanforblock(vm, size)))		/* cannot find a block */
	{
		vm->gcapplied = 1;
		if (!(expandheap(vm, &vm->hp, size) && (offset=scanforblock(vm, size))))
		{
			setvmerror(vm, ZEN_VM_OUT_OF_MEMORY);
			return 0;
		}
	}
	memset((void*)getdata(vm->hp.heap, offset), 0, size);
	return offset;
}

/*
	Free a block in vm's heap. The block has the offset of boffset.
	Size of the block is returned.
*/
long freeblock(avm* vm, heap* hp, long boff)
{
	int i;
	long size = 0;
	for (i=0; i<hp->altable.nextslot; i++)
	{
		if (hp->altable.mbt[i].boff == boff)
		{
			size = hp->altable.mbt[i].eoff-hp->altable.mbt[i].boff+1;
			bookblock(vm, &hp->avtable, hp->altable.mbt[i].boff, hp->altable.mbt[i].eoff, 1);
			unbookblock(vm, &hp->altable, hp->altable.mbt[i].boff, hp->altable.mbt[i].eoff);
			hp->availsize += size;
			break;
		}
	}
	if (size == 0)
		setvmerror(vm, ZEN_VM_HEAP_ALLOCATIONTABLE_CORRUPT);
	return size;
}

/* Recursively mark live objects. */
void rmark(avm* vm, word* w)
{
	int i;
	long offset;
	objheader* obj;
	ctable* table;
	cnode *nodes, *node;
	thread* th;
	if (!w || !isobject(w)) return;
	offset = istable(w)?ind2off(w->entity.ival):w->entity.ival;
	obj = (objheader*)getobj(vm->hp.heap, offset);
	table = (ctable*)getdata(vm->hp.heap, offset);
	obj->marked = 1;
	if (istable(w))
	{
		vm->tot.table[w->entity.ival].marked = 1;
		nodes = (cnode*)&(table->nodes);
		/*
			Bugs here; non-existing objects should not be marked.
			If so, the heap will be corrupted.
		*/
		for (i=0; i<table->tablesize; i++)
		{
			node = &nodes[i];
			while (1)	/* mark each node of the slot's list's */
			{
				rmark(vm, &node->key);
				rmark(vm, &node->value);
				if (node->nextnode == 0)
					break;
				node = (cnode*)getdata(vm->hp.heap, node->nextnode);
			}
		}
	}
	else if (isthread(w))	/* mark its stack */
	{
		 th = (thread*)getdata(vm->hp.heap, w->entity.ival);
		 obj = (objheader*)getobj(vm->hp.heap, th->s->soff);
		 obj->marked = 1;
	}
}

/* Mark all the variables in thread th. */
void rmarkthread(avm* vm, thread* th)
{
	int i;
	for (i=0; i<th->regs[rs].entity.ival; i++)
		rmark(vm, &th->s->sbody[i]);
	for (i=ra; i<=re; i++)
		rmark(vm, &th->regs[i]);
}

/* Add an external object to the list. */
void addextobj(avm* vm, word* w)
{
	extobj *obj, *obj1;
	obj1 = (extobj*)calloc(1, sizeof(extobj));
	obj1->w = *w;
	if (!vm->extobjs)
		vm->extobjs = obj1;
	else
	{	/* seach to the end of the list */
		for (obj=vm->extobjs; obj->next!=0; obj=obj->next);
		obj->next = obj1;
	}
}

/* Free all external objects. */
void freeextobjs(avm* vm)
{
	extobj *obj, *obj1;
	obj = obj1 = vm->extobjs;
	while (obj)
	{
		obj1 = obj;
		obj = obj->next;
		if (gettypew(&obj1->w) == TBLOB)
			free((void*)obj1->w.entity.ival);
		free(obj1);
	}
	vm->extobjs = 0;
}

/* Collect the garbage in vm's heap. Current instruction must be gc-safe. */
void collect(avm* vm)
{
	int i;
	long offset, pieces=0, totalsize=0;
	objheader* obj;
	for (i=0; i<(int)vm->tot.size; i++)	/* clear out TOT's mark bits*/
		vm->tot.table[i].marked = 0;
	/* clear out allocated objects' mark bits */
	for (i=0; i<vm->hp.altable.nextslot; i++)
	{
		offset = vm->hp.altable.mbt[i].boff;
		obj = (objheader*)getobj(vm->hp.heap, offset);
		obj->marked = 0;
	}
	/* Mark current thread. */
	for (i=0; i<vm->regs[rs].entity.ival; i++)	/* mark from root set */
		rmark(vm, &vm->cs->sbody[i]);
	for (i=ra; i<=re; i++)
		rmark(vm, &vm->regs[i]);
	/* Mark all other threads. */
	for (i=0; i<vm->ts.nextslot; i++)
		rmarkthread(vm, vm->ts.threads[i]);
	for (i=0; i<vm->hp.altable.nextslot;)	/* sweep dead objects */
	{
		offset = vm->hp.altable.mbt[i].boff;
		obj = (objheader*)getobj(vm->hp.heap, offset);
		if (!obj->marked)	/* dead */
		{
			totalsize += freeblock(vm, &vm->hp, offset);
			pieces++;
		}
		else i++;
	}
	clearTOT(vm);
	vm->gcapplied = 0;
}
