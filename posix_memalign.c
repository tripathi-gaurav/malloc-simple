#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
//#include "linked_list.h"

/*
The  function  posix_memalign() allocates size bytes and places the address of the allocated memory in *memptr.
The address of the allocated memory will be a multiple of alignment, which must be a power of two and a multiple of sizeof(void *).
If size is 0, then the value placed in *memptr is either NULL, or a unique pointer value that can later be successfully passed to free(3).

posix_memalign()  returns  zero  on  success,  or  one  of the error values listed in the next section on failure.
The value of errno is not set.
On Linux (and other systems),posix_memalign() does not modify memptr on failure.
A requirement standardizing this behavior was added in POSIX.1-2016.

*/

int posix_memalign(void **memptr, size_t alignment, size_t size){
  if( size == (size_t) 0 ){
    //If size is 0, then the value placed in *memptr is either NULL
    *memptr = NULL;
    return 0;
  }else if(alignment % 2 != 0 || alignment % sizeof( void*) != 0 ){
    //EINVAL The alignment argument was not a power of two, or was not a multiple of sizeof(void *).
    return EINVAL;
  }

  *memptr = malloc(size);
  if(*memptr == NULL){
    //ENOMEM There was insufficient memory to fulfill the allocation request.
    return ENOMEM;
  }else{
    return 0;
  }
}
