#include "threadpool.h"


void* run( void* arg ) {
	int id = *(int*)arg ;
	free(arg) ;

	int i = 0 ;
	printf("线程[%p]执行任务[%d]\n",pthread_self(),id) ;
	sleep(2) ;
	printf("线程[%p]===========结束任务[%d]\n",pthread_self(),id) ;
}
void main(){
	int i ;	
	threadpool_t pool ;
	threadpool_init(&pool,3) ;
// 向线程池添加5个任务
	for( i = 0 ; i<5 ;++i){
		int* p = malloc(sizeof(int)) ;
		*p = i ;
// 在添加任务中去寻找线程来处理任务
		threadpool_add_task( &pool , run , (void*)p ) ;
	}
	printf("main end\n") ;
	sleep(10) ;
	
	threadpool_destroy(&pool) ;
}
