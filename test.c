#include <stdio.h>
#include <unistd.h>

main(){

long a = sysconf(_SC_PAGESIZE);
long b = sysconf(_SC_NPROCESSORS_ONLN);
printf("PAGE_SIZE=%ld\nNUMBER OF CORES=%ld\n", a, b);
}
