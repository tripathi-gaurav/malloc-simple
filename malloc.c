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
int numberOfFreeLists = 13;
//MallocNode* freeList[13];
__thread MallocNode *head = NULL;
MallocArena *arena_head = NULL;
int numberOfThreads = 0;
//thread specific global
__thread MallocArena* arenaAssignedToThread = NULL;

/*TODO: Remove initHead() method*/
//void initHead();
MallocArena * initArenas();
MallocArena * getArenaForRequest(int arenaToHandleRequest);
MallocNode *allocateMemoryInArena(MallocArena *assignedArena, size_t size);
MallocNode *findFreeBlockInFreeList(MallocArena* freeList, size_t size);
MallocNode *getMoreMemoryFromArena(MallocArena* freeList, size_t size);

void * malloc(size_t size){

  if(size == 0){
    return NULL;
  }
  char buf[1024];

  size_t allocSize = size + sizeof(MallocNode);
  if(arena_head == NULL){
    initArenas();
  }

  /*
  TODO: Remove initHead() method
  if (head == NULL){
    initHead();
  }
  */

  int indexInFreeList = LOG(allocSize) - 1;

  //determine the arena in which memory will be mapped to
  int cores = sysconf(_SC_NPROCESSORS_ONLN);
  int arenaToHandleRequest = numberOfThreads % cores;
  MallocArena *arenaAssigned;
  if ( arenaAssignedToThread == NULL ) {
    pthread_mutex_lock(&mutex);
    arenaAssigned = getArenaForRequest(numberOfThreads);
    arenaAssignedToThread = arenaAssigned;
    pthread_mutex_unlock(&mutex);
  }else{
    arenaAssigned = arenaAssignedToThread;
  }

/*
  if(!arenaAssigned){
    sprintf(buf, "%s\n", "[ERROR] malloc(): Arena is NULL" );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);

  }else{
    sprintf(buf, "%s\n", "[INFO] malloc(): Arena is not null" );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
  }
*/

  pthread_mutex_lock(&arenaAssigned->arena_lock);
  MallocNode *allocatedNode = allocateMemoryInArena(arenaAssigned, size);
  if( allocatedNode == NULL ) {
    errno = ENOMEM;
    pthread_mutex_unlock(&mutex);
    //sprintf(buf, "%s\n", "[ERROR] malloc(): Allocated node  is NULL" );
    //write(STDOUT_FILENO, buf, strlen(buf) + 1);
    pthread_mutex_unlock(&arenaAssigned->arena_lock);
    return NULL;
  }else{
    allocatedNode->isAllocated = 1;
    pthread_mutex_unlock(&arenaAssigned->arena_lock);
    return (char *) allocatedNode + sizeof(MallocNode);
  }
}

/*TODO: rename to createArena()*/
MallocArena * initArenas(){
  char buf[1024];
  //size of arena = 400 mb ~= 4096 * 4096 * 25
  long long int sizeOfArena = PAGE_SIZE * PAGE_SIZE * 25;
  int cores = sysconf(_SC_NPROCESSORS_ONLN); //including virtualized cores
  int cnt = 0;
  MallocArena *curr = arena_head;
  MallocArena *prev = arena_head;
  void *temp;
  int endOfHeap = sbrk(0);
  endOfHeap = endOfHeap + sizeOfArena; //leaving 400 mb space for heap to grow otherwise
  temp = mmap(NULL, sizeOfArena, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (temp == NULL) {
    sprintf(buf, "%s","mmap failed" );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    return;
  }
  curr = (MallocArena*)temp;
  curr->size = sizeOfArena;
  curr->nextArena = NULL;
  curr->prevArena = NULL; //the first time prev == NULL as arena_head == NULL //update: now should always point to NULL
  curr->startAddress = temp;
  curr->endAddress = (void* ) (temp  + sizeOfArena);
  //convert the entire arena into a free block and place it on freeList[13-1]
  int sizeOfFreeBlock =  sizeOfArena - sizeof(MallocArena);

  MallocNode *freeBlock = (MallocNode*) ( (char *) curr + sizeof(MallocArena) );
  freeBlock->size = sizeOfFreeBlock;
  freeBlock->next = NULL;
  freeBlock->prev = NULL;
  freeBlock->isAllocated = 0;
  freeBlock->arenaNumber = cnt;
  freeBlock->blockMaxAddr = (int *) ( (char*) curr + sizeOfArena );
  curr->freeList[12] = freeBlock;
  curr->ordblks = 1;
  curr->smblks = 0;
  curr->hblkhd = 0;
  curr->hblkhd = 0;
  curr->uordblks = 0;
  curr->fordblks = sizeOfArena;

  if( arena_head == NULL ){
    arena_head = curr;
  }

  cnt++;
  prev = curr;
  /*
  sprintf(buf, "[INFO] Arena %d initiated \n", cnt );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */
  return curr;
}


MallocNode *allocateMemoryInArena(MallocArena *assignedArena, size_t size){
  if( size == 0 ){
    return NULL;
  }
  size_t allocSize = size + sizeof(MallocNode);
  int indexInFreeList = (ceil((log(allocSize) / log(BASE)))) - 1;

  char buf[1024];
  /*
  sprintf(buf, "[INFO] malloc(): size of MallocNode: %zu\n allocSize was: %zu\nlooking at index: %d\n",sizeof(MallocNode), allocSize, indexInFreeList );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  //return findFreeBlockInFreeList(assignedArena, size);
  if (indexInFreeList < 12){
    MallocNode* temp = findFreeBlockInFreeList(assignedArena, allocSize);
    arenaAssignedToThread->smblks -= 1;
    arenaAssignedToThread->hblkhd += size;
    arenaAssignedToThread->uordblks += size;
    arenaAssignedToThread->fordblks -= size;
    return temp + sizeof(MallocNode);
  }else{
    MallocNode* temp = getMoreMemoryFromArena(assignedArena, allocSize);
    arenaAssignedToThread->ordblks -= 1;
    arenaAssignedToThread->hblkhd += size;
    arenaAssignedToThread->uordblks += size;
    arenaAssignedToThread->fordblks -= size;
    return temp + sizeof(MallocNode);
  }

}


MallocNode *findFreeBlockInFreeList(MallocArena* arena, size_t size){
  int i = 0;
  char buff2[1024];

  int indexInFreeList = (ceil((log(size) / log(BASE)))) - 1;
  MallocNode *nodeToAllocate = NULL;


  //char buff2[1024];
  //sprintf(buff2, "[INFO] malloc(): findFreeBlockInFreeList(): value of indexInFreeList=%d\nTHREAD ID=%0x\n", indexInFreeList, pthread_self() );
  //write(STDOUT_FILENO, buff2, strlen(buff2) + 1);

  if( arena->freeList[indexInFreeList] != NULL ){
    //free block of the required size is available

    /*
    char buff2[1024];
    sprintf(buff2, "%s\n", "found the nodeToAllocate" );
    write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
    */

    nodeToAllocate = (MallocNode *) arena->freeList[indexInFreeList];

    nodeToAllocate->next = NULL;
    nodeToAllocate->prev = NULL;
    nodeToAllocate->isAllocated = 1;
    nodeToAllocate->size = size - sizeof(MallocNode);

    if(nodeToAllocate->next != NULL){
      arena->freeList[indexInFreeList] = arena->freeList[indexInFreeList]->next;
      arena->freeList[indexInFreeList]->prev = NULL;
    }else{
      arena->freeList[indexInFreeList] = NULL;
    }

    return nodeToAllocate;
  }else{
    //free block of required size is not available
    //find if a free block of greater size is available
    char buf[1024];
    /*
    sprintf(buf, "[INFO] malloc(): indexInFreeList: %d and numberOfFreeLists: %d\n", indexInFreeList, numberOfFreeLists );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
    for(i=indexInFreeList;i<numberOfFreeLists-1;i++){
      MallocNode *temp;
      int currentIndex = i;

      /*
      sprintf(buf, "[INFO] malloc(): currentIndex: %d\n", currentIndex );
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      */

      if( arena->freeList[i] != NULL ){
        /*
        sprintf(buf, "%s\n", "[INFO] malloc(): Found nodeToAllocate" );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        */
        //a free block of greater size is found, then break it down to meet rqmts
        while( currentIndex != indexInFreeList ){
          temp = arena->freeList[currentIndex];
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
          new->arenaNumber = temp->arenaNumber;
          //if the block under consideration has more blocks in front of it,
          //...then the pointers of those blocks need to be updated
          if( temp->next != NULL ){
            MallocNode *next = temp->next;
            if( !next->prev ){
              //handling a BUG
              next = NULL;
            }else{
              next->prev = NULL;
              arena->freeList[currentIndex] = next;
            }

          }else{
            arena->freeList[currentIndex] = NULL;
          }
          //either case, the current block (temp* in our case) needs to be updated
          temp->next = new;  //this logic can actually go above the if case
          currentIndex--;
          arena->freeList[currentIndex] = temp;
          //sprintf(buf, "temp's size: %zu and currentIndex: %d\n", temp->size, currentIndex );
          //write(STDOUT_FILENO, buf, strlen(buf) + 1);
          arenaAssignedToThread->smblks += 1;
        }
        nodeToAllocate = arena->freeList[currentIndex];
        if(nodeToAllocate->next != NULL){
          arena->freeList[currentIndex] = arena->freeList[currentIndex]->next;
          arena->freeList[currentIndex]->prev = NULL;
          nodeToAllocate->next = NULL;
          nodeToAllocate->prev = NULL;
        }else{
          arena->freeList[currentIndex] = NULL;
        }
        //by now we have found a free block and broken it down
        //so we can break the loop and return the block
        return nodeToAllocate;
      }else if( arena->freeList[i] == NULL && currentIndex == numberOfFreeLists-2 ){
        //means we have reached the end of free list and COULD NOT find a free block
        //therefore, request a new free block
        /*
        sprintf(buf, "%s\n", "[INFO] malloc(): Time to fetch more mem" );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        */
        MallocNode *moreMemory = getMoreMemoryFromArena(arena, PAGE_SIZE); // sbrk(PAGE_SIZE);

        if(moreMemory == NULL){
          errno = ENOMEM;
          return NULL;
        }
        arena->freeList[numberOfFreeLists-2] = (MallocNode *) moreMemory;
        arena->freeList[numberOfFreeLists-2]->size = PAGE_SIZE;
        arena->freeList[numberOfFreeLists-2]->isAllocated = 0;
        arena->freeList[numberOfFreeLists-2]->next = NULL;
        arena->freeList[numberOfFreeLists-2]->prev = NULL;
        arena->freeList[numberOfFreeLists-2]->dummy = 99999;
        arena->freeList[numberOfFreeLists-2]->blockMaxAddr = (int *) ( (char*) moreMemory + PAGE_SIZE );
        arena->freeList[numberOfFreeLists-2]->blockMinAddr = arena->startAddress;
        arena->freeList[numberOfFreeLists-2]->arenaNumber = moreMemory->arenaNumber;
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

MallocNode * getMoreMemoryFromArena(MallocArena* arena, size_t size){
  MallocNode *freeMemory = arena->freeList[numberOfFreeLists-1];
  MallocNode *temp = freeMemory;
  char buf[1024];
  /*
  sprintf(buf, "%s\n", "[INFO] malloc(): Getting more memory" );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  if ( temp > arena->endAddress || temp < arena->startAddress ){
  temp = NULL;
}
  */


  while( temp->next != NULL && temp->isAllocated != 0 ){
    temp = temp->next;
  }
  if( temp == NULL ){
    /*
    sprintf(buf, "%s\n", "[ERROR] malloc(): getMoreMemoryFromArena(): No more memory left" );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
    return NULL; //No memory left
  }
  /*
          while( temp->size - sizeof(MallocNode) < 4096 ){
            //look for another block that is big enough..this block is dirty for us
            while( temp != NULL && temp->isAllocated != 0 ){
              temp = temp->next;
            }
            if( temp == NULL ){
              return NULL; //No memory left
            }
          }
  */

  MallocNode *newFreeBlock = (MallocNode*) ( (char *) temp + size );
  newFreeBlock->size = temp->size - size;

  newFreeBlock->prev = temp->prev;
  newFreeBlock->next = temp->next;

  arena->freeList[numberOfFreeLists-1] = (MallocNode *) newFreeBlock;
  //arena->freeList[numberOfFreeLists-1]->size = size;
  arena->freeList[numberOfFreeLists-1]->isAllocated = 0;
  arena->freeList[numberOfFreeLists-1]->dummy = 99999;
  arena->freeList[numberOfFreeLists-1]->blockMaxAddr = freeMemory->blockMaxAddr;
  arena->freeList[numberOfFreeLists-1]->blockMinAddr = freeMemory->blockMinAddr;

  temp->prev = NULL;
  temp->next = NULL;
  temp->size = 4096;
  temp->dummy = 99999;
  temp->blockMaxAddr = (void *) ( (char*) temp + PAGE_SIZE ) ;
  temp->blockMinAddr = freeMemory->blockMinAddr;
  return temp;
}

MallocArena * getArenaForRequest(int arenaToHandleRequest){
  if( arena_head == NULL ){
    MallocArena *newArena = initArenas();
    arenaAssignedToThread = newArena;
    return initArenas();
  }
  int cores = sysconf(_SC_NPROCESSORS_ONLN);
  MallocArena* arenaToReturn;
  if(numberOfThreads > cores){
    int arenaNumber = numberOfThreads % cores;
    int t = 0;
    MallocArena *curr = arena_head;
    while(t<arenaNumber){
      t++;
      curr = curr->nextArena;
    }
    arenaToReturn = curr;
  }else{
    MallocArena *curr = arena_head;
    int cnt = 0;
    while(curr->nextArena != NULL){
      cnt++;
      curr = curr->nextArena;
      /*sprintf(buf, "Actually fetching arena number: %d \n", cnt );
      write(STDOUT_FILENO, buf, strlen(buf) + 1);*/
    }
    char buf[1024];
    /*
    sprintf(buf, "[INFO] malloc(): Fetched arena number: %d in thread: %0x\n", cnt, pthread_self() );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
    MallocArena *newArena = initArenas();
    curr->nextArena = newArena;
    newArena->prevArena = curr;
    arenaToReturn = newArena;

  }


  return arenaToReturn;
}
