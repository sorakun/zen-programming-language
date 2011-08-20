/* See Copyright Notice in azure.h. */

#ifndef _AUTIL_H_
#define _AUTIL_H_

#include <stdio.h>

char* cpystr(const char* str);
void strcut(char* str, char* spaces);
long flengthh(FILE* file);
long flength(const char* fname);
char* readfile(const char* fname);
void getdir(const char* path, char* dir);
void* memexp(void* mem, uint oldsize, uint newsize);

#endif
