#include <stdio.h>
#include <time.h>
void main(){
	struct timespec abstime ;
	clock_gettime(CLOCK_REALTIME , &abstime ) ;
//	abstime.tv_sec += 5 ;
	printf("%d",abstime.tv_sec) ;
}
