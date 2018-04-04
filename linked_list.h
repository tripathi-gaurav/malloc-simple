#include <stdio.h>
#include <pthread.h>

#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#define BASE 2
//will be using this log to compute the power to which 2 is raised to
//for satisfying an allocate request of x bytes
#define LOG(size) (ceil((log(size) / log(BASE))))
#define PAGE_SIZE 4096
#define FREE_LISTS LOG(PAGE_SIZE)


typedef struct node{
	size_t size;
	struct node *next;
	struct node *prev;
	int isAllocated;
	int *blockMaxAddr;
	int *blockMinAddr;
	int arenaNumber;
	int dummy;
} MallocNode;

typedef struct arena{
	pthread_mutex_t arena_lock;
	//free list holding lists of free MallocNodes
	//each index signifies 2^(index) of bytes of MallocNode that is free
	MallocNode* freeList[13]; //should be numberOfFreeLists - 1
	int numberOfFreeLists;
	void *startAddress;
	void *endAddress;
	void *nextArena;
	void *prevArena;
	size_t size;
	int ordblks;   /* Number of free chunks */
	int smblks;    /* Number of free fastbin blocks */
	int hblks;     /* Number of mmapped regions */
	int hblkhd;    /* Space allocated in mmapped regions (bytes) */
	int uordblks;  /* Total allocated space (bytes) */
  int fordblks;  /* Total free space (bytes) */
} MallocArena;

typedef struct mallinfo {

	int arena;     /* Non-mmapped space allocated (bytes) */
	int ordblks;   /* Number of free chunks */
  int smblks;    /* Number of free fastbin blocks */
  int hblks;     /* Number of mmapped regions */
  int hblkhd;    /* Space allocated in mmapped regions (bytes) */
  int usmblks;   /* Maximum total allocated space (bytes) */
  int fsmblks;   /* Space in freed fastbin blocks (bytes) */
  int uordblks;  /* Total allocated space (bytes) */
  int fordblks;  /* Total free space (bytes) */
  int keepcost;  /* Top-most, releasable space (bytes) */
};

extern __thread MallocNode *head;
extern MallocArena *arena_head;
extern __thread MallocArena *arenaAssignedToThread;
extern pthread_mutex_t mutex;
extern int numberOfThreads;

//TODO: !!URGENT!! remove this for hw4
extern int numberOfFreeLists; // (ceil((log(PAGE_SIZE) / log(BASE))));
//TODO: !!URGENT!! remove this for hw4
//extern MallocNode* freeList[12];
#endif
