#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>

#define BASE 2
#define LOG(size) (ceil((log(size) / log(BASE))))
#define PAGE_SIZE 4096

/*
ytes=6188, alignment=32
The  obsolete  function memalign() allocates size bytes and returns a pointer to the allocated memory.
The memory address will be a multiple of alignment, which must be a power of two

*/
void *memalign(size_t alignment, size_t size){
  void *memptr;
  if( size%2 != 0 ){
    return memptr;
  }

  return malloc(size);
/*
  if(res != 0){
    return NULL;
  }else{
    return memptr;
  }
  */

}
