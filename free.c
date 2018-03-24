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


void free(void *ptr){


  pthread_mutex_lock(&mutex);

  if( ptr == NULL ){
        pthread_mutex_unlock(&mutex);
        return ;
  }


  MallocNode *nodeToFree = (MallocNode*) ( (char *) ptr - sizeof(MallocNode) );

  if( nodeToFree->isAllocated != 1 || nodeToFree->isAllocated != 999 ){
    pthread_mutex_unlock(&mutex);
    return ;
  }
  /*
  char buf[1024];
  sprintf(buf, "size: %zu\nallocated: %d\nend addr:%p\nstart addr:%p\nnext?:%p\n ==ptr==%p", nodeToFree->size, nodeToFree->isAllocated, nodeToFree->blockMaxAddr, nodeToFree->blockMinAddr, nodeToFree->next, ptr );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */
  if( nodeToFree->size >= (size_t) 4096 ){
    /*
    char buff[1024];
    sprintf(buff, "%s", "performing munmap\n" );
    write(STDOUT_FILENO, buff, strlen(buff) + 1);
    */
    int res = munmap( nodeToFree, nodeToFree->size );
  }else{
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
        //char buf[1024];
        /*
        char buff2[1024];
        sprintf(buff2, "%s", "!!!buddy is the first element in the list!!!\n" );
        write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
        */
        freeList[currentIndex] = tempNext;
      }else if( buddy->next == NULL && buddy->prev != NULL ){
        //buddy is the last element in the list
        tempPrev = buddy->prev;
        tempPrev->next = NULL;
        //char buf[1024];
        /*
        char buff2[1024];
        sprintf(buff2, "%s", "!!!buddy is the last element in the list!!!\n" );
        write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
        */
      }else if( buddy->next == NULL && buddy->prev == NULL ){
        //buddy is the first and last element
        freeList[currentIndex] = NULL;
        /*
        char buff2[1024];
        sprintf(buff2, "%s", "!!!buddy is the first and last element!!!\n" );
        write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
        */
        //21-MAR
        buddy->next = NULL;
      }else{
        //char buf[1024];
        char buff2[1024];
        //sprintf(buff, "prev: %p and next: %p\n", buddy->prev, buddy->next );
        sprintf(buff2, "%s", "!!!free.c Error!!!\n" );
        write(STDOUT_FILENO, buff2, strlen(buff2) + 1);
      }

      nodeToFree->next = NULL;
      if( buddy < nodeToFree ){
        nodeToFree = buddy;
      }
      //probably need to do this
      //buddy = NULL;
      nodeToFree->size = nodeToFree->size * 2;
      fflush(stdout);
      buddy = findBuddy(nodeToFree);
    }

    //place the coalesced block at the correct location
    int indexInFreeList = LOG(nodeToFree->size) - 1;
    if( freeList[indexInFreeList] != NULL ){
      MallocNode *curr = freeList[indexInFreeList];
      while( curr->next != NULL ){
        curr = curr->next;
      }
      curr->next = nodeToFree;
      nodeToFree->prev = curr;
    }else{
      freeList[indexInFreeList] = nodeToFree;
    }
  }
  pthread_mutex_unlock(&mutex);
}

MallocNode* findBuddy(MallocNode* nodeToFree){
  int freeListIndex = LOG(nodeToFree->size) - 1;

  /*
  char buf[1024];
  sprintf(buf, "End addr of nodeToFree: %p\n and index to look at is: %d\n", nodeToFree->blockMaxAddr, freeListIndex );
  write(STDOUT_FILENO, buf, strlen(buf) + 1);
  */

  MallocNode *curr = freeList[freeListIndex];


  while( curr != NULL ){

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
      if( (nodeToFree->blockMaxAddr == curr->blockMaxAddr) &&
          ( ( (int) nodeToFree ^  nodeToFree->size ) == (int) curr )
        ){
        //sprintf( buf, "End addr of curr that is being free'd: %p\n", nodeToFree->blockMaxAddr );
        //write(STDOUT_FILENO, buf, strlen(buf) + 1);
        /*
        sprintf( buf, "%s\n", "wohoo" );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        sprintf(buf, "wohoo== nodeToFree->next:%p and curr->dummy: %d and curr->next: %p\n", nodeToFree->next, curr->dummy, curr->next );
        write(STDOUT_FILENO, buf, strlen(buf) + 1);
        */

        /*
        if( !curr->next && !curr->prev){
          sprintf(buf, "wohoo== %s\n", "bc ye to chala" );
          write(STDOUT_FILENO, buf, strlen(buf) + 1);
        }else{
          sprintf(buf, "wohoo== %s\n", "bc bakchod billi" );
          write(STDOUT_FILENO, buf, strlen(buf) + 1);
        }
        */
        return curr;
      }

    }
    curr = curr->next;
  }
  return curr;
}
