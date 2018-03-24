#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include "linked_list.h"

#define BASE 2
#define LOG(size) (ceil((log(size) / log(BASE))))
#define PAGE_SIZE 4096

void *realloc(void *ptr, size_t size){
  if(ptr == NULL){
    return malloc(size);
  }

  MallocNode *originalBlock = ptr - sizeof(MallocNode);
  //The contents will be unchanged in the range from the start of the region up to  the
  //minimum  of  the  old and new sizes.  If the new size is larger than the old size,
  //the added memory will not be initialized.
  size_t sizeOfOriginalBlock = originalBlock->size;
  size_t sizeToCopy = size > sizeOfOriginalBlock ? sizeOfOriginalBlock : size;

  void *blockToReturn = malloc(size);

  if( sizeToCopy == size){
    //expanding the existing block..can copy over all of the data to the new block
    blockToReturn = memcpy(blockToReturn, ptr, size);
  }else{
    blockToReturn = memcpy(blockToReturn, ptr, sizeToCopy);
  }
  //If the area pointed to was moved, a free(ptr) is done.
  //Since the implementation returns a new block anyway, always free()
  free(ptr);

  return blockToReturn + sizeof(blockToReturn);
}
