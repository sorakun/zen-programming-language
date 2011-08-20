#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *open_sesame (char *name, char mode)
{
  FILE *stream;
  errno = 0;
  stream = fopen (name, mode);
  if (stream == NULL)
  {
    printf (stderr, "I/O error: Couldn’t open file %s; %s\n",
	     name, strerror (errno));
    exit (EXIT_FAILURE);
  }
  else
    return stream;
}


int ffile_copy(const char f1, const char f2)
{
  FILE *in, *out;
  int c;

  in = fopen(f1, "r");
  if (!in) {
    fprintf(stderr, "Error opening input.txt for reading.\n");
    return 1;
  }

  out = fopen(f2, "w");
  if (!out) {
    fprintf(stderr, "Error opening output.txt for writing.\n");
    fclose(in);
    return 1;
  }

  while ((c = fgetc(in)) != EOF) {
    fputc(c, out);
  }

  fclose(out);
  fclose(in);
  return 0;
}
