#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "malloc.c"
#include "free.c"
#include "calloc.c"
#include "realloc.c"


int main(int argc, char **argv)
{
  char buf[1024];
  size_t size = 4004;

  void *mem = calloc(4004, 1);
  assert( mem != NULL );
  sprintf(buf, "Successfully calloc'd %zu bytes at addr %p\n", 4004, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  free(mem);

/*
  void *mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  assert(mem != NULL);
  free(mem);

  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  mem = malloc(size);

  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  assert(mem != NULL);
  free(mem);

  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  assert(mem != NULL);
  free(mem);

  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
*/




/*
  void *mem1 = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem1);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  void *mem2 = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem2);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  void *mem3 = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem3);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

  free(mem1);
  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem1);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  free(mem2);
  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem2);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  free(mem3);
  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem3);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
*/


  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  assert(mem != NULL);
  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  mem = malloc(size);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);



  void *mem4 = malloc(5096);
  sprintf(buf, "Successfully malloc'd %zu bytes at addr %p\n", size, mem4);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  free(mem4);
  sprintf(buf, "Successfully free'd %zu bytes from addr %p\n", size, mem4);
  write(STDOUT_FILENO, buf, strlen(buf) + 1);

/*

  */

  return 0;
}
