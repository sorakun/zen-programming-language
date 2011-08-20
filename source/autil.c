/*
 * Utilities.
 * See Copyright Notice in azure.h.
*/

#include <stdlib.h>
#include <string.h>
#include "zen.h"
#include "autil.h"
#include "aerror.h"

/* Make a copy of a string. */
char* cpystr(const char* str)
{
	char* s = calloc(strlen(str)+1, sizeof(char));
	if (s == NULL) return NULL;
	strcpy(s, str);
	return s;
}

/* Cut all the "space" characters from both ends. */
void strcut(char* str, char* spaces)
{
	uint i, pos;
	char* strcopy = calloc(strlen(str)+1, sizeof(char));
	if (strcopy == NULL) return;
	for (i=0; i<strlen(str); i++)
		if (!strchr(spaces, str[i]))
			break;
	for (pos=0; i<strlen(str); i++, pos++)	/* forward */
		strcopy[pos] = str[i];
	for (i=strlen(strcopy); i>=0; i--)		/* backward */
		if (!strchr(spaces, strcopy[i]))
			break;
	strcopy[i+1] = 0;
	strcpy(str, strcopy);
	free(strcopy);
}

/* Get file length from a handle. */
long flengthh(FILE* file)
{
	long cur = ftell(file), len;
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, cur, SEEK_SET);
	return len;
}

/* Get file length from a file name. */
long flength(const char* fname)
{
	FILE* file = fopen(fname, "rb");
	long flen = 0;
	if (file)
	{
		flen = flengthh(file);
		fclose(file);
	}
	return flen;
}

/* Read the file content into a buffer and return it. Return 0 if failed. */
char* readfile(const char* fname)
{
	char* buf;
	FILE* fp;
	uint flen = flength(fname);
	if ((buf=calloc(flen+1, sizeof(char))) == NULL)
		return NULL;
	fp = fopen(fname, "rb");
	if (fp == NULL)
	{
		free(buf);
		return NULL;
	}
	fread(buf, sizeof(char), flen, fp);
	fclose(fp);
	buf[flen] = 0;
	return buf;
}

/* Get the directory part of a path. */
void getdir(const char* path, char* dir)
{
	int len = (int)strlen(path), i;
	for (i=len-1; i>0; i--)
		if (path[i]=='/' || path[i]=='\\')
			break;
	if (i!=0)
		memcpy(dir, path, (i+1)*sizeof(char));
}

/*
	Expand a memory block. The "old" part in the new block is identical to
	the old block, while the rest is zero-initialized. The new block is then
	returned. The old block is freed. All sizes are in bytes.
*/
void* memexp(void* mem, uint oldsize, uint newsize)
{
	void* newmem;
	if (!mem) return (void*)0;
	if (!(newmem=calloc(newsize,1))) return (void*)0;
	memcpy(newmem, mem, oldsize);
	free(mem);
	return newmem;
}
