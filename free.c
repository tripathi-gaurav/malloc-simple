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

MallocNode* findBuddy(MallocNode* nodeToFree);
MallocArena * getArenaByArenaNumber(int arenaToHandleRequest);


void free(void *ptr){

  char buf[1024];
  /*
  sprintf(buf, "%s\n", "[INFO] free(): called" );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  //pthread_mutex_lock(&mutex);

  if( ptr == NULL ){
    /*
        sprintf(buf, "%s\n", "[ERROR] free(): ptr sent was NULL" );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
        //pthread_mutex_unlock(&mutex);
        return ;
  }

  MallocNode *nodeToFree = (MallocNode*) ( (void *) ptr - sizeof(MallocNode) );
  /*
  sprintf(buf, "[INFO] free(): \nsize: %zu\nallocated: %d\nend addr:%p\nstart addr:%p\nnext?:%p\n ==ptr==%p\n",
          nodeToFree->size, nodeToFree->isAllocated, nodeToFree->blockMaxAddr, nodeToFree->blockMinAddr, nodeToFree->next, ptr
        );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  if ( nodeToFree > arenaAssignedToThread->endAddress || nodeToFree < arenaAssignedToThread->startAddress ){
    //HACK to fix dangling pointer issue
    nodeToFree = NULL;
    return ;
  }

  if( nodeToFree->isAllocated != 1 ){
    //pthread_mutex_unlock(&mutex);
    /*
    sprintf(buf, "%s\n", "[INFO] free(): incorrect value of isAllocated " );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
    return ;
  }

  if( nodeToFree->size == 0 ){
    //pthread_mutex_unlock(&mutex);
    /*
    sprintf(buf, "%s\n", "[ERROR] free(): size 0 " );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
    return ;
  }

  pthread_mutex_lock(&mutex);
  if(!arenaAssignedToThread){
    pthread_mutex_unlock(&mutex);
    return;
  }
  MallocArena *arena = arenaAssignedToThread; //getArenaByArenaNumber(nodeToFree->arenaNumber);
  pthread_mutex_unlock(&mutex);

  /*
  if(arena){
    sprintf(buf, "[INFO] free(): Arena fetched of number: %d \n", nodeToFree->arenaNumber );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
  }
  */


  pthread_mutex_lock(&arena->arena_lock);

  nodeToFree->isAllocated = 0;
  //now find buddy and coalesce
  MallocNode *buddy = findBuddy(nodeToFree);
  MallocNode *tempNext, *tempPrev;
  while( buddy != NULL ){
    /*
    char buff[1024];
    sprintf(buff, "%s", "buddy is not NULL\n" );
    write(STDOUT_FILENO, buff, strlen(buff) + 1);
    */

    int currentIndex = LOG(buddy->size) - 1;
      if( buddy->next != NULL && buddy->prev != NULL ){
      //buddy is in between the list
      tempNext = buddy->next;
      tempPrev = buddy->prev;
      tempPrev->next = tempNext;
      tempNext->prev = tempPrev;
      /*
      char buff2[1024];
      sprintf(buff2, "%s", "!!!buddy is in between the list!!!\n" );
      write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      */
    }else if( buddy->next != NULL && buddy->prev == NULL ){
      //buddy is the first element in the list
      tempNext = buddy->next;
      tempNext->prev = NULL;

      /*
      char buff2[1024];
      sprintf(buff2, "%s", "!!!buddy is the first element in the list!!!\n" );
      write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      */

      arena->freeList[currentIndex] = tempNext;
    }else if( buddy->next == NULL && buddy->prev != NULL ){
      //buddy is the last element in the list
      tempPrev = buddy->prev;
      tempPrev->next = NULL;
      /*
      char buff2[1024];
      sprintf(buff2, "%s", "!!!buddy is the last element in the list!!!\n" );
      write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      */
    }else if( buddy->next == NULL && buddy->prev == NULL ){
      //buddy is the first and last element
      arena->freeList[currentIndex] = NULL;
      /*
      char buff2[1024];
      sprintf(buff2, "%s", "!!!buddy is the first and last element!!!\n" );
      write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      */
      buddy->next = NULL;
    }else{
      /*
      char buff2[1024];
      sprintf(buff2, "%s", "[ERROR] !!!free.c Error!!!\n" );
      write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      */
    }
    nodeToFree->next = NULL;
    if( buddy < nodeToFree ){
      nodeToFree = buddy;
    }
    //probably need to do this
    buddy = NULL;
    nodeToFree->size = nodeToFree->size * 2;
    fflush(stdout);
    buddy = findBuddy(nodeToFree);
  }
  //place the coalesced block at the correct location
  int indexInFreeList = LOG(nodeToFree->size) - 1;
  if( indexInFreeList >= 12 ){
    //because all such blocks will be placed on the 12th index (i.e. 13th poistion)
    indexInFreeList = 12;
  }
  if( arena->freeList[indexInFreeList] != NULL ){
    MallocNode *curr = arena->freeList[indexInFreeList];
    while( curr->next != NULL ){
      curr = curr->next;
    }
    curr->next = nodeToFree;
    nodeToFree->prev = curr;
  }else{
    arena->freeList[indexInFreeList] = nodeToFree;
  }
  if( indexInFreeList < 12 ){
    arenaAssignedToThread->smblks += 1;     /* Number of free fastbin blocks */
  }
  arenaAssignedToThread->hblkhd -= nodeToFree->size;    /* Space allocated in mmapped regions (bytes) */
  arenaAssignedToThread->uordblks -= nodeToFree->size;  /* Total allocated space (bytes) */
  arenaAssignedToThread->fordblks -= nodeToFree->size;  /* Total free space (bytes) */

  pthread_mutex_unlock(&arena->arena_lock);
}

MallocNode* findBuddy(MallocNode* nodeToFree){
  int freeListIndex = LOG(nodeToFree->size) - 1;
  int arenaNumber = nodeToFree->arenaNumber;
  pthread_mutex_lock(&mutex);
  MallocArena * arena = arenaAssignedToThread; //getArenaByArenaNumber(arenaNumber);
  pthread_mutex_unlock(&mutex);

  char buf[1024];
  /*
  sprintf(buf, "End addr of nodeToFree: %p\n and index to look at is: %d\n", nodeToFree->blockMaxAddr, freeListIndex );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  MallocNode *curr = arena->freeList[freeListIndex];
  if ( curr > arenaAssignedToThread->endAddress || curr < arenaAssignedToThread->startAddress ){
    //HACK to fix dangling pointer issue
    curr = NULL;
    return NULL;
  }

  while( curr != NULL ){

    /*
    sprintf(buf, "Curr->isAllocated:%d and curr->blockMaxAddr: %p\n and index to look at is: %d\n",
     curr->isAllocated, curr->blockMaxAddr, freeListIndex );
     */

    write(STDOUT_FILENO, buf, strlen(buf) + 1);

    if(curr->isAllocated==0){
      //char buf[1024];
      /*
      sprintf(buf, "curr's ptr at %p \n", curr );
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      sprintf(buf, "size is : %zu \n", sizeof(MallocNode) );
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      sprintf(buf, "CURR INFO: curr->isAllocated: %d, curr->size: %zu, curr->blockMaxAddr: %p\n", curr->isAllocated, curr->size, curr->blockMaxAddr );
      write(STDOUT_FILENO, buf, strlen(buf) + 1);
      */

  //&& ( (int) nodeToFree ^ (int) nodeToFree->size && ( ( (char) nodeToFree->blockMinAddr / (int) nodeToFree->size * 2 ) ) == 0 )
      if( (nodeToFree->blockMaxAddr == curr->blockMaxAddr)
        //&& ( ( (int) nodeToFree ^  (int) nodeToFree->size ) == (int) curr )
        ){
        //sprintf( buf, "End addr of curr that is being free'd: %p\n", nodeToFree->blockMaxAddr );
        //write(STDOUT_FILENO, buf, strlen(buf) + 1);
        /*
        sprintf( buf, "%s\n", "wohoo" );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        sprintf(buf, "wohoo== nodeToFree->next:%p and curr->dummy: %d and curr->next: %p\n", nodeToFree->next, curr->dummy, curr->next );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        */
        return curr;
      }

    }
    curr = curr->next;
  }
  return curr;
}

MallocArena * getArenaByArenaNumber(int arenaToHandleRequest){
  if( arena_head == NULL ){
    return NULL;
  }

/*
  char buf[1024];
  sprintf(buf, "[INFO] free(): Fetching arena number: %d \n", arenaToHandleRequest );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  MallocArena *curr = arena_head;
  int cnt = 0;
  while(cnt < arenaToHandleRequest){
    cnt++;
    curr = curr->nextArena;
    /*
    sprintf(buf, "[INFO] free(): Actually fetching arena number: %d \n", cnt );
    write(STDOUT_FILENO, buf, strlen(buf) + 1);
    */
  }
  return curr;
}
