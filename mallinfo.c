#include "linked_list.h"

struct mallinfo mallinfo(){
  struct mallinfo info;
  info.ordblks = 0;
  info.smblks = 0;     /* Number of free fastbin blocks */
  info.hblkhd = 0;    /* Space allocated in mmapped regions (bytes) */
  info.uordblks = 0;  /* Total allocated space (bytes) */
  info.fordblks = 0;  /* Total free space (bytes) */

  MallocArena * curr = arena_head;
  while( curr !=  NULL ) {
    info.ordblks += curr->ordblks;
    info.smblks += curr->smblks;
    info.hblkhd += curr->hblkhd;
    info.uordblks += curr->uordblks;
    info.fordblks += curr->fordblks;
  }

  return info;
}
