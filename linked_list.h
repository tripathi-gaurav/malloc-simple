#include <stdio.h>
#include <pthread.h>

#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#define BASE 2
//will be using this log to compute the power to which 2 is raised to
//for satisfying an allocate request of x bytes
#define LOG(size) (ceil((log(size) / log(BASE))))
#define PAGE_SIZE 4096


typedef struct node{
	size_t size;
	struct node *next;
	struct node *prev;
	int isAllocated;
	int *blockMaxAddr;
	int *blockMinAddr;
	int dummy;
} MallocNode;

extern __thread MallocNode *head;
extern pthread_mutex_t mutex;
//free list holding lists of free MallocNodes
//each index signifies 2^(index) of bytes of MallocNode that is free
extern int numberOfFreeLists; // (ceil((log(PAGE_SIZE) / log(BASE))));
extern MallocNode* freeList[12]; //should be number_OfFreeLists - 1

#endif
