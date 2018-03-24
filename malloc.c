#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include "linked_list.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int numberOfFreeLists = 12;
MallocNode* freeList[12];
__thread MallocNode *head = NULL;

void initHead();
MallocNode *findFreeBlockInFreeList(size_t size);

void *malloc(size_t size){

  if(size == 0){
    return NULL;
  }

  pthread_mutex_lock(&mutex);
  size_t allocSize = size + sizeof(MallocNode);
  if (head == NULL){
    initHead();
  }
  /*
  if( freeList[numberOfFreeLists-1] == NULL ){
    errno = ENOMEM;
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  */
  int indexInFreeList = LOG(allocSize) - 1;
  char buf[1024];

  if (allocSize < (ssize_t) 4096 ){
    //if allocSize is less than PAGE_SIZE then findFreeBlockInFreeList(allocSize)
    //if NULL is returned from findFreeBlockInFreeList, then call sbrk()


    //sprintf(buf, "allocSize was: %zu\nlooking at index: %d\n",allocSize, indexInFreeList );
    //write(STDOUT_FILENO, buf, strlen(buf) + 1);


      MallocNode *allocatedNode = findFreeBlockInFreeList(allocSize);
      if( allocatedNode == NULL ) {
        errno = ENOMEM;
        pthread_mutex_unlock(&mutex);
        return NULL;
      }else{
        allocatedNode->isAllocated = 1;
        pthread_mutex_unlock(&mutex);
        return (char *) allocatedNode + sizeof(MallocNode);
      }
  }else{
    //if allocSize is more than PAGE_SIZE, then return mmap(allocSize)
    //findFreeBlockInFreeList() will iterate over the free list........
    //sprintf(buf, "allocSize was: %zu\nperforming mmap\n",allocSize );
    //write(STDOUT_FILENO, buf, strlen(buf) + 1);

    void* newMemory = mmap(0, allocSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(newMemory != MAP_FAILED);

    MallocNode *temp = (MallocNode*) ( (char*) newMemory );
    temp->size = allocSize;
    temp->isAllocated = 999;

    pthread_mutex_unlock(&mutex);
    return newMemory + sizeof(MallocNode);
  }

}

void initHead(){  //initHeap instead?
  if(head == NULL){
    head = sbrk(PAGE_SIZE);
    if( head == NULL ){
      errno = ENOMEM;
      return ;
    }
    freeList[numberOfFreeLists-1] = (MallocNode *) head; //heap ??
    freeList[numberOfFreeLists-1]->size = PAGE_SIZE;
    freeList[numberOfFreeLists-1]->isAllocated = 0;
    freeList[numberOfFreeLists-1]->next = NULL;
    freeList[numberOfFreeLists-1]->prev = NULL;
    freeList[numberOfFreeLists-1]->dummy = 99999;
    freeList[numberOfFreeLists-1]->blockMaxAddr = (int *) ( (char*) head + PAGE_SIZE );
    /*char buf[1024];
    sprintf(buf, "End addr of head: %p\n", freeList[numberOfFreeLists-1]->blockMaxAddr );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
  }
}

MallocNode *findFreeBlockInFreeList(size_t size){
  int i = 0;
  int indexInFreeList = (ceil((log(size) / log(BASE)))) - 1;
  MallocNode *nodeToAllocate = NULL;

  /*
  char buff2[1024];
  sprintf(buff2, "value of indexInFreeList=%d\n", indexInFreeList );
  write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
  */

  if( freeList[indexInFreeList] != NULL ){
    //free block of the required size is available

    /*
    char buff2[1024];
    sprintf(buff2, "%s\n", "found the nodeToAllocate" );
    write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
    */

    nodeToAllocate = (MallocNode *) freeList[indexInFreeList];

    nodeToAllocate->next = NULL;
    nodeToAllocate->prev = NULL;
    nodeToAllocate->isAllocated = 1;
    nodeToAllocate->size = size - sizeof(MallocNode);

    if(nodeToAllocate->next != NULL){
      freeList[indexInFreeList] = freeList[indexInFreeList]->next;
      freeList[indexInFreeList]->prev = NULL;
    }else{
      freeList[indexInFreeList] = NULL;
    }

    return nodeToAllocate;
  }else{
    //free block of required size is not available
    //find if a free block of greater size is available
    char buf[1024];

    //sprintf(buf, "indexInFreeList: %d and numberOfFreeLists: %d\n", indexInFreeList, numberOfFreeLists );
    //write(STDOUT_FILENO, buf, strlen(buf) + 1);

    for(i=indexInFreeList;i<numberOfFreeLists;i++){
      MallocNode *temp;
      int currentIndex = i;
      if( freeList[i] != NULL ){
        //a free block of greater size is found, then break it down to meet rqmts
        while( currentIndex != indexInFreeList ){
          temp = freeList[currentIndex];
          temp->size = temp->size / 2;
          //create a new block at a location that is <size> bytes away
          MallocNode *new = (MallocNode*) ( (char*) temp + temp->size );

          //TEMP and NEW are the two blocks now
          new->size = temp->size;
          new->isAllocated = 0;
          new->prev = temp;
          new->next = NULL;
          new->blockMaxAddr = temp->blockMaxAddr;
          new->dummy = temp->dummy;
          //if the block under consideration has more blocks in front of it,
          //...then the pointers of those blocks need to be updated
          if( temp->next != NULL ){
            MallocNode *next = temp->next;
            next->prev = NULL;
            freeList[currentIndex] = next;
          }else{
            freeList[currentIndex] = NULL;
          }
          //either case, the current block (temp* in our case) needs to be updated
          temp->next = new;  //this logic can actually go above the if case
          currentIndex--;
          freeList[currentIndex] = temp;
          //sprintf(buf, "temp's size: %zu and currentIndex: %d\n", temp->size, currentIndex );
          //write(STDOUT_FILENO, buf, strlen(buf) + 1);
        }
        nodeToAllocate = freeList[currentIndex];
        if(nodeToAllocate->next != NULL){
          freeList[currentIndex] = freeList[currentIndex]->next;
          freeList[currentIndex]->prev = NULL;
          //21 MAR...release the nodeToFree from it's bonds
          nodeToAllocate->next = NULL;
          nodeToAllocate->prev = NULL;
        }else{
          freeList[currentIndex] = NULL;
        }
        //by now we have found a free block and broken it down
        //so we can break the loop and return the block
        return nodeToAllocate;
      }else if( freeList[i] == NULL && currentIndex == numberOfFreeLists-1 ){
        //means we have reached the end of free list and COULD NOT find a free block
        //therefore, request a new free block
        MallocNode *moreMemory = sbrk(PAGE_SIZE);

        if(moreMemory == NULL){
          errno = ENOMEM;
          return NULL;
        }
        freeList[numberOfFreeLists-1] = (MallocNode *) moreMemory;
        freeList[numberOfFreeLists-1]->size = PAGE_SIZE;
        freeList[numberOfFreeLists-1]->isAllocated = 0;
        freeList[numberOfFreeLists-1]->next = NULL;
        freeList[numberOfFreeLists-1]->prev = NULL;
        freeList[numberOfFreeLists-1]->dummy = 99999;
        freeList[numberOfFreeLists-1]->blockMaxAddr = (int *) ( (char*) moreMemory + PAGE_SIZE );
        /*
        char buf[1024];
        sprintf(buf, "End addr of moreMemory: %p\n", freeList[numberOfFreeLists-1]->blockMaxAddr );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        */
        //decrement the counter by 1 so that loop can continue from this element
        //and then break down this new block
        i--;
      }
    }//end of for loop that finds an empty block

  }
  return nodeToAllocate;
}
