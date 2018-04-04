Malloc implementation
=======================================

Usage
---------------------------------------
* `make check`  : Compiles libmalloc.so and runs it against t-test1.c made by Wolfram Gloger
* `make gdb`    : Runs the t-test1 against libmalloc with LD_PRELOAD in gdb
* `make clean`  : cleans the dir

Design overview:
---------------------------------------

Used the concept as explained here:
https://www.cs.uml.edu/~jsmith/OSReport/frames.html

The previous implementation has been extended to add arenas.
As a when a new thread comes in, a new arena for allocation/deallocation is created using mmap().
There will be a total of `sysconf(_SC_NPROCESSORS_ONLN)` arenas.
Any more threads will be assigned to thread_count % `sysconf(_SC_NPROCESSORS_ONLN)`.

The array of freeList holding free blocks now holds an additional linked list of free blocks that are of size greater than 4096.

---------------------------------------


In my implementation, I am maintaining an array called freeList, which
is an array of doubly LinkedLists connected by nodes.
Each Node of the LinkedList signifies a block of memory and the next and
previous pointers to the next block of free memory of equal size (a
LinkedList on a particular index of the freeList will have Nodes of free
blocks of equal sizes):
#### malloc()

**The ListNode contains the following information:**

0. `size_t size`: Stores the complete size of the allocated memory block
which also includes the additional space required for storing the
metadata (the space required for the Node itself)
1. `Node *prev` : A pointer to the block of memory that came before this
block in the LinkedList.
2. `Node *next`: A pointer to the block of memory that came before this
block in the LinkedList.
3. `int isAllocated`: A simple flag signifying whether the block of memory
has been allocated to the user or not. Used to verify if the block pointed to by the pointer was allocated by libmalloc.so
4. `int *blockMaxAddr`: Store the maximum address of the block of memory,
when a block is obtained using a sbrk() or mmap() call. This information is used while coalescing two blocks.
5. `int *blockMinAddr`: Should be used to store the base address of the allocated block. Wanted to use this for coalescing, but instead rely on XOR.

The LOG(size) preprocessor helps computer the log base 2 for a given a
size, which signifies the index number on the freeList to look for a
block of memory. Explanation:

2^12 = 4096 which is equivalent to [PAGE_SIZE]
log base 2 (2^12) = 12
freeList[12-1] = freeList[11] = LinkedList of all free memory blocks
that are of size 4096 bytes

similarly a request for 64 bytes will have the following translation for
freeList:
64 = 2^6
log base 2 (2^6) = 6
freeList[6-1] = freeList[5] = LinkedList of all free memory blocks that
are of size 64 bytes.

Now, note that our ListNode is 56 bytes large. Each memory block will
contain a ListNode at the beginning, which will contain the metadata for
the entire block. Therefore any allocation will be take at least **64 bytes** of space. Ex. a request of 1 bytes will try to allocate 56+1 = 57 bytes of space, but will return a block of 64 bytes with the starting address from the 57 byte for the user.
#### free()

`free()` takes in a pointer to a block and converts it to `struct MallocNode`. Then looks at `node->isAllocated` to verify the block's status. Further, it will check `node->size` to see if it's greater than 4096 bytes, in which case, it will run `munmap` else, it will try to look for a "buddy" of the block.

##### A block's buddy will be:
0. Located at `int freeListIndex = LOG(nodeToFree->size) - 1`
1. If a block is found at `freeListIndex` then:
  * The two block will share the same `blockMaxAddr`, i.e. `nodeToFree->blockMaxAddr == curr->blockMaxAddr`
  * Also, to ensure that the two blocks are *consecutive*, we can *XOR* the current block's address with it's own size, if the result is equal to the address of the next block, then the blocks are consecutive. (`nodeToFree ^  nodeToFree->size`)

##### Example of buddy malloc and free:
When a request for 64 bytes comes in:
0. the allocator will try to allocate 64+56 (size of metadata)=120 bytes
1. To do this, the allocator will see if there is any block available in the `freeList` at `index` LOG base 2 (120) = 6 (truncated to 6 since we are assigning to int)
2. If no block is available at freeList[6], the allocator will look at index 7,8,9....11 till a block is found.
3. If no block is found till freeList[11], the allocator will call `sbrk(4096)` to request for more memory, and store that block at freeList[11]
4. Now the allocator will break the 4096 block at freeList[11] into 2048 blocks and store both blocks at freeList[10] and freeList[11] will now store `NULL`
5. The first block at freeList[10] will be broken into two 1024 blocks and stored at freeList[9] and freeList[10] will now store the second block of size 2048.
6. Each of the first block will be broken into two equal sized blocks and stored at the lower index till two blocks are stored at freeList[6]
7. The first block of freeList[6] is allocated to the user with the starting address from the 57th byte and freeList[6] stores the second block of 128 bytes.

When a request to free comes in for the block immediately after malloc:
0. The allocator looks for an available block in `freeList` at `index` LOG base 2 (120) = 6 (truncated to 6 since we are assigning to int)
1. A block is available from previous breakdowns
2. The blockMaxAddr and starting address are checked to see it is from the same PAGE and is consecutive
3. Both matches check and block's size and next and prev pointers are updated
4. freeList[6] now stores NULL
5. The allocator looks for an available block in `freeList` at `index` LOG base 2 (256) = 7
6. A block is found and coalesced with the current block and freeList[7] now points to NULL.
7. The allocator continues the process of coalescing and finally places a block of size 4096 on freeList[11].

Design decisions that you made.
--------------------
* `mallocNode->isAllocated` to store 999 when allocated using mmap else, stores 1 when allocated using the buddy system/freeList
* `mallocNode->prev` serves the sole purpose of identifying whether the * block is in the middle/end/start in the list. It can be done away with.
* `mallocNode->dummy` is just a dummy variable and should be removed.
* `mallocNode->blockMinAddr` should store the base address of the PAGE/ block from which it is allocated. But I stuck with XOR on pointer address with it's size.

Known bugs and errors
--------------------
The t-test1 is passing a NULL ptrs to `free()`. This bug avoided using `mallocNode->isAllocated`
