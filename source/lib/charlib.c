
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0')
      return 0;
    char * p;
    strtod (s, &p);
    return *p == '\0';
}

/* upper-cases s in place */
void str_toupper(char *s)
{
    while(*s)
    {
        *s=toupper(*s);
        s++;
    }
}


/* lower-cases s in place */
void str_tolower(char *s)
{
    while(*s)
    {
        *s=tolower(*s);
        s++;
    }
}
char CHR(int x)
{
	char buf[2] = {0};
	buf[0] = (char)x;
	return buf;
}

int ORD ( char c )
{
    return (int)c;
}

char * String_Concat(char *string, const char *string_to_append)
{
    int len1, len2;
    char *result;
    len1 = strlen(string);
    len2 = strlen(string_to_append);
    if(NULL ==
            (result = realloc(string, (len1+len2+1) * sizeof *result)))
        return (NULL);
    memcpy(result + len1, string_to_append, len2+1);
    return result;
}

char * String_Concat2(char * string, const char * string_to_append)
{
    int i, j = 0;

    int size = strlen(string);
    int size2 = strlen(string_to_append);

    for (i = strlen(string); i< size+size2; i++)
    {
       string[i]  = string_to_append[j];
       j++;
    }
    return string;
}

char * string_repeat(const char * s,  int n) {
  size_t slen = strlen(s);
  char * dest = calloc(n*slen+1, sizeof(char));

  int i; char * p;
  for ( i=0, p = dest; i < n; ++i, p += slen ) {
    memcpy(p, s, slen);
  }
  return dest;
}

/* This function assumes the passed pointer points to a valid, zero-terminated string */
void string_reverse (char *s)
{
  char *t = s;

  while (*t != '\0') t++;
  while (s < t)
  {
    int c = *s;
    *s++ = *--t;
    *t = c;
  }
}

/* Swaps strings by swapping pointers */
void swap(char **str1_ptr, char **str2_ptr)
{
  char *temp = *str1_ptr;
  *str1_ptr = *str2_ptr;
  *str2_ptr = temp;
}
