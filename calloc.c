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

void *calloc(size_t nmemb, size_t size){
  if(nmemb == (size_t) 0 || size == (size_t) 0){
    return NULL;
  }
  int allocSize = nmemb * size;
  void *blockToReturn = malloc(allocSize);


  if( blockToReturn != NULL ){
    
    blockToReturn = memset(blockToReturn, 0, allocSize);
  }
  return blockToReturn;
}
