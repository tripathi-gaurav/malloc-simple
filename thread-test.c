#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <pthread.h>
#include "malloc.c"
#include "free.c"
#include "realloc.c"
#include "calloc.c"
#include "memalign.c"
#include "posix_memalign.c"


void *func() {
	printf("Inside thread 1\n");
	void *p = malloc(3);

  //printf("\n\n Successfully malloc'd %zu func 1 bytes at addr %p\n", 64, p);
	void *p1 = malloc(10);
	//printf("\n\nSuccessfully malloc'd %zu func 2 bytes at addr %p\n", 64, p1);

	//void *p3 = calloc(10,3);
	//void *p8= realloc(p1,13);

	void *p12 = calloc(12,3);
	void *p8= realloc(p1,18);
	void *p11 = memalign(128,3);
void *p17 = memalign(128,3);
void **myptr;
//posix_memalign (myptr, 128, 3);
//reallocarray(p12,5,8);

	//printf("\n\nSuccessfully malloc'd %zu func 3 bytes at addr %p\n", 64, p3);
	free(p);
//	free(p1);
	void *p4 = malloc(3);
	printf("-----Thread 1 Done----\n");
	return NULL;

}

void *func1() {

	printf("Inside thread 2\n");
	void *p5 = malloc(5);
	//printf("\n\nSuccessfully malloc'd %zu bytes at addr %p\n", 5000, p2);
	void *p6 = malloc(5);
	//printf("\n\nSuccessfully malloc'd %zu bytes at addr %p\n", 64, p);
	void *p7 = malloc(2);
	//printf("\n\nSuccessfully malloc'd %zu bytes at addr %p\n", 64, p1);

	void *p8 = malloc(5);
	void *p9 = malloc(3);
	free(p9);
	void *p11 = memalign(128,3);
	void *p17 = memalign(128,3);
	printf("-----Thread 2 Done----\n");
	return NULL;

}

int main() {

	pthread_t t1, t2, t3;

	for(int i=0;i<800;i++){
  pthread_create(&t1, NULL, func,  NULL);
	pthread_create(&t2, NULL, func1, NULL);
	pthread_create(&t3, NULL, func,  NULL);
  pthread_join(t1, NULL);
	pthread_join(t2, NULL);
  pthread_join(t3, NULL);
}

	return 0;
}
