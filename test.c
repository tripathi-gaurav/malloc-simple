#include <stdio.h>
#include <unistd.h>

main(){
long a = sysconf(_SC_PAGESIZE);
printf("%ld", a);
}
